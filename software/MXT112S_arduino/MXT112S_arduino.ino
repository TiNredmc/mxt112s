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
// Object query address.
uint16_t obj_addr_cnt = 0;

/* Register map*/
// Store address of each object.
uint16_t T5_addr = 0;
uint16_t T7_addr = 0;
uint16_t T9_addr = 0;

// TODO : properly build report ID map.
// Store min and max report ID of T9
uint8_t T9_report_id_min = 0;
uint8_t T9_report_id_max = 0;

// Genral purpose buffer array.
uint8_t MXT_BUF[256];

uint16_t xpos, ypos;
uint8_t amplitude;

// Read data from MXT112S
void mxt_read(uint16_t addr, uint8_t *data, uint16_t len) {

  Wire.beginTransmission(MXT_ADDR);// Begin TX
  Wire.write((uint8_t)addr);// Write LSB byte address to MXT112S.
  Wire.write((uint8_t)(addr >> 8));// Write MSB byte address to MXT112S.

  Wire.endTransmission();

  Wire.requestFrom(MXT_ADDR, len);// Begin RX
  while (Wire.available() && len--) { //
    *data++ = Wire.read();
  }

}

// Write data to MXT112S
void mxt_write(uint16_t addr, uint8_t *data, uint8_t len) {

  Wire.beginTransmission(MXT_ADDR);// Begin tx
  Wire.write((uint8_t)addr);// Write LSB byte address to MXT112S.
  Wire.write((uint8_t)(addr >> 8));// Write MSB byte address to MXT112S.

  while (len--)
    Wire.write(*data++);

  Wire.endTransmission();// End tx

}

// Query basic informations of MXT112S
void mxt_identify() {
  mxt_read(0x0000, MXT_BUF, MXT_ID_BLK_SIZ);

  num_obj = MXT_BUF[6];// save the object table elements count.

  Serial.print("Family ID 0x");
  Serial.println(MXT_BUF[0], HEX);
  Serial.print("Variant ID 0x");
  Serial.println(MXT_BUF[1], HEX);
  Serial.print("Version ");
  Serial.print((uint8_t)((MXT_BUF[2] & 0xF0) >> 4), DEC);
  Serial.print(".");
  Serial.println((uint8_t)(MXT_BUF[2] & 0x0F), DEC);
  Serial.print("Build Number 0x");
  Serial.println(MXT_BUF[3], HEX);

  Serial.print("X channels :");
  Serial.println(MXT_BUF[4], DEC);
  Serial.print("Y channels :");
  Serial.println(MXT_BUF[5], DEC);

  Serial.print("OJB table elements count");
  Serial.println(num_obj, DEC);

  // maXTouch can have many object. But we specifically looking for T5 A.K.A Message Processor
  // and T100 A.K.A Multitouch
  //
  // maXTouch can have many object. But we specifically looking for T5 A.K.A Message Processor
  // and T9 Multitouch object
 
  for (uint8_t i = 0; i < num_obj; i ++) {
  mxt_read(obj_addr_cnt, MXT_BUF, MXT_ID_BLK_SIZ - 1);

  switch (MXT_BUF[0]) {
    case 5:// T5 message processor.
      Serial.print("Found T5 object\n");
      T5_addr = (MXT_BUF[2] << 8) | MXT_BUF[1];
      Serial.print("T5 object address : 0x");
      Serial.println(T5_addr, HEX);
      break;

    case 7:// T7 power config stuffs.
      Serial.print("Found T7 Object\n");
      T7_addr = (MXT_BUF[2] << 8) | MXT_BUF[1];
      break;

    case 9:// T9 Multi touch (Assuming the maXTouch S series have this instead of T100 from U series).
      Serial.print("Found T9 object\n");
      T9_addr = (MXT_BUF[2] << 8) | MXT_BUF[1];
      Serial.print("T9 object address : 0x");
      Serial.println(T9_addr, HEX);

      // TODO : properly build report ID map.
      //T9_report_id_min = MXT_BUF[5];
      //T9_report_id_max = MXT_BUF[5] + MXT_BUF[4]; 
      break;

  }
  
  obj_addr_cnt += 6;// move on to next object.
  }

  // Dummy read CRC. We don't actually use it.
  mxt_read(obj_addr_cnt, MXT_BUF, 3);

  // Get X,Y range
  mxt_read(T9_addr + 18, MXT_BUF, 4);
  Serial.print("X range : ");
  Serial.println((MXT_BUF[1] << 8) | MXT_BUF[0], DEC);
  Serial.print("Y range : ");
  Serial.println((MXT_BUF[3] << 8) | MXT_BUF[2], DEC);

  // Set T7 into Free-running mode
  mxt_read(T7_addr, MXT_BUF, 3);
  MXT_BUF[0] = 255;
  MXT_BUF[1] = 255;
  mxt_write(T7_addr, MXT_BUF, 3);
}

// Report Multi touch from T9 object.
// But data is read from T5 message object.
void mxt_report_t9(){
  if(digitalRead(MXT_INT) == 0){
    mxt_read(T5_addr, MXT_BUF, 16);
    Serial.write(0x0C);// clear terminal
    // Convert 2 bytes to 16bit uint X and Y position 
    xpos = (MXT_BUF[2] << 4) | ((MXT_BUF[4] >> 4) & 0x0F);
    ypos = (MXT_BUF[3] << 2) | (MXT_BUF[4] & 0x0F);

    amplitude =  MXT_BUF[6];

    Serial.print("X pos : ");
    Serial.println(xpos, DEC);
    Serial.print("Y pos : ");
    Serial.println(ypos, DEC);
    Serial.print("Pressure : ");
    Serial.println(amplitude, DEC);    
  }
}

void setup() {
  // put your setup code here, to run once:0
  Serial.begin(115200);// Init UART.
  Wire.setClock(100000);// 100kHz I2C clock
  Wire.begin();// Init I2C

  pinMode(MXT_INT, INPUT_PULLUP);
  mxt_identify();
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  mxt_report_t9();
  delay(10);// throttle down a bit. 
}
