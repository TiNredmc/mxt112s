// Atmel maXTouch MXT112S example code with STM8L
// Ported by TinLethax 2022/08/11 +7

#include <Wire.h>
#include <stdint.h>

/* I2C 7bit address of MXT112S */
#define MXT_ADDR  0x4B

#define MXT_ID_BLK_SIZ  7// Query ID size of 7 bytes

// Number of elements in the Object Table
uint8_t num_obj = 0;

// Read data from MXT112S
void mxt_read(uint16_t addr, uint8_t *data, uint16_t len) {

  Wire.beginTransmission(MXT_ADDR);// Begin TX
  Wire.write((uint8_t)(addr << 8));// Write LSB byte address to MXT112S.
  Wire.write((uint8_t)addr);// Write MSB byte address to MXT112S.

  Wire.endTransmission();

  Wire.requestFrom(MXT_ADDR, len);// Begin RX
  while (Wire.available() && len--) { //
    *data++ = Wire.read();
  }

}

// Write data to MXT112S
void mxt_write(uint16_t addr, uint8_t *data, uint8_t len) {

  Wire.beginTransmission(MXT_ADDR);// Begin tx
  Wire.write((uint8_t)(addr << 8));// Write LSB byte address to MXT112S.
  Wire.write((uint8_t)addr);// Write MSB byte address to MXT112S.

  while (len--)
    Wire.write(*data++);

  Wire.endTransmission();// End tx

}

// Query basic informations of MXT112S
void mxt_identify() {
  uint8_t buf[MXT_ID_BLK_SIZ] = {0};

  mxt_read(0x0000, buf, MXT_ID_BLK_SIZ);

  num_obj = buf[6];// save the object table elements count.

  Serial.print("Family ID 0x");
  Serial.println(buf[0], HEX);
  Serial.print("Variant ID 0x");
  Serial.println(buf[1], HEX);
  Serial.print("Version ");
  Serial.print((uint8_t)((buf[2] & 0xF0) << 4), DEC);
  Serial.print(".");
  Serial.print((uint8_t)(buf[2] & 0x0F), DEC);
  Serial.print("Build Number 0x");
  Serial.println(buf[3], HEX);

  Serial.print("X channels :");
  Serial.println(buf[4], DEC);
  Serial.print("Y channels :");
  Serial.println(buf[5]);

  Serial.print("OJB table elements count");
  Serial.println(buf[6], DEC);

}

void setup() {
  // put your setup code here, to run once:0
  Serial.begin(115200);// Init UART.
  Wire.setClock(100000);// 100kHz I2C clock
  Wire.begin();// Init I2C

  delay(100);
  mxt_identify();
}

void loop() {
  // put your main code here, to run repeatedly:

}
