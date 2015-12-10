/*
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Core invests time and resources providing this open source code, please
support Core & open-source hardware by purchasing products from Core!
 
Copyright (c) 2013 Core Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include "Core_GFX.h"
#include "glcdfont.h"
#ifdef __AVR__
 #include <avr/pgmspace.h>
#elif defined(ESP8266)
 //#include <pgmspace.h>
#else
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

#ifndef min
 #define min(a,b) ((a < b) ? a : b)
#endif

Core_GFX::Core_GFX(int16_t w, int16_t h):
  WIDTH(w), HEIGHT(h)
{
  _width    = WIDTH;
  _height   = HEIGHT;
  rotation  = 0;
  cursor_y  = cursor_x    = 0;
  textsize  = 1;
  textcolor = textbgcolor = 0xFFFF;
  wrap      = true;
  
}

// Draw a circle outline
void Core_GFX::drawCircle(int16_t x0, int16_t y0, int16_t column,
    uint16_t color) {
  int16_t f = 1 - column;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * column;
  int16_t x = 0;
  int16_t y = column;

  drawPixel(x0  , y0+column, color);
  drawPixel(x0  , y0-column, color);
  drawPixel(x0+column, y0  , color);
  drawPixel(x0-column, y0  , color);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
  }
}

void Core_GFX::drawCircleHelper( int16_t x0, int16_t y0,
               int16_t column, uint8_t cornername, uint16_t color) {
  int16_t f     = 1 - column;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * column;
  int16_t x     = 0;
  int16_t y     = column;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4) {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    } 
    if (cornername & 0x2) {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

void Core_GFX::fillCircle(int16_t x0, int16_t y0, int16_t column,
			      uint16_t color) {
  drawFastVLine(x0, y0-column, 2*column+1, color);
  fillCircleHelper(x0, y0, column, 3, 0, color);
}

// Used to do circles and roundrects
void Core_GFX::fillCircleHelper(int16_t x0, int16_t y0, int16_t column,
    uint8_t cornername, int16_t delta, uint16_t color) {

  int16_t f     = 1 - column;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * column;
  int16_t x     = 0;
  int16_t y     = column;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) {
      drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
      drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
      drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}

// Bresenham's algorithm - thx wikpedia
void Core_GFX::drawLine(int16_t x0, int16_t y0,
			    int16_t x1, int16_t y1,
			    uint16_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

// Draw a rectangle
void Core_GFX::drawRect(int16_t x, int16_t y,
			    int16_t w, int16_t h,
			    uint16_t color) {
  drawFastHLine(x, y, w, color);
  drawFastHLine(x, y+h-1, w, color);
  drawFastVLine(x, y, h, color);
  drawFastVLine(x+w-1, y, h, color);
}

void Core_GFX::drawFastVLine(int16_t x, int16_t y,
				 int16_t h, uint16_t color) {
  // Update in subclasses if desired!
  drawLine(x, y, x, y+h-1, color);
}

void Core_GFX::drawFastHLine(int16_t x, int16_t y,
				 int16_t w, uint16_t color) {
  // Update in subclasses if desired!
  drawLine(x, y, x+w-1, y, color);
}

void Core_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
			    uint16_t color) {
  // Update in subclasses if desired!
  for (int16_t i=x; i<x+w; i++) {
    drawFastVLine(i, y, h, color);
  }
}

void Core_GFX::fillScreen(uint16_t color) {
  fillRect(0, 0, _width, _height, color);
}

// Draw a rounded rectangle
void Core_GFX::drawRoundRect(int16_t x, int16_t y, int16_t w,
  int16_t h, int16_t column, uint16_t color) {
  // smarter version
  drawFastHLine(x+column  , y    , w-2*column, color); // Top
  drawFastHLine(x+column  , y+h-1, w-2*column, color); // Bottom
  drawFastVLine(x    , y+column  , h-2*column, color); // Left
  drawFastVLine(x+w-1, y+column  , h-2*column, color); // Right
  // draw four corners
  drawCircleHelper(x+column    , y+column    , column, 1, color);
  drawCircleHelper(x+w-column-1, y+column    , column, 2, color);
  drawCircleHelper(x+w-column-1, y+h-column-1, column, 4, color);
  drawCircleHelper(x+column    , y+h-column-1, column, 8, color);
}

// Fill a rounded rectangle
void Core_GFX::fillRoundRect(int16_t x, int16_t y, int16_t w,
				 int16_t h, int16_t column, uint16_t color) {
  // smarter version
  fillRect(x+column, y, w-2*column, h, color);

  // draw four corners
  fillCircleHelper(x+w-column-1, y+column, column, 1, h-2*column-1, color);
  fillCircleHelper(x+column    , y+column, column, 2, h-2*column-1, color);
}

// Draw a triangle
void Core_GFX::drawTriangle(int16_t x0, int16_t y0,
				int16_t x1, int16_t y1,
				int16_t x2, int16_t y2, uint16_t color) {
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

// Fill a triangle
void Core_GFX::fillTriangle ( int16_t x0, int16_t y0,
				  int16_t x1, int16_t y1,
				  int16_t x2, int16_t y2, uint16_t color) {

  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1); swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1); swap(x0, x1);
  }

  if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if(x1 < a)      a = x1;
    else if(x1 > b) b = x1;
    if(x2 < a)      a = x2;
    else if(x2 > b) b = x2;
    drawFastHLine(a, y0, b-a+1, color);
    return;
  }

  int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
  int32_t
    sa   = 0,
    sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2) last = y1;   // Include y1 scanline
  else         last = y1-1; // Skip it

  for(y=y0; y<=last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    drawFastHLine(a, y, b-a+1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y<=y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if(a > b) swap(a,b);
    drawFastHLine(a, y, b-a+1, color);
  }
}

void Core_GFX::drawBitmap(int16_t x, int16_t y,
			      const uint8_t *bitmap, int16_t w, int16_t h,
			      uint16_t color) {

  int16_t i, j, byteWidth = (w + 7) / 8;

  for(j=0; j<h; j++) {
    for(i=0; i<w; i++ ) {
      if(pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
        drawPixel(x+i, y+j, color);
      }
    }
  }
}

// Draw a 1-bit color bitmap at the specified x, y position from the
// provided bitmap buffer (must be PROGMEM memory) using color as the
// foreground color and bg as the background color.
void Core_GFX::drawBitmap(int16_t x, int16_t y,
            const uint8_t *bitmap, int16_t w, int16_t h,
            uint16_t color, uint16_t bg) {

  int16_t i, j, byteWidth = (w + 7) / 8;
  
  for(j=0; j<h; j++) {
    for(i=0; i<w; i++ ) {
      if(pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
        drawPixel(x+i, y+j, color);
      }
      else {
      	drawPixel(x+i, y+j, bg);
      }
    }
  }
}

//Draw XBitMap Files (*.xbm), exported from GIMP,
//Usage: Export from GIMP to *.xbm, rename *.xbm to *.c and open in editor.
//C Array can be directly used with this function
void Core_GFX::drawXBitmap(int16_t x, int16_t y,
                              const uint8_t *bitmap, int16_t w, int16_t h,
                              uint16_t color) {
  
  int16_t i, j, byteWidth = (w + 7) / 8;
  
  for(j=0; j<h; j++) {
    for(i=0; i<w; i++ ) {
      if(pgm_read_byte(bitmap + j * byteWidth + i / 8) & (1 << (i % 8))) {
        drawPixel(x+i, y+j, color);
      }
    }
  }
}

#if ARDUINO >= 100
size_t Core_GFX::write(uint8_t c) {
#else
void Core_GFX::write(uint8_t c) {
#endif
	if (c == '\n') {
    cursor_y += textsize*8;
    cursor_x  = 0;
  } else if (c == '\c') {
    // skip em
  }
  else if (c == 0x0D)
  {
	  cursor_x = 0;
  }
  else {
	  
	  ///
	  
	  if (((c == 0xD0 )||(c == 0xD1)) & (utf8handle==0x00) ) {
	  utf8handle = c;

#if ARDUINO >= 100
			return 1;
#endif

		exit;
	  
	  };
	  
	  if ( utf8handle != 0x00) {
		  
		   switch (utf8handle)
			  {
			  case 0xD0:
				  switch (c)
				  {
				  case 0x81:
					  //Ё 
					  c = 143;
					  break;
				  case 0x86:
					 // І Ukrain
					  c = 178;
					  break;

				  case 0x87:
					  //Ї Ukrain
					  c = 175;
					  break;
				  default:
					  c += 48;
					  break;
				  }
				  break;
			  case 0xD1:
				  switch (c)
				  {
				  case 0x91:
					  //ё
					  c = 184;
					  break;
				  case 0x96:
						//і Ukrain
					  c = 179;
						  break;

				  case 0x97:
					  //ї
					  c = 191;
					  break;
				  
				  default:
					  c += 112;
					  break;
				  }
				  			  
				  break;

			  }
		   utf8handle = 0x00;
				}

    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
   // в зависимости от ориентации текста в какую сторону будем писать следующий символ
	switch (textdirection)
	{
	case 1: //	From top to down
		cursor_y += textsize * 8;
		break;
	case 2: //	From right to left
		cursor_x += textsize * 6;
		if (wrap && (cursor_x < 0)) {
			cursor_y += textsize * 8;
			cursor_x = _width - textsize * 6;
		}
		break;
	case 0: //	Normal text direction left to right
		cursor_x += textsize * 6;
		if (wrap && (cursor_x > (_width - textsize * 6))) {
			cursor_y += textsize * 8;
			cursor_x = 0;
		}
		break;
	}
  }
#if ARDUINO >= 100
  return 1;
#endif
}




// Draw a character
void Core_GFX::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
	uint8_t row,column,FontHeight,FontWidth;
	uint8_t X,Y,F,Z;
	uint8_t tmp;
	
	c-=32;
	// x ширина x+5 точек символ
	FontHeight=8 * textsize;
	FontWidth=5 * textsize;
	for (column=0; column<FontWidth;column++ )
	{
		 
		Z=column/ textsize;
		tmp= pgm_read_byte(&font5x8[c][Z]);
		
		for (row= 0;row<FontHeight;row++)
		{
			F = (row / size);
			X = x + column;
			Y = y + row;
			if ((tmp & (1 << F)) != 0) {
				drawPixel(X, Y, color);
			}
			else if (bg != color)
			{
				drawPixel(X, Y, bg);
			};
		}
	} 

  }

  
  
  
void Core_GFX::setCursor(int16_t x, int16_t y) {
  cursor_x = x;
  cursor_y = y;
}

int16_t Core_GFX::getCursorX(void) const {
  return cursor_x;
}

int16_t Core_GFX::getCursorY(void) const {
  return cursor_y;
}

void Core_GFX::setTextSize(uint8_t s) {
  textsize = (s > 0) ? s : 1;
}

void Core_GFX::setTextColor(uint16_t c) {
  // For 'transparent' background, we'll set the bg 
  // to the same as fg instead of using a flag
  textcolor = textbgcolor = c;
}

void Core_GFX::setTextColor(uint16_t c, uint16_t b) {
  textcolor   = c;
  textbgcolor = b; 
}

void Core_GFX::setTextWrap(boolean w) {
  wrap = w;
}

uint8_t Core_GFX::getRotation(void) const {
  return rotation;
}

void Core_GFX::setRotation(uint8_t x) {
  rotation = (x & 3);
  switch(rotation) {
   case 0:
   case 2:
    this->_width  = WIDTH;
	this->_height = HEIGHT;
    break;
   case 1:
   case 3:
	   _width  = HEIGHT;
	   _height = WIDTH;
    break;
  }
}

void  Core_GFX::setTextDirection(uint8_t d){
	
	textdirection = (d & 3 );

}


// Return the size of the display (per current rotation)
int16_t Core_GFX::width(void) const{
  return _width;
}
 
int16_t Core_GFX::height(void) const {
  return _height;
}

void Core_GFX::invertDisplay(boolean i) {
  // Do nothing, must be subclassed if supported
}

/***************************************************************************/
// code for the GFX button UI element

