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
//#include "LCDexp.h"	// LCD function prototypes
//#include "DS1820ex.h"	// DS1820 function prototypes

// RS232 interface initialization. Timer 1 is sacrificed for this purpose.
void Init_RS232()
{
	SCON  = 0x50;                   /* SCON: mode 1, 8-bit UART, enable rcvr    */
	TMOD |= 0x20;                   /* TMOD: timer 1, mode 2, 8-bit reload      */
	TH1   = 0xf3;                   /* TH1:  reload value for 2400 baud         */
	TR1   = 1;                      /* TR1:  timer 1 run                        */
	TI    = 1;                      /* TI:   set TI to send first char of UART  */
}

// ------------------------------------------------------------------------------
//			LCD interface support
// ------------------------------------------------------------------------------

// Data structures
code char szTempEng[] = "TEMPERATURE 25.5";
code char szHumidEng[] = " HUMIDITY   30%";

code char szTempRus[] = {'T', 'E', 'M' ,0x00, 'E', 'P', 'A', 'T', 0x01, 'P', 'A', ' ', '2','5', '.', '5'};
code char szHumidRus[] = {' ', 'B', 0x02, 'A', 0x03, 'H', 'O', 'C', 'T', 0x04, ' ', ' ', '3', '0', '%'};

// Final data structures
// ---------------------
//idata char szLcdBuf0[16];	// temporary buffer for formatting text before output to LCD, upper line
//idata char szLcdBuf1[16];	// temporary buffer for formatting text before output to LCD, lower line
idata char szLcdBuf[2][16];	// temporary buffer for formatting text before output to LCD, two lines

// Patterns of Cyrullic letters for CGRAM loading
code BYTE RusPattern[6][8] = 
	{	0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00,\
		0x11, 0x11, 0x11, 0x0A, 0x04, 0x08, 0x10, 0x00,\
		0x07, 0x09, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00,\
		0x15, 0x15, 0x0E, 0x04, 0x0E, 0x15, 0x15, 0x00,\
		0x10, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E, 0x00,\
		0x0F, 0x11, 0x11, 0x0F, 0x05, 0x09, 0x11, 0x00};

// Error messages
//code char szNoSensor0[] = "Missing sensor 0";
//code char szNoSensor1[] = "Missing sensor 1";
code char* szNoSensor[] = {"Missing sensor 0", "Missing sensor 1"};

// ID headers
//code char szIdHdr0[4] = "ID0:";
//code char szIdHdr1[4] = "ID1:";
code char* szIdHdr[] = {"ID0:", "ID1:"};

// Measurements in English
code char szTempInEng[] = {' ','I','N','D','O','O','R',' ',' ',' ',' ','2','2','.','0',0xDF};
code char szTempOutEng[] = {' ','O','U','T','D','O','O','R',' ',' ',' ','3','1','.','5',0xDF};

// Measurements in Russian
code char szTempInRus[] = {'B','H',0x01,'T','P','E','H','H',0x05,0x05,' ','2','2','.','0',0xDF};
code char szTempOutRus[] = {' ','H','A','P',0x01,0x03,'H','A',0x05,' ',' ','3','1','.','5',0xDF};

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
	for(i = 0; i < 6; i++)
		for(j = 0; j < 8; j++)
		{
			Set_CGRAM_Addr(bAddr++);
			Set_RAM_Data(RusPattern[i][j]);
		}
}

void PutDemoInEnglish()
{
	int j;

	ClearDisplay();
	Set_DDRAM_Addr(0);

	bAddr = 0;	// reset address

	for(j = 0; j < sizeof(szTempEng) - 1; j++)
		Set_RAM_Data((BYTE)szTempEng[bAddr++]);
	
	Set_DDRAM_Addr(0x40);
	
	bAddr = 0;

	for(j = 0; j < sizeof(szHumidEng) - 1; j++)
		Set_RAM_Data((BYTE)szHumidEng[bAddr++]);
}

void PutDemoInRussian()
{
	int j;

	ClearDisplay();
	Set_DDRAM_Addr(START_OF_FIRST_LINE);

	bAddr = 0;

	for(j = 0; j < sizeof(szTempRus); j++)
		Set_RAM_Data((BYTE)szTempRus[bAddr++]);
	
	Set_DDRAM_Addr(START_OF_SECOND_LINE);
	
	bAddr = 0;

	for(j = 0; j < sizeof(szHumidRus); j++)
		Set_RAM_Data((BYTE)szHumidRus[bAddr++]);
}

/*
	Function name: UpdateTempInBuffer()

	Description:
		This function updates the temperature reading in the LCD buffer before
		it is displayed

	Input:
		bIndex - index of line in LCD buffer to update
		wTemperature - temperature value (module)
		bNegative - temperature sign. Zer of positive

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
			012345 - index withitn buffer
*/
void UpdateTempInBuffer(BYTE bIndex, WORD wTemprValue, BYTE bNegative)
{
	// print module of temperature in temporary buffer
//	sprintf(Buff, "%6.5d", (int)wTemprValue);
	sprintf(Buff, "%6.5d", (int)wTemprValue);

	// Apply one more zero to values less than 1 degree (0 and 5)
	if((wTemprValue == 0) || (wTemprValue == 5))
		Buff[3] = '0';

	// Apply temperature sign. Zero will be displayed with no sign
	if(wTemprValue != 0)
	{
		// for two-digit values
		if(Buff[2] == ' ')
		{
			Buff[2] = bNegative?'-':'+';
		}
		else
		{
			// for three-digit values
			if(Buff[1] == ' ')
			{
				Buff[1] = bNegative?'-':'+';
			}
			else
				Buff[0] = bNegative?'-':'+';	// this is four-digit value
		}
	}

	// check results
	printf("%s\n", Buff);

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

	ClearDisplay();

	// display upper line
	Set_DDRAM_Addr(0);

	bAddr = 0;

	for(j = 0; j < LCD_LINE_LENGTH; j++)
		Set_RAM_Data((BYTE)szLcdBuf[0][bAddr++]);
	
	// display lower line
	Set_DDRAM_Addr(0x40);
	
	bAddr = 0;

	for(j = 0; j < LCD_LINE_LENGTH; j++)
		Set_RAM_Data((BYTE)szLcdBuf[1][bAddr++]);
}

