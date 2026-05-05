#include "LCD.h"
#include "WinTypes.h"

/*
	Module description

  This module contains driver routines for M1632 (2x16 character) LCD display, connected
  to 8051 ports. The port pin usage is the following:

  P1.0 - P1.7 : data bus
  P3.4 (T0)   : LCD Enable (E)
  P3.5 (T1)   : LCD Select (RS)

  It is assumed that we only write to the LCD, so R/W' pin of the LCD is tied to GND.
  We give maximum time for each command execution, since there's no feedback from the LCD
  in this case.
*/

void Sleep(WORD msec)
{
	BYTE i;

	do
	{
		for(i = 0; i < 125; i++){;}	// time wasting loop. It must take approx 1ms to execute
	}
	while(msec-- > 0);
}

//	This function writes a command byte to the LCD
void WriteCommand (BYTE bCommand)
{
	LCD_RS = 0;		// notify that command will be sent
	LCD_E  = 1;
	LCD_DATA = bCommand;
	LCD_E  = 0;
}

//	This function writes a data byte to the LCD
void WriteData (BYTE bData)
{
	LCD_RS = 1;		// notify that data will be sent
	LCD_E  = 1;
	LCD_DATA = bData;
	LCD_E  = 0;
}

void ClearDisplay()
{
	WriteCommand(DISPLAY_CLEAR);

	// clear display can take up to 1.64 ms so wait enough time
	Sleep(3);
}

void EntryModeSet(BYTE bFlags)
{
	WriteCommand(ENTRY_MODE_SET | bFlags);

	Sleep(1);
}

void DisplayOnOff(BYTE bFlags)
{
	WriteCommand(DISPLAY_ON_OFF | bFlags);

	Sleep(1);

}

void FunctionSet(BYTE bFlags)
{
	WriteCommand(FUNCTION_SET | bFlags);

//	Sleep(2);

}

void Set_CGRAM_Addr(BYTE bAddr)
{
	WriteCommand(SET_CG_RAM_ADDR | (bAddr & 0x3F));	// CGRAM_Addr address is 6-bit

	Sleep(1);

}

void Set_DDRAM_Addr(BYTE bAddr)
{
	WriteCommand(SET_DD_RAM_ADDR | (bAddr & 0x7F));	// DDRAM_Addr address is 7-bit

	Sleep(1);

}

void Set_RAM_Data(BYTE bData)
{
	WriteData(bData);

	Sleep(1);
}

void Init_LCD()
{
	Sleep(20);

	FunctionSet(FS_8_BIT_INTERFACE);
	Sleep(5);

	FunctionSet(FS_8_BIT_INTERFACE);
	Sleep(2);
	
	FunctionSet(FS_8_BIT_INTERFACE);
	Sleep(2);

//	FunctionSet(FS_8_BIT_INTERFACE);
//	Sleep(2);

	DisplayOnOff(DOO_LCD_ON | DOO_CURSOR_ON | DOO_BLINK_OFF);

	ClearDisplay();

	EntryModeSet(EMS_INCREMENT);
}

// These functions are not used in thermometer application
// --------------------------------------------------------------
#ifdef EXTENDED_LCD_COMMANDS

void CusrorAtHome()
{
	WriteCommand(CURSOR_HOME);

	// cursor at home can take up to 1.64 ms so wait enough time
	Sleep(3);
}

void CursorDisplayShift(BYTE bFlags)
{
	WriteCommand(CURS_N_DISPLAY_SHIFT | bFlags);

	Sleep(1);

}

#endif
// --------------------------------------------------------------
