// MAIN.C
//
// Main program for digital thermometer
// ------------------------------------
//#pragma DEBUG OBJECTEXTEND CODE   /* pragma lines can contain state C51       */
#pragma OBJECTEXTEND CODE   /* pragma lines can contain state C51       */
                                  /* command line directives                  */
 
#include <reg52.h>                /* special function register declarations   */
                                  /* for the intended 8051 derivative         */

#include <stdio.h>                /* prototype declarations for I/O functions */
#include <string.h>		// memcpy declarations
#include "WinTypes.h"	// windows-style variable type declarations
#include "LCD.h"		// LCD control codes
#include "DS1820.h"		// DS1820 control codes
#include "Exports.h"	// exported functions prototypes

// ------------------------------------------------------------------------------
//			LCD interface support
// ------------------------------------------------------------------------------

// Final data structures
// ---------------------
idata char szLcdBuf[NUM_LCD_LINES][LCD_LINE_LENGTH];	// temporary buffer for formatting text before output to LCD, two lines

// Patterns of Cyrullic letters for CGRAM loading (Pe, Y, L, Ze, m.znak, Ya, Che, I)
#define NUM_CUSTOM_CHARACTERS	8
#define NUM_LINES_IN_PATTERN	8

code BYTE RusPattern[NUM_CUSTOM_CHARACTERS][NUM_LINES_IN_PATTERN] = 
	{	0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00,\
		0x11, 0x11, 0x11, 0x0A, 0x04, 0x08, 0x10, 0x00,\
		0x07, 0x09, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00,\
		0x15, 0x15, 0x0E, 0x04, 0x0E, 0x15, 0x15, 0x00,\
		0x10, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E, 0x00,\
		0x0F, 0x11, 0x11, 0x0F, 0x05, 0x09, 0x11, 0x00,\
		0x11, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x01, 0x00,\
		0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x00};

// Intro message
code char* szIntro[] = {"  Dual Channel  ", "Thermometer V1.1"};

// Error messages
code char* szNoSensorEng[] = {"Missing sensor 0", "Missing sensor 1"};
//code char* szNoSensorRus[] = {"HET CEHCOPA  #0", "HET CEHCOPA  #1"};

code char szNoSensorRus[NUM_LCD_LINES][LCD_LINE_LENGTH] =	// error messages in Russian
	{'H', 'E', ' ', 'B', 0x07, 0x03, 0x01, ' ', 'C', 'E', 'H', 'C', 'O', 'P', ' ', '0',\
	 'H', 'E', ' ', 'B', 0x07, 0x03, 0x01, ' ', 'C', 'E', 'H', 'C', 'O', 'P', ' ', '1'};

// ID headers
code char* szIdHdr[] = {"ID0:", "ID1:"};

// Measurements in English
code char szTempInEng[] =  {' ','I','N','D','O','O','R',' ',' ',' ',' ','2','2','.','0',0xDF};
code char szTempOutEng[] = {' ','O','U','T','D','O','O','R',' ',' ',' ','3','1','.','5',0xDF};

// Measurements in Russian
code char szTempInRus[] =  {'K','O' ,'M' ,'H' ,'A' ,'T','H','A' ,0x05,' ',' ','2','2','.','0',0xDF};
code char szTempOutRus[] = {' ',0x01,0x02,0x07,0x06,'H','A',0x05,' ' ,' ',' ','3','1','.','5',0xDF};

