/*
 * DHT22.c
 *
 * Created: 1/24/2026 11:55:40 AM
 * Author : Alex Mertz
 * This program operates the DHT22 temperature and humidity sensor using an AVR Atmega328P MCU 
 * This program also creates the functions for interacting with a 2-line 16-character LCD 
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

//Pin Mapping 
/*
Port B:
PB0: Status LED
PB1: DB3 (LCD Display)
PB2: DB4 (LCD Display)
PB3: DB5 (LCD Display)
PB4: DB6 (LCD Display)
PB5: DB7 (LCD Display) 
PB6: XTAL1 (16 MHz Clock)
PB7: XTAL2 (16 MHz Clock)

Port C:
PC2:
PC3: 
PC5: SCLK for I2C
PC4: SDA for I2C
PC5: Need to change PB3 to here
PC6: Reset pin for Flashing

PORT D:
PD0: RS (LCD Display)
PD1: R/W (LCD Display) May exclude
PD2: E Chip Enable (LCD Display)
PD3: DB0 (LCD Display)
PD4: DB1 (LCD Display)
PD5: DB2 (LCD Display)
PD6: VO Contrast Adjust (LCD Display)
PD7: DHT22 IC
*/


//DHT22 port/pin definitions (PORT D, Pin 7) 
#define DHT22DDR DDRD
#define DHT22In PIND
#define DHT22Port PORTD
#define DHT22Pin PD7

//LCD port/pins definitions
#define LCD_DDR1 DDRD
#define LCD_Port1 PORTD
#define LCD_Pin1 PIND
#define LCD_DDR2 DDRB
#define LCD_Port2 PORTB
#define LCD_PIN2 PINB
#define LCD_E PD2
#define LCD_VO PD6
#define LCD_RW PD1
#define LCD_RS PD0
#define LCD_DB0 PD3
#define LCD_DB1 PD4
#define LCD_DB2 PD5
#define LCD_DB3 PB1
#define LCD_DB4 PB2
#define LCD_DB5 PB3
#define LCD_DB6 PB4
#define LCD_DB7 PB5

//MISC port/Pins definitions (PORT B, Pin 0)
#define StatusLEDDDR DDRB
#define StatusLEDIn PINB
#define StatusLEDPort PORTB
#define StatusLEDPin PB0

//DHT22 Functions
bool DHT22Initialize (void);
bool readDHT22 (uint8_t *data);
void displayDHT22 (uint8_t *data);
void verifyDHT22 (uint8_t *data);

//LCD Functions
void LCDInitialize (void);
void LCDInstruction (uint8_t instruction, char RS);
void LCDWrite (uint8_t *output); 

int main(void)
{
bool readSuccess;	
uint8_t DHT22Data[5];
// Data Direction Register B: turns bit 0 (PB0) into output mode for the status LED
StatusLEDDDR |= 0b00000001; 

//runs the initialization routine until the DHT22 sensor is appropriately initiated  
while (!DHT22Initialize()); 
//Delays the program for 80us to allow the DHT22 to change to data sending mode 
_delay_us(80); 

    while (1) 
    {
		do
		{
		readSuccess = readDHT22(DHT22Data);
		}while (!readSuccess);
			;
		LCDWrite(DHT22Data);
		_delay_ms(2000);
    }
}

bool DHT22Initialize(void){
//Initializes DHT22 sensor into output mode
// Data Direction Register D: turns bit 7 (PD7) into output mode and pulls PD7 to a low level
// for 1.5ms in order initialize the DHT22
DHT22DDR |= (1 << DHT22Pin);

	DHT22Port &= ~(1 << DHT22Pin);
	_delay_ms(1.5);
	DHT22Port |= (1 << DHT22Pin);
	//Delays program progression for 30us to allow for DHT22 response as per DHT22 data sheet requirements
	//Transitions PD7 to input mode for response from DHT22 
	_delay_us(30);
	DHT22DDR &= ~(1 << DHT22Pin);
	
	//Delays for 80us prior to allow DHT22 time to response 
	_delay_us(80);
	//Checks if the DHT22 responses as expected as per data sheet (Should pull the voltage back low)
	if (!(DHT22In & (1 << DHT22Pin)))
	{
		//lights up LED at PB0 to indicate successful initialization
		StatusLEDPort |= (1<< StatusLEDPin);
		return true;
	}
	else{
		return false;
	}
}

