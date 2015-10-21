# ST7558
 
 
ST7558 lcd library for Arduino ESP8266


Motorola C115 Display, chipset ST7558, compatible with:

     Motorola C113/C115/C116/C118/C123
     
## Pinouts
	Lcd chip: ST7558 - Wintek Lcd
	Resolution: 94x64
	Technology: FSTN
	I2C write address: 0x78 - (0x3C with *Wire* arduino funtion)

    Pin1 Vdd +2.8V		.-----------------------------.
    Pin2 Res (n/c)		|                             |
    Pin3 Sclk			|        Motorola C115        |
    Pin4 Sdat 			|            102x65           |
    Pin5 A0 (Gnd)		|       (visible 94x64)       |
    Pin6 Gnd			'-----------------------------'
    Pin7 Vlcd +12V		     |  |  |  |  |  |  |  |
    Pin8 Reset			     1  2  3  4  5  6  7  8 

##3d Cube example video:

https://www.youtube.com/watch?v=zOoVFpy9CIY

### Author
Tapia Favio <technicc@gmail.com>
## Editor
Valera Serzhantov <valera.serjantov@hotmail.com>
