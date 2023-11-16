#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include <ModbusMaster.h>
#define baud 19200
#define timeout 100
#define polling 100
#define retry_count 10
#define ID_meter 1
ModbusMaster node;
//define the pins used by the transceiver module
#define ss 5
#define rst 15
#define dio0 4
// sck 18
// mosi 23
// miso 18
int incomingByte = 0;
// String kode_alat = "";
uint16_t data[10];
DynamicJsonDocument doc(1024);
#define ID_meter 1
#define Total_of_Reg 2

#define Reg_LN 3109  //  1. // frequencia 426F xxxx ;
#define Reg_FQ 3027  //  2

uint16_t Reg_addr[2] = {
  Reg_LN,
  Reg_FQ,
};

float DATA_METER[Total_of_Reg];

float HexTofloat(uint32_t x) {
  return (*(float*)&x);
}

uint32_t FloatTohex(float x) {
  return (*(uint32_t*)&x);
}

float Read_Meter_float(char addr, uint16_t REG) {
  float i = 0;
  uint8_t result, j;

  uint16_t data[2];
  uint32_t value = 0;
  node.begin(ID_meter, Serial2);
  // node.preTransmission(preTransmission);
  // node.postTransmission(postTransmission);

  result = node.readHoldingRegisters(REG, 2);  ///< Modbus function 0x03 Read Holding Registers
  delay(100);
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
    delay(100);
    return 0;
  }
}
void GET_METER() {  // Update read all data
  delay(100);
  for (char i = 0; i < Total_of_Reg; i++) {
    DATA_METER[i] = Read_Meter_float(ID_meter, Reg_addr[i]);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(baud);
  Serial2.begin(baud, SERIAL_8E1);
  while (!Serial)
    ;
  Serial.println("LoRa Receiver");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);

  //replace the LoRa.begin(---E-) argument with your location's frequency
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(915E6)) {
    Serial.println(".");
    delay(500);
  }
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  LoRa.setTxPower(20);  // Supported values are 2 to 20
  // LoRa.setSpreadingFactor(12);      // Supported values are between 6 and 12
  // LoRa.setSignalBandwidth(10.4E3);  // Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3.
  // LoRa.setCodingRate4(8);                // Supported values are between 5 and 8, these correspond to coding rates of 4/5 and 4/8
  LoRa.setGain(6);  // Supported values are between 0 and 6
}

void loop() {
  // put your main code here, to run repeatedly:
  if (millis() % 1000 == 0) {
    GET_METER();
    String kirim = String(DATA_METER[0]) + ";" + String(DATA_METER[1]);
    LoRa.beginPacket();
    LoRa.print(kirim);
    LoRa.endPacket();
  }
}