bool readDHT22(uint8_t *DHT22Data){
//This function reads the data out from the DHT22 
// data is sent in a 40 bit format:
// 16 bits of data about humidity (2 bytes)
// 16 bits of data about temperature (2 bytes)
// 8 bits for checksum purposes (1 byte) 
//MSB is sent first, so data has to be read in MSB -> LSB

uint8_t bitCounter = 7;
uint8_t byteCounter = 0;

//Clear Data buffer of any previous data
for (uint8_t i = 0; i< 5; i++){
	DHT22Data[i]=0;
}

//load in data
		for (uint8_t j = 0; j<40;j++)
		{
			 // Wait for LOW
			 while (DHT22In & (1 << DHT22Pin));
			 // Wait for HIGH
			 while (!(DHT22In & (1 << DHT22Pin)));
			 /* Sample after 50us if the value is already high (0 bits stays high for 26us after initial 40us
			    verse 1 bits which stays high for 70us, the bit is a 1, else it is a zero. 
			    Since the buffer is already cleared to zero at start, no action is needed */
			 
			 _delay_us(50);	
			 if (DHT22In & (1 << DHT22Pin))
			 {
				DHT22Data[byteCounter] |= (1 << bitCounter);
			 } 
			 if (bitCounter == 0) {
				 byteCounter++;
				 bitCounter = 7; 
			 }
			 else
			 {
			 bitCounter--;
			 }
			 // Wait for HIGH to finish to ensure no issues for next loop
			 while (DHT22In & (1 << DHT22Pin));
	}
	//Verify the (data % 256) is == to the checksum to ensure proper transmission
	if ( ((DHT22Data[0]+DHT22Data[1]+DHT22Data[2]+DHT22Data[3]) % 256) == DHT22Data[4])
	{
		return true;
	}
	else return false;
	}

void LCDInitialize (void){
//Initializes the LCD for outputting data
// Step 1: Sets the instruction register to 8 bit mode, 2 lines
// 5x8 font 
// Step 2: Turns the display off for initialization
// Step 3: Clears DDRAM of the display
// Step 4: Sets cursor to move to rightwards and turns off display shift
// Step 5: Turns the display on
//Places all LCD pins into output mode

//PORTD
LCD_DDR1 |= 0b01111111;

//PORTB
LCD_DDR2 |= 0b00111110;

	//pause required from power start to LCD and first command,
	//ensures no issues
	_delay_ms(45);
	
	// Step 1: Sets the instruction register to 8 bit mode, 2 lines
	// 5x8 font
	// RW and RS are pulled low to allow for IR setting
	// E, chip select must be pulsed low to allow for command latching 
	//DB5, DB4 and DB3 high and DB6 and DB7 low
	LCDInstruction(0x38,0);
	//Step 2: Turn the display off
	//DB3 high, DB2, DB1
	LCDInstruction(0x08,0);
	//Step 3: Clear DDRAM 
	LCDInstruction(0x01,0);
	//Step 4: Set cursor, turn off shift
	LCDInstruction(0x06,0);
	//Step 5: Turn Display back on
	LCDInstruction(0x0C,0);
}

void LCDWrite (uint8_t *output){
//Outputs the passed argument to the LCD
//Expects data from DHT22 which is a 18 char array 
char outputStr[17];

//Outputs humidity as 0.1% RH
uint16_t humidity = ((output[0] << 8 | output[1]))/10; 
//Outputs temperature as 0.1% C
uint16_t temperature = ((output[2]<< 8 | output [3]))/10;

//Convert the output uint16_ts into strings using sprintf
sprintf(outputStr,"Humidity:%2d%%", humidity);
//moves cursor to first line
LCDInstruction(0x80,0);

for (uint8_t i = 0; i<16 && outputStr[i]; i++){
	LCDInstruction(outputStr[i],1);
}

sprintf(outputStr,"Temperature:%2dC",temperature);
LCDInstruction(0xC0,0); //moves cursor to second line

for (uint8_t i = 0; i<16 && outputStr[i];i++){
	LCDInstruction(outputStr[i],1);
}
}

void LCDInstruction (uint8_t instruction,char RS){
//converts a hex instruction code into binary for the IR
//Also can be used for writing characters to the LCD by changing RS to 1 
//DB7(MSB) -> DB0 (LSB)

if (RS == 1) LCD_Port1 |= (1 << LCD_RS);
else LCD_Port1 &= ~(1 << LCD_RS);

LCD_Port1 &= ~(1 << LCD_RW);

//PORTD PORT1
//Clears pins first
LCD_Port1 &= ~(1 << LCD_DB0 | 1 << LCD_DB1 | 1 << LCD_DB2);
LCD_Port1 |= (instruction & 0x07);

//PORTB Port2
//Clears pins first
LCD_Port2 &= ~(1 << LCD_DB3 | 1 << LCD_DB4 | 1 << LCD_DB5 | 1 << LCD_DB6 | 1 << LCD_DB7);
LCD_Port2 |= ((instruction & 0x38)>> 3) << 1;

if (instruction & (1 << 6)) LCD_Port2 |= (1 << LCD_DB6); 
if (instruction & (1 << 7)) LCD_Port2 |= (1 << LCD_DB7);


//Pulses E and clears IR of prior command
	//E pulse
	LCD_Port1 |= (1 << LCD_E);
	_delay_us(1);
	LCD_Port1 &= ~(1 << LCD_E);
	//to account for propagation delay
	_delay_us(40);

	//Clears data from IR 
	LCD_Port1 &= ~(1 << LCD_DB0 | 1 << LCD_DB1 | 1<< LCD_DB2);
	LCD_Port2 &= ~(1 << LCD_DB3 | 1 << LCD_DB4 | 1<< LCD_DB5 | 1<< LCD_DB6 | 1<< LCD_DB7); 
}