void DispByte(BYTE i)
{
	switch(i)
	{
	case 0x00:
		printf("0\n");
		break;

	case 0x01:
		printf("1\n");
		break;

	case 0x02:
		printf("2\n");
		break;

	case 0x03:
		printf("3\n");
		break;

	case 0x04:
		printf("4\n");
		break;

	case 0x05:
		printf("5\n");
		break;

	case 0x06:
		printf("6\n");
		break;

	case 0x07:
		printf("7\n");
		break;

	case 0x08:
		printf("8\n");
		break;

	case 0x09:
		printf("9\n");
		break;

	case 0x0A:
		printf("A\n");
		break;

	case 0x0B:
		printf("B\n");
		break;

	case 0x0C:
		printf("C\n");
		break;

	case 0x0D:
		printf("D\n");
		break;

	case 0x0E:
		printf("E\n");
		break;

	case 0x0F:
		printf("F\n");
		break;

	default:
		printf("%d > 0x0F\n", i);
	}
}

/****************/
/* main program */
/****************/
void main (void)
{
	BYTE i, j;
//	char j;	// loop iterators	// it was *int* before.It must be signed, since we use downcount

	// initialize serial port
	Init_RS232();

	// init and configure LCD
	InitAndConfigure_LCD();

//	printf("LCD init done\n");

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
//			memcpy(szLcdBuf[i], szIdHdr[i], 4);

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
			
			printf("sensor %02x failed init\n", i);

			// set error flag
			bNoSensor = 1;
			
			// output warning to RS-232
			printf("%s\n", szNoSensor[i]);
			
			// copy warning to LCD buffer
			for(j = 0; j < LCD_LINE_LENGTH; j++)
				szLcdBuf[i][j] = szNoSensor[i][j];
//			memcpy(szLcdBuf[i], szNoSensor[i], LCD_LINE_LENGTH);
		}
	}

//	printf("before displaying buffer\n");

//	for(j = 0; j < LCD_LINE_LENGTH; j++)
//		printf("%c %c\n", szLcdBuf[0][j], szLcdBuf[1][j]);

//	printf("%s\n", szLcdBuf[0]);
//	printf("%s\n", szLcdBuf[1]);

	// Display formatted LCD buffer
	DisplayLcdBuffer();

	// give user a chance to read it
	Sleep(20000);	// 20 sec delay

//	printf("after displaying buffer\n");

	// As was mentioned before, if it appears that one of the temperature sensors is missing,
	// we willl not proceed further and stop here kicking around in endless loop
	if(bNoSensor)
	{
		while(1){;};	// loop here forever
	}

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

//		memcpy(szLcdBuf[0], szTempInRus, LCD_LINE_LENGTH);
//		memcpy(szLcdBuf[1], szTempOutRus, LCD_LINE_LENGTH);
	}
	else
	{
		for(j = 0; j < LCD_LINE_LENGTH; j++)
		{
			szLcdBuf[0][j] = szTempInEng[j];
			szLcdBuf[1][j] = szTempOutEng[j];
		}

//		memcpy(szLcdBuf[0], szTempInEng, LCD_LINE_LENGTH);
//		memcpy(szLcdBuf[1], szTempOutEng, LCD_LINE_LENGTH);
	}

/*
	// debug stuff only
	// ---------------
	DisplayLcdBuffer();

	Sleep(5000);

	LED = 0;

	if(DS1820_ReadTemp(0, &wTemperature, &bTempSign))
		printf("Temp 0 = %d\n", (int)wTemperature);
	else
		printf("Failed\n");

	if(DS1820_ReadTemp(1, &wTemperature, &bTempSign))
		printf("Temp 1 = %d\n", (int)wTemperature);
	else
		printf("Failed\n");

	LED = 1;

	PutDemoInEnglish();

	Sleep(5000);

	PutDemoInRussian();

	// attempt to display formatted temperature
	bVal = 5;
	sprintf(Buff, "%5.4d", (int)bVal);
	printf("%s", Buff);

	bVal = 25;
	sprintf(Buff, "%5.4d", (int)bVal);
	printf("%s", Buff);

	bVal = 125;
	sprintf(Buff, "%5.4d", (int)bVal);
	printf("%s", Buff);

	bVal = 0;
	sprintf(Buff, "%5.4d", (int)bVal);
	printf("%s", Buff);

	// END OF DEBUG STUFF
*/

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
		LED = 0;

		// query both sensors in turn and update display
		for(i = 0; i < 2; i++)
		{
			// read temperature from sensor
			DS1820_ReadTemp(i, &wTemperature, &bTempSign);

			// update relevant buffer contents
			UpdateTempInBuffer(i, wTemperature, bTempSign);

			// update display buffer
			// TODO
			printf("Temp %d = %d, sign = %02X\n", (int)i, (int)wTemperature, (int)bTempSign);
		}

		// turn LED off
		LED = 1;

		// refresh LCD output
		DisplayLcdBuffer();

		// wait here in time wasting loop. Using Sleep() we can reach one minute value
		// in one call
		Sleep(60000);	// 60 sec delay
	}
}