Core_GFX_Button::Core_GFX_Button(void) {
   _gfx = 0;
}

void Core_GFX_Button::initButton(Core_GFX *gfx,
					  int16_t x, int16_t y, 
					  uint8_t w, uint8_t h, 
					  uint16_t outline, uint16_t fill, 
					  uint16_t textcolor,
					  char *label, uint8_t textsize)
{
  _x = x;
  _y = y;
  _w = w;
  _h = h;
  _outlinecolor = outline;
  _fillcolor = fill;
  _textcolor = textcolor;
  _textsize = textsize;
  _gfx = gfx;
  strncpy(_label, label, 9);
  _label[9] = 0;
}

 

 void Core_GFX_Button::drawButton(boolean inverted) {
   uint16_t fill, outline, text;

   if (! inverted) {
     fill = _fillcolor;
     outline = _outlinecolor;
     text = _textcolor;
   } else {
     fill =  _textcolor;
     outline = _outlinecolor;
     text = _fillcolor;
   }

   _gfx->fillRoundRect(_x - (_w/2), _y - (_h/2), _w, _h, min(_w,_h)/4, fill);
   _gfx->drawRoundRect(_x - (_w/2), _y - (_h/2), _w, _h, min(_w,_h)/4, outline);
   
   
   _gfx->setCursor(_x - strlen(_label)*3*_textsize, _y-4*_textsize);
   _gfx->setTextColor(text);
   _gfx->setTextSize(_textsize);
   _gfx->print(_label);
 }

boolean Core_GFX_Button::contains(int16_t x, int16_t y) {
   if ((x < (_x - _w/2)) || (x > (_x + _w/2))) return false;
   if ((y < (_y - _h)) || (y > (_y + _h/2))) return false;
   return true;
 }


 void Core_GFX_Button::press(boolean p) {
   laststate = currstate;
   currstate = p;
 }
 
 boolean Core_GFX_Button::isPressed() { return currstate; }
 boolean Core_GFX_Button::justPressed() { return (currstate && !laststate); }
 boolean Core_GFX_Button::justReleased() { return (!currstate && laststate); }
