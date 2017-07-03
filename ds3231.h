#include <Wire.h>
 
#define DS3231_I2C_ADDRESS 104
 
// SCL - pin A5
// SDA - pin A4
 
byte seconds, minutes, hours, day, date, month, year;
char weekDay[4];
 
byte tMSB, tLSB;
float temp3231;

// 10진수를 2진화 10진수인 BCD 로 변환 (Binary Coded Decimal)
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}

void DS3231_set(void) 
{ 
  uint8_t i, century; 
  uint8_t TimeDate[7] = { 0,6,3, 1, 18, 10, 16 }; 

  if (year > 2000) { 
    century = 0x80; 
    year = year - 2000; 
  } else { 
    century = 0; 
    year = year - 1900; 
  }

  Wire.beginTransmission(DS3231_I2C_ADDRESS); 
  Wire.write(0x00); 
  for (i = 0; i <= 6; i++) { 
    TimeDate[i] = decToBcd(TimeDate[i]); 
    if (i == 5) 
      TimeDate[5] += century; 
      Wire.write(TimeDate[i]); 
    } 
    Wire.endTransmission(); 
} 

void get3231Date()
{
  Wire.begin();
	
  // send request to receive data starting at register 0
  Wire.beginTransmission(DS3231_I2C_ADDRESS); // 104 is DS3231 device address
  Wire.write(0x00); // start at register 0
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7); // request seven bytes
 
  if(Wire.available()) {
    seconds = Wire.read(); // get seconds
    minutes = Wire.read(); // get minutes
    hours   = Wire.read();   // get hours
    day     = Wire.read();
    date    = Wire.read();
    month   = Wire.read(); //temp month
    year    = Wire.read();
       
    seconds = (((seconds & B11110000)>>4)*10 + (seconds & B00001111)); // convert BCD to decimal
    minutes = (((minutes & B11110000)>>4)*10 + (minutes & B00001111)); // convert BCD to decimal
    hours   = (((hours & B00110000)>>4)*10 + (hours & B00001111)); // convert BCD to decimal (assume 24 hour mode)
    day     = (day & B00000111); // 1-7
    date    = (((date & B00110000)>>4)*10 + (date & B00001111)); // 1-31
    month   = (((month & B00010000)>>4)*10 + (month & B00001111)); //msb7 is century overflow
    year    = (((year & B11110000)>>4)*10 + (year & B00001111));
  }
  else {
    //oh noes, no data!
  }
 
  switch (day) {
    case 1: strcpy(weekDay, "Sun");
      break;
    case 2: strcpy(weekDay, "Mon");
      break;
    case 3: strcpy(weekDay, "Tue");
      break;
    case 4: strcpy(weekDay, "Wed");
      break;
    case 5: strcpy(weekDay, "Thu");
      break;
    case 6: strcpy(weekDay, "Fri");
      break;
    case 7: strcpy(weekDay, "Sat");
      break;
  }
}

float get3231Temp(void){
  Wire.begin();
  //temp registers (11h-12h) get updated automatically every 64s
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0x11);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 2);
 
  if(Wire.available()) {
    tMSB = Wire.read(); //2's complement int portion
    tLSB = Wire.read(); //fraction portion
   
    temp3231 = (tMSB & B01111111); //do 2's math on Tmsb
    temp3231 += ( (tLSB >> 6) * 0.25 ); //only care about bits 7 & 8
  }
  else {
    //error! no data!
  }
  return temp3231;
}

