/***************************************************
  This is a library for the ST7558 i2c display.

  These displays use I²C to communicate, 2 or 3 pins are required to
  interface (RST is optional)
  
  Based on the driver written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

  It depends on libraries Adafruit-GFX

  Author: Tapia Favio: technicc(at)gmail.com
 
 Pinout:
 
	Lcd chip: ST7558 - Wintek Lcd
	Resolution: 94x64
	Technology: FSTN
	I2C write address: 0x78 - (0c3C with Wire arduino funtion)

    Pin1 Vdd +2.8V		+-----------------------------+
    Pin2 Res (n/c)		|                             |
    Pin3 Sclk			|        Motorola C115        |
    Pin4 Sda 			|            102x65           |
    Pin5 A0 (Gnd)		|       (visible 94x64)       |
    Pin6 Gnd			+-----------------------------+
    Pin7 Vlcd +12V		     |  |  |  |  |  |  |  |
    Pin8 Reset			     1  2  3  4  5  6  7  8 

 ****************************************************/


#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#ifdef __AVR__
 #include <avr/pgmspace.h>
#elif defined(ESP8266)
 #include <pgmspace.h>
#endif

#ifndef _BV
  #define _BV(x) (1 << (x))
#endif

#include <stdlib.h>
#include <Wire.h>
#include "ST7558.h"

 
// the memory framebuffer for the LCD
uint8_t st7558_buffer[918];

#define enablePartialUpdate

#ifdef enablePartialUpdate
static uint8_t xUpdateMin, xUpdateMax, yUpdateMin, yUpdateMax;
#endif

void  ST7558::initBacklight(uint8_t GPIO) {
	BacklightGPIO = GPIO;
	pinMode(BacklightGPIO, OUTPUT );
}
void ST7558::BacklightOn(void) {
	//analogWrite(BacklightGPIO, BlLevel);
	digitalWrite(BacklightGPIO, LOW);
}

void ST7558::BacklightOff(void) {
	//analogWrite(BacklightGPIO, 255)
	digitalWrite(BacklightGPIO, HIGH);
}
void ST7558::SetBacklightLevel(uint8_t level) {
	BlLevel = level; 
	analogWrite(BacklightGPIO, BlLevel);
}
static void updateBoundingBox(uint8_t xmin, uint8_t ymin, uint8_t xmax, uint8_t ymax) {
#ifdef enablePartialUpdate
  if (xmin < xUpdateMin) xUpdateMin = xmin;
  if (xmax > xUpdateMax) xUpdateMax = xmax;
  if (ymin < yUpdateMin) yUpdateMin = ymin;
  if (ymax > yUpdateMax) yUpdateMax = ymax;
#endif
}

ST7558::ST7558( uint8_t rst)
 : Core_GFX(ST7558_WIDTH, ST7558_HEIGHT)
{
 _rst  = rst;
}

inline void ST7558::i2cwrite(uint8_t *data, uint8_t len) {
        
  Wire.beginTransmission(I2C_ADDR_DISPLAY);
  Wire.write(data, len);
  Wire.endTransmission();

}

void ST7558::hwReset(void){
  
  if (_rst!=-1) {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(500);
    digitalWrite(_rst, LOW);
    delay(500);
    digitalWrite(_rst, HIGH);
    delay(500);
  }
}
  
