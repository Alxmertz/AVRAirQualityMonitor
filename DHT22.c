/*
 * DHT22.c
 *
 * Created: 1/12/2026 11:55:40 AM
 * Author : Alex Mertz
 * This program operates the DHT22 temperature and humidity sensor using an AVR Atmega328P MCU 
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

bool DHT22Initialize (void);
bool readDHT22 (uint8_t *data);
void displayDHT22 (uint8_t *data);
void verifyDHT22 (uint8_t *data);

int main(void)
{
bool readSuccess;	
uint8_t DHT22Data[5];
// Data Direction Register B: turns bit 0 (PB0) into output mode
DDRB |= 0b00000001; 

//runs the initialization routine until the DHT22 sensor is appropriately initiated  
while (!DHT22Initialize()); 
//Delays the program for 80us to allow the DHT22 to change to data sending mode 
_delay_us(80); 

    while (1) 
    {
		do
		{
		readSuccess = readDHT22(DHT22Data)
		}while (!readSuccess);
			;
		displayDHT22(DHT22Data);
    }
}

bool DHT22Initialize(void){
//Initializes DHT22 sensor into output mode
// Data Direction Register D: turns bit 7 (PD7) into output mode and pulls PD7 to a low level
// for 1.5ms in order initialize the DHT22
DDRD |= (1 << 7);

	PORTD &= ~(1 << 7);
	_delay_ms(1.5);
	PORTD |= (1 << 7);
	//Delays program progression for 30us to allow for DHT22 response as per DHT22 data sheet requirements
	//Transitions PD7 to input mode for response from DHT22 
	_delay_us(30);
	DDRD &= ~(1 << 7);
	
	//Delays for 80us prior to allow DHT22 time to response 
	_delay_us(80);
	//Checks if the DHT22 responses as expected as per data sheet (Should pull the voltage back low)
	if (!(PIND & (1 << 7)))
	{
		return true;
		//lights up LED at PB0 to indicate successful initialization
		PORTB |= (1<<0);
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
			 while (PIND & (1 << 7));
			 // Wait for HIGH
			 while (!(PIND & (1 << 7)));
			 /* Sample after 50us if the value is already high (0 bits stays high for 26us after initial 40us
			    verse 1 bits which stays high for 70us, the bit is a 1, else it is a zero. 
			    Since the buffer is already cleared to zero at start, no action is needed */
			 
			 _delay_us(50);	
			 if (PIND & (1 << 7))
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
			 while (PIND & (1 << 7));
	}
	//Verify the (data % 256) is == to the checksum to ensure proper transmission
	if ( ((DHT22Data[0]+DHT22Data[1]+DHT22Data[2]+DHT22Data[3]) % 256) == DHT22Data[4])
	{
		return true;
	}
	else return false;
	}
