#define ID_meter  1
uint16_t Reg_addr;

float DATA_METER;
#include <ModbusMaster.h>
//#include <SoftwareSerial.h>
#define baud 19200
#define timeout 100
#define polling 100
#define retry_count 10
ModbusMaster node;

#include <SPI.h>
#include <LoRa.h>
#define ss 5
#define rst 15
#define dio0 4
int incomingByte = 0;

//01 04 00 00 00 02 71 3F // Test 30001
//------------------------------------------------
// Convent 32bit to float
//------------------------------------------------
float HexTofloat(uint32_t x) {
  return (*(float*)&x);
}

uint32_t FloatTohex(float x) {
  return (*(uint32_t*)&x);
}
//------------------------------------------------

float Read_Meter_float(char addr, uint16_t REG) {
  float i = 0;
  uint8_t result, j;

  uint16_t data[2];
  uint32_t value = 0;
  node.begin(ID_meter, Serial2);
  // node.preTransmission(preTransmission);
  // node.postTransmission(postTransmission);

  result = node.readHoldingRegisters(REG, 2);  ///< Modbus function 0x03 Read Holding Registers
  delay(500);
  if (result == node.ku8MBSuccess) {
    for (j = 0; j < 2; j++) {
      data[j] = (node.getResponseBuffer(j));
    }
    //uint16_t x = data[1]; // para o valor da frequência tenho que ler algo em torno de 426F xxxx;
    //uint16_t y = data[0];

    Serial.print(data[1], HEX);
    Serial.println(data[0], HEX);

    value = data[0];
    value = value << 16;
    value = value + data[1];
    i = HexTofloat(value);
    //Serial.println("Connect modbus Ok.");
    return i;
  } else {
    Serial.print("Connect modbus fail. REG >>> ");
    Serial.println(REG);  // Debug
    delay(1000);
    return 0;
  }
}

void setup() {
  Serial.begin(baud);
  Serial2.begin(baud, SERIAL_8E1);
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(915E6)) {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  LoRa.setTxPower(20);  // Supported values are 2 to 20
  // LeRa.setSpreadingFactor(12);           // Supported values are between 6 and 12
  // LoRa.setSignalBandwidth(62.5E3);       // Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3.
  // LoRa.setCodingRate4(8);                // Supported values are between 5 and 8, these correspond to coding rates of 4/5 and 4/8
  LoRa.setGain(6);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print("dari lora : ");
      Serial.println(LoRaData);
      Reg_addr = LoRaData.toInt()%10000;
      if (Reg_addr) {
        DATA_METER = Read_Meter_float(ID_meter, Reg_addr - 1);
        Serial.print(DATA_METER, 3);
        // delay(3000);
        if (DATA_METER > 0.001) {
          String kirim = String(DATA_METER);
          Serial.println(kirim);
          LoRa.beginPacket();
          LoRa.print(kirim);
          LoRa.endPacket();
        } else {
          Serial.println("no data");
          LoRa.beginPacket();
          LoRa.print("no data");
          LoRa.endPacket();
        }
      } else {
        Serial.println("error");
      }
    }
  }

  if (Serial.available() > 0) {
    String kirim = Serial.readString();
    Serial.println(kirim);
    LoRa.beginPacket();
    LoRa.print(kirim);
    LoRa.endPacket();
  }
}
