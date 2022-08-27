// Atmel maXTouch MXT112S example code with STM8L
// Ported by TinLethax 2022/08/11 +7

#include <Wire.h>
#include <stdint.h>

/* I2C 7bit address of MXT112S */
#define MXT_ADDR  0x4B

// Interrupt input.
#define MXT_INT 4

#define MXT_ID_BLK_SIZ  7// Query ID size of 7 bytes

// Number of elements in the Object Table
uint8_t num_obj = 0;

// Store address of each object.
uint16_t T5_addr = 0;
uint16_t T9_addr = 0;
//uint16_t T100_addr = 0;

// Store message size of each object.
uint8_t T5_msg_size = 0;
uint8_t T9_msg_size = 0;
//uint8_t T100_msg_size = 0;

// Store number of report instances of each report.
uint8_t T9_instances = 0;
//uint8_t T100_instances = 0;

// Store number of report ID per instance.
uint16_t T9_report_cnt = 0;
//uint16_t T100_report_cnt = 0;

// Genral purpose buffer array
uint8_t MXT_BUF[256];

uint16_t xpos, ypos;
uint8_t amplitude;

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
  Serial.println((uint8_t)(buf[2] & 0x0F), DEC);
  Serial.print("Build Number 0x");
  Serial.println(buf[3], HEX);

  Serial.print("X channels :");
  Serial.println(buf[4], DEC);
  Serial.print("Y channels :");
  Serial.println(buf[5]);

  Serial.print("OJB table elements count");
  Serial.println(buf[6], DEC);

  // maXTouch can have many object. But we specifically looking for T5 A.K.A Message Processor
  // and T100 A.K.A Multitouch
  //
  for (uint8_t i = 0; i < num_obj; i ++) {
    mxt_read(0x0007, buf, MXT_ID_BLK_SIZ - 1);

    switch (buf[0]) {
      case 5:// T5 message processor.
        Serial.println("Found T5 object");
        T5_addr = (buf[2] << 8) | buf[1];
        T5_msg_size = buf[3] + 1;
        break;

      case 9:// T9 Multi touch (Assuming the maXTouch S series have this instead of T100 from U series).
        Serial.println("Found T9 object");
        T9_addr = (buf[2] << 8) | buf[1];
        T9_msg_size = buf[3] + 1;
        T9_instances = buf[4] + 1;
        T9_report_cnt = (buf[4] + 1) * buf[5];
        break;

//      case 100:// T100 Multi touch
//        Serial.println("Found T100 object");
//        T100_addr = (buf[2] << 8) | buf[1];
//        T100_msg_size = buf[3] + 1;
//        T100_instances = buf[4] + 1;
//        T100_report_cnt = (buf[4] + 1) * buf[5];
//        break;
    }
    
  }

}

// Report Multi touch from T9 object.
// But data is read from T5 message object.
// Since T9 can report Upto 10 fingers. 
// The report ID can be more than 1 Report ID.
// Starting from 0x09 (T9 base report ID).
void mxt_report_t9(){
  if(digitalRead(MXT_INT) == 0){
    mxt_read(T5_addr, MXT_BUF, T9_msg_size);
  
    if(MXT_BUF[0] != 9){// If we didn't get T9 report (check for the T9 report ID).
      // Just ignore the interrupt.
      return;      
    }

    // Check status from first byte.
    if(MXT_BUF[1] & 0x02)// If touch has been suppressed by Grip or Face (imagine put your phone to your ear).
     // Just ignore
      return;

    // Convert 2 bytes to 16bit uint X and Y position 
    xpos = (MXT_BUF[0x02] << 4) | ((MXT_BUF[0x04] >> 4) & 0x0F);
    ypos = (MXT_BUF[0x03] << 4) | (MXT_BUF[0x04] & 0x0F);

    // Check for touch amplitude (Pressure?)
    if(MXT_BUF[1] & 0x04)
      amplitude =  MXT_BUF[0x06];
  }
}

void setup() {
  // put your setup code here, to run once:0
  Serial.begin(115200);// Init UART.
  Wire.setClock(100000);// 100kHz I2C clock
  Wire.begin();// Init I2C

  pinMode(MXT_INT, INPUT_PULLUP);

  delay(100);
  mxt_identify();
}

void loop() {
  // put your main code here, to run repeatedly:

}