static uint8_t 
          cmd_init[]= {CONTROL_RS_CMD, // Bit de Control A0=0
                                0x2E,                       // MXMY
                                0x21,                       // Extend Set H=1
                                0x12,                       // Bias
                                0xC0,                       // VOP
                                0x0B,                       // Boost
                                0x20,                       // Normal Set H=0
                                0x11,                       // PRS
                                0x00,                       // nop
                                0x40,                       // Y addr
                                0x80,                       // X addr
	  },
          cmd_invert[]= {CONTROL_RS_CMD, 0x20, 0x0D},
          cmd_on[]= {CONTROL_RS_CMD, 0x20, 0x0C},
          cmd_off[]= {CONTROL_RS_CMD, 0x20, 0x08},
          zero16[] = { CONTROL_RS_RAM, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void ST7558::displayOff(void){

  i2cwrite(cmd_off, sizeof(cmd_off));
}
void ST7558::displayOn(void){
  i2cwrite(cmd_on,sizeof(cmd_on));
}

void ST7558::setAddrXY(uint8_t x, uint8_t y){

  uint8_t cmdXY[]={CONTROL_RS_CMD, 0x20, colstart+x, rowstart+y};  
  
  i2cwrite( cmdXY, sizeof(cmdXY) );
}

void ST7558::setContrast(uint8_t val) {
    
  if (val > 0x7f) {
    val = 0x7f;
  }
  
  uint8_t cmd[]={CONTROL_RS_CMD, 
                           ST7558_FUNCTIONSET | ST7558_EXTENDEDINSTRUCTION, 
                           ST7558_SETVOP | val, 
                           ST7558_FUNCTIONSET
  };

  i2cwrite(cmd, sizeof(cmd));
  
}


void ST7558::init(void) {
  
  
  Wire.begin();

  colstart= 0x80;
  rowstart= 0x40;
  
  hwReset();
  
  i2cwrite(cmd_init, sizeof(cmd_init));
  
  // set up a bounding box for screen updates
  updateBoundingBox(0, 0, _width-1, _height-1);
  
}

void ST7558::display(void) {
  uint8_t col, maxcol, p, maxPages;
  uint8_t buff[17], i;
  
  maxPages=uint8_t(1+(_height-1)/8) ;
  buff[0]= CONTROL_RS_RAM;
  
  //for(p = 0; p < _height/8; p++) {
  for(p = 0; p < maxPages; p++) {
#ifdef enablePartialUpdate
    // check if this page is part of update
    if ( yUpdateMin >= ((p+1)*8) ) {
      continue;   // nope, skip it!
    }
    if (yUpdateMax < p*8) {
      break;
    }
#endif

#ifdef enablePartialUpdate
    col = xUpdateMin;
    maxcol = xUpdateMax;
#else
    // start at the beginning of the row
    col = 0;
    maxcol = _width-1;
#endif
    
    setAddrXY(col, p);
    
    while(col<= maxcol ){
      for(i=1; (i<17)&&(col<= maxcol); i++,col++)
        buff[i]=st7558_buffer[(_width*p)+col];
      i2cwrite(buff, i);
    }
  }
  
  displayOn();
  
  setAddrXY(0, 0);

  //command(PCD8544_SETYADDR );  // no idea why this is necessary but it is to finish the last byte?
#ifdef enablePartialUpdate
  xUpdateMin = _width - 1;
  xUpdateMax = 0;
  yUpdateMin = _height-1;
  yUpdateMax = 0;
#endif

}

void ST7558::drawPixel(int16_t x, int16_t y,  uint16_t color) {
  
  if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;
  /// Switch Y orientation Y 
  y = _height - y;

  int16_t t;
  switch(rotation){
    case 1:
      t = x;
      x = y;
      y =  _height - 1 - t;
      break;
    case 2:
      x = _width - 1 - x;
      y = _height - 1 - y;
      break;
    case 3:
      t = x;
      x = _width - 1 - y;
      y = t;
      break;
  }

  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
    return;

  // x is which column
  if (!color) 
    st7558_buffer[x+ (y/8)*_width] |= _BV(y%8);  
  else
    st7558_buffer[x+ (y/8)*_width] &= ~_BV(y%8); 
  
  updateBoundingBox(x,y,x,y);
}

uint8_t ST7558::getPixel(int8_t x, int8_t y) {
  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
    return 0;

  return (st7558_buffer[x+ (y/8)*_width] >> (y%8)) & 0x1;  
}

uint8_t ST7558::getPixel(int8_t x, int8_t y, const uint8_t *bitmap, uint8_t w, uint8_t h) {
  if ((x < 0) || (x >= w) || (y < 0) || (y >= h))
    return 0;

  return (pgm_read_byte(bitmap + (y/8)*w + x) >> (y%8)) & 0x1;  
}

void ST7558::drawFastVLine(int16_t x, int16_t y, int16_t h,  uint16_t color){
  
  if((x >= _width) || (y >= _height)) return;
  if((y+h-1) >= _height) h = _height-y;
  
  for(uint8_t i=y; i<y+h; i++)
     drawPixel(x, i, color);
}

void ST7558::drawFastHLine(int16_t x, int16_t y, int16_t w,  uint16_t color){
  
  if((x >= _width) || (y >= _height)) return;
  if((x+w-1) >= _width)  w = _width-x;
  
  for(uint8_t i=x; i<x+w; i++)
     drawPixel(i, y, color);
}

void ST7558::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color) {
  
  int16_t i, j;
  
  if((x >= _width) || (y >= _height)) return;
  if((x + w - 1) >= _width)  w = _width  - x;
  if((y + h - 1) >= _height) h = _height - y;
  
  for(j=h; j>0; j--) {
    for(i=w; i>0; i--) {
      drawPixel(x+i, y+j, color);
    }
  }
}

void ST7558::invertDisplay(boolean i){
   
   if(i==true)
     i2cwrite(cmd_invert, sizeof(cmd_invert));
   else if(i==false)
     i2cwrite(cmd_on, sizeof(cmd_on));
}

// clear everything
void ST7558::clearDisplay(void) {
  memset(st7558_buffer, 0, sizeof(st7558_buffer));
  updateBoundingBox(0, 0, _width-1, _height-1);
  cursor_y = cursor_x = 0;
}
