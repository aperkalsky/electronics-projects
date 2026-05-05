#ifndef _LCD_H_
#define _LCD_H_

//#include <reg51.h>

// LCD interface definitions and function prototypes

// commands to LCD
#define DISPLAY_CLEAR			0x01
#define CURSOR_HOME				0x02
#define ENTRY_MODE_SET			0x04
#define DISPLAY_ON_OFF			0x08
#define CURS_N_DISPLAY_SHIFT	0x10
#define FUNCTION_SET			0x28
#define SET_CG_RAM_ADDR			0x40
#define SET_DD_RAM_ADDR			0x80

// LCD mode/action flags:	(they must be ORed with appropriate command)
// ----------------------

// for "entry mode set"
#define EMS_INCREMENT	0x02
#define EMS_DECREMENT	0x00

#define EMS_SHIFT		0x01

// for "display on/off"
#define DOO_LCD_ON		0x04
#define DOO_LCD_OFF		0x00

#define DOO_CURSOR_ON	0x02
#define DOO_CURSOR_OFF	0x00

#define DOO_BLINK_ON	0x01
#define DOO_BLINK_OFF	0x00

// for "cursor & display shift"
#define CDS_DISPLAY_SHIFT	0x08
#define CDS_CURSOR_MOVE		0x00

#define CDS_SHIFT_RIGHT		0x04
#define CDS_SHIFT_LEFT		0x00

// for "function set"
#define FS_4_BIT_INTERFACE	0x00
#define FS_8_BIT_INTERFACE	0x10

// DDRAM addresses
#define START_OF_FIRST_LINE		0x00
#define START_OF_SECOND_LINE	0x40

#define LCD_LINE_LENGTH		16	// length of LCD line of M1632 LCD

// Mapping of LCD pins to microcontroller ports
// --------------------------------------------
sbit LCD_RS		= 0xB5;	// T1
sbit LCD_E		= 0xB4;	// T0
sfr LCD_DATA	= 0x90;	// P1

//#define LCD_E		T0
//#define LCD_RS		T1
//#define LCD_DATA	P1

#endif