/***************************************************

  Simple example using the ST7558 and Adafruit-GFX librarie
  
  Author: Tapia Favio: technicc(at)gmail.com

****************************************************/

#include <pgmspace.h>
#include <Adafruit_GFX.h>
#include <ST7558.h>
#include <Wire.h>

#define RST_PIN 7

ST7558 lcd = ST7558();

extern const unsigned char flecha[];
extern const unsigned char cara[];
extern const unsigned char logo16[];
extern const unsigned char black[];

int8_t i=88;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  Wire.begin(4,5);
  
  lcd.init();
  lcd.setContrast(65);
  lcd.setRotation(0);
  //lcd.display();
  //delay(1500);
  
  lcd.clearDisplay();
  lcd.setTextColor(ST7558_BLACK);
  lcd.setCursor(15,0);
  lcd.print("Привет миру!");
  lcd.setCursor(15,20);
  lcd.setTextSize(2);
  lcd.print("Hello World!");
  
  lcd.setCursor(1,40);
  lcd.setTextSize(3);
  lcd.print("Hello World!");
  lcd.display();
  delay(15000);
  lcd.clearDisplay();
  //lcd.drawBitmap(70, 0, cara,8, 8, ST7558_BLACK);
  //lcd.drawBitmap(0, 0, black, 96, 65, ST7558_BLACK);
  lcd.drawBitmap(69, 0, logo16, 16, 16, ST7558_BLACK);
  //lcd.drawFastVLine(50, 0, ST7558_HEIGHT, ST7558_BLACK);
  //grid();
  //delay(2000);
}

void loop() {
 
}

void grid(){
  uint8_t x, y;
  lcd.clearDisplay();
  for(x=0; x<ST7558_WIDTH; x+=5)
    for(y=0; y<ST7558_WIDTH; y+=5)
      lcd.drawPixel(x, y, ST7558_BLACK);
  lcd.display();
}

