#ifndef _DS1820_H_
#define _DS1820_H_

// mapping of sensors to the ports of microcontroller
sbit SENSOR_A	= P3^2;	// INT0
sbit SENSOR_B	= P3^3;	// INT1

// ROM function commands
#define DS1820_READ_ROM		0x33
#define DS1820_MATCH_ROM	0x55
#define DS1820_SKIP_ROM		0xCC
#define DS1820_SEARCH_ROM	0xF0
#define DS1820_ALARM_SEARCH	0xEC

// Memory function commands
#define DS1820_WRITE_SCRATCHPAD		0x4E
#define DS1820_READ_SCRATCHPAD		0xBE
#define DS1820_COPY_SCRATCHPAD		0x48
#define DS1820_CONVERT_TEMPERATURE	0x44
#define DS1820_RECALL_EEPROM		0xB8
#define DS1820_READ_POWER_SUPPLY	0xB4

// DS1820 responses
#define DS1820_DONE_CONVERTING		0xFF

#endif