// Hex numbers trannslation table
code char szHexTable[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

data BYTE bAddr;	// current address of data transactions to LCD
data bit bNoSensor = 0;	// Error flag. Is set if one of the sensors is missing
data WORD wTemperature;	// temperature reading
data BYTE bTempSign;	// temperature sign
data BYTE bTmp;	// temporary storage for data read
data BYTE bHi, bLo;

// For testing only - char buffer
data char Buff[6];
data BYTE bVal;

// Mapping of language selecting switch
sbit LANG_SWITCH	= P3^6;	// #WR

#define OUTPUT_IN_RUSSIAN 0	// switch statuses

// mapping of auxiliary LED
sbit LED		= P3^7;	// #RD

// LED is tied up to Vcc
#define LED_ON	0
#define LED_OFF	1

// mapping of DIP switch (selector of delay between measurements)
sbit DELAY_SELECT_LO = P3^0;	// RXD
sbit DELAY_SELECT_HI = P3^1;	// TXD

// switch statuses
#define DELAY_A		0
#define DELAY_B		1
#define DELAY_C		2
#define DELAY_D		3

void InitAndConfigure_LCD()
{
	int i, j;

	// initialize LCD
	Init_LCD();

	// reset address iterator
	bAddr = 0;

	// turn cursor off
	DisplayOnOff(DOO_LCD_ON | DOO_CURSOR_OFF | DOO_BLINK_OFF);

	// load CGRAM with Cyrillic patterns
	for(i = 0; i < NUM_CUSTOM_CHARACTERS; i++)
		for(j = 0; j < NUM_LINES_IN_PATTERN; j++)
		{
			Set_CGRAM_Addr(bAddr++);
			Set_RAM_Data(RusPattern[i][j]);
		}
}

void PutIntroMessage()
{
	int j;

	ClearDisplay();
	Set_DDRAM_Addr(START_OF_FIRST_LINE);

	bAddr = 0;	// reset address

	for(j = 0; j < LCD_LINE_LENGTH; j++)
		Set_RAM_Data((BYTE)szIntro[0][bAddr++]);
	
	Set_DDRAM_Addr(START_OF_SECOND_LINE);
	
	bAddr = 0;

	for(j = 0; j < LCD_LINE_LENGTH; j++)
		Set_RAM_Data((BYTE)szIntro[1][bAddr++]);
}

/*
	Function name: UpdateTempInBuffer()

	Description:
		This function updates the temperature reading in the LCD buffer before
		it is displayed

	Input:
		bIndex - index of line in LCD buffer to update
		wTemperature - temperature value (module)
		bNegative - temperature sign. Zero if positive

	Output:
		None

	Return Value:
		None

	Note:
		The temporary buffer size is 6 bytes and the values will be printed as following:
			------
			 1250
			  235
			   20
			    0
			------
           012345 - index within buffer

			 1111 - index within LCD buffer
			90124

		LCD buffer layout:
		   -----------------------------------------------------------------
		   |' ','O','U','T','D','O','O','R',' ',' ','+','3','1','.','5','°'|	LCD
		   -----------------------------------------------------------------
             I   I   I   I   I   I   I   I   I   I   I   I   I   I   I   I
		   -----------------------------------------------------------------
		   | 0   1   2   3   4   5   6   7   8   9   1   1   1   1   1   1 |
		   |                                         0   1   2   3   4   5 |	LCD buffer
		   -----------------------------------------------------------------
                                                 I   I   I   I       I      
		   -----------------------------------------------------------------
		   |                                 0   1   2   3   4       5     |	Temporary buffer
		   -----------------------------------------------------------------
*/
void UpdateTempInBuffer(BYTE bIndex, WORD wTemprValue, BYTE bNegative)
{
	// print module of temperature in temporary buffer
	sprintf(Buff, "%6.5d", (int)wTemprValue);

	// Apply one more zero to values less than 1 degree (0 and 5)
	if((wTemprValue == 0) || (wTemprValue == 5))
		Buff[4] = '0';

	// Apply temperature sign. Zero will be displayed with no sign
	if(wTemprValue)
	{
		// for values less than 10
		if(Buff[3] == ' ')
			Buff[3] = bNegative?'-':'+';
		else
		{
			// for values less than 100
			if(Buff[2] == ' ')
				Buff[2] = bNegative?'-':'+';
			else
			{
				// for values less than 1000
				if(Buff[1] == ' ')
					Buff[1] = bNegative?'-':'+';
			}
		}
	}

	// Now copy formatted value to the LCD buffer
	szLcdBuf[bIndex][14] = Buff[5];
	szLcdBuf[bIndex][12] = Buff[4];
	szLcdBuf[bIndex][11] = Buff[3];
	szLcdBuf[bIndex][10] = Buff[2];
	szLcdBuf[bIndex][9]  = Buff[1];

}

/*
	Function name: DisplayLcdBuffer()

	Description:
		This function clears LCD and outputs a new text to it. The new text is taken from memory buffer dedicated for
		formatting LCD data prior to output.

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void DisplayLcdBuffer()
{
	int j;	// loop iterator

//	ClearDisplay();

	// display upper line
	Set_DDRAM_Addr(START_OF_FIRST_LINE);

	bAddr = 0;

	for(j = 0; j < LCD_LINE_LENGTH; j++)
		Set_RAM_Data((BYTE)szLcdBuf[0][bAddr++]);
	
	// display lower line
	Set_DDRAM_Addr(START_OF_SECOND_LINE);
	
	bAddr = 0;

	for(j = 0; j < LCD_LINE_LENGTH; j++)
		Set_RAM_Data((BYTE)szLcdBuf[1][bAddr++]);
}


/****************/
/* main program */
/****************/
void main (void)
{
	BYTE i, j;			// loop iterators
	BYTE bDelayValue;	// reading of time selecting DIP switch

	// initialize serial port
//	Init_RS232();

	// init and configure LCD
	InitAndConfigure_LCD();

	// put intro message
	PutIntroMessage();

	// give user a chance to read it
	Sleep(5000);	// 5 sec delay

	// ========================================================================
	// Check sensors presence. Format LCD buffer according to the system state.
	// If sensor is missing, print an error message. If sensor is present, display
	// it's serial number (ID). If one of sensors is missing, we'll enter an endless loop
	// after displaying the system state. Otherwise we'll proceed with real temperature
	// measurements.

	// In order to check sensor presence we need to output a reset pulse and check for
	// presence pulse from the sensor (see DS1820_Init()).
	//
	// In order to read sensor ID we need to send READ_ROM command to it. The sensor will
	// respond with 8 bytes with the following layout:
	//
	//   Last byte  >>>>>>>>>>>>>>   First byte
	//          |                    |
	//          V                    V
	//          xx xx xx xx xx xx xx xx
	//          |  |               | |
	//          |  |               | ---- DS1820 family code (0x10)
	//          |  |               ------ LS Byte of 48-bit sensor ID
	//          |  ---------------------- MS Byte of 48-bit sensor ID
	//          ------------------------- 8-but CRC code
	//
	// The first and last bytes are unnecessary to display, so we will display the
	// sensor ID only.
	// ========================================================================

	// turn LED on
	LED = LED_ON;
			
	// query both sensors in turn
	for(i = 0; i < 2; i++)
	{
		if(DS1820_Init(i))
		{	// ** on success **

			// send ID request
			DS1820_WriteByte(i, DS1820_READ_ROM);
			
			// Copy ID header string to the LCD buffer
			for(j = 0; j < 4; j++)
				szLcdBuf[i][j] = szIdHdr[i][j];

			// The first byte is a family code, make a dummy read
			bTmp = DS1820_ReadByte(i);
			
			// Next six bytes will be the sensor ID in reverse order. Read them one by one
			// and format the LCD buffer
			for(j = 0; j < 6; j++)
			{
				// read byte
				bTmp = DS1820_ReadByte(i);

				// print high nibble of it into LCD buffer
				szLcdBuf[i][(BYTE)(14-(j<<1))] = szHexTable[HINIBBLE(bTmp)];
				
				// print low nibble of it into LCD buffer
				szLcdBuf[i][(BYTE)(15-(j<<1))] = szHexTable[LONIBBLE(bTmp)];
			}
			
			// The last byte read is CRC, make a dummy read
			bTmp = DS1820_ReadByte(i);
		}
		else
		{	// ** error case **

			// set error flag
			bNoSensor = 1;
			
			// copy warning to LCD buffer accprding to selected language
			if(LANG_SWITCH == OUTPUT_IN_RUSSIAN)
			{
				for(j = 0; j < LCD_LINE_LENGTH; j++)
					szLcdBuf[i][j] = szNoSensorRus[i][j];
			}
			else
			{
				for(j = 0; j < LCD_LINE_LENGTH; j++)
					szLcdBuf[i][j] = szNoSensorEng[i][j];
			}
		}
	}

	// turn LED off
	LED = LED_OFF;
	
	// Display formatted LCD buffer
	DisplayLcdBuffer();

	// As was mentioned before, if it appears that one of the temperature sensors is missing,
	// we willl not proceed further and stop here kicking around in endless loop
	if(bNoSensor)
	{
		while(1)
		{
			// blink LED
			LED = LED_ON;
			Sleep(500);
			LED = LED_OFF;
			Sleep(500);
		};	// loop here forever
	}

	// In the case that the sensors are present give user a chance to read their IDs
	Sleep(10000);	// 10 sec delay

	// ========================================================================
	// If we are here, both sensors are persent and we can enter a main loop where we will
	// measure and display the temperature. The LCD messages can be displayed in two languages,
	// in Russian and in English. The output language can be selected by external switch which
	// will be tested once before we enter a main loop.
	// ========================================================================

	// Format display buffer according to selected language
	if(LANG_SWITCH == OUTPUT_IN_RUSSIAN)
	{
		for(j = 0; j < LCD_LINE_LENGTH; j++)
		{
			szLcdBuf[0][j] = szTempInRus[j];
			szLcdBuf[1][j] = szTempOutRus[j];
		}
	}
	else
	{
		for(j = 0; j < LCD_LINE_LENGTH; j++)
		{
			szLcdBuf[0][j] = szTempInEng[j];
			szLcdBuf[1][j] = szTempOutEng[j];
		}
	}


	// ========================================================================
	// Here we enter the main loop. It's rather sumple, only three tasks are
	// performed here:
	//	1) read temperature of both sensors;
	//  2) update display with new measurements;
	//  3) make a delay before starting over.
	//
	// The auxiliary is lit during temperature conversion to indicate user that
	// something is happening.
	// ========================================================================
	while(1)
	{
		// turn LED on
		LED = LED_ON;

		// query both sensors in turn and update display
		for(i = 0; i < 2; i++)
		{
			// read temperature from sensor
			DS1820_ReadTemp(i, &wTemperature, &bTempSign);

			// update relevant buffer contents
			UpdateTempInBuffer(i, wTemperature, bTempSign);
		}

		// turn LED off
		LED = LED_OFF;

		// refresh LCD output
		DisplayLcdBuffer();

		// wait here in time wasting loop. Using Sleep() we can reach one minute value
		// in one call. The wait time is a configurable parameter, so we need to study
		// state of external DIP switch and make a delay according to it
		bDelayValue = DELAY_SELECT_HI;	// read high bit
		bDelayValue = DELAY_SELECT_LO | (bDelayValue << 1);	// read low bit

		switch(bDelayValue)
		{
		case DELAY_A:	// shortest delay
			Sleep(500);		// 0.5 sec
			break;

		case DELAY_B:
			Sleep(5000);	// 5 sec
			break;

		case DELAY_C:
			Sleep(20000);	// 20 sec
			break;

		case DELAY_D:
			Sleep(60000);	// 1 min
			break;
		}
	}
}
