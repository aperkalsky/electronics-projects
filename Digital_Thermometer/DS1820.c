// This file contains drivers for Dallas DS1820 temperature sensor
// ---------------------------------------------------------------

#include <reg52.h>                /* special function register declarations   */
                                  /* for the intended 8051 derivative         */
#include "DS1820.h"
#include "WinTypes.h"
#include <stdio.h>	// to be removed later

extern void Sleep(WORD msec);	// this function reside in LCD.h file

/*
	Function name: DS1820_Init()

	Description:
		This function resets DS1820 connected to the specified 8051 port and checks
		for presence pulse.

	Input:
		bIndex - index of connected sensor. The valid values are 0 and 1

	Output:
		None

	Return Value:
		TRUE - sensor is present
		FALSE - no sensor
*/
bit DS1820_Init(BYTE bIndex)
{
	BYTE i;			// loop iterator
	bit bResult;	// result of port read

	// reset sensor
	if(bIndex == 0)
		SENSOR_A = 0;
	else
		SENSOR_B = 0;

	// keep it low at least 480 us
	for(i = 0; i < 100; i++);	// 5 us per iteration @ 12 MHz clock

	// release port to high state
	if(bIndex == 0)
		SENSOR_A = 1;
	else
		SENSOR_B = 1;

	// wait at least 60 us before presense pulse check
	for(i = 0; i < 14; i++);

	// read port status
	if(bIndex == 0)
		bResult = SENSOR_A;
	else
		bResult = SENSOR_B;

	// wait at least 300 us to give the sensor time to release the line
	for(i = 0; i < 60; i++);

	return (!bResult);
}

/*
	Function name: DS1820_WriteBit()

	Description:
		This function writes a bit to the DS1820 via specified port index

	Input:
		bIndex - index of connected sensor. The valid values are 0 and 1
		bData - bit to write

	Output:
		None

	Return Value:
		None
*/
void DS1820_WriteBit(BYTE bIndex, bit bData)
{
	BYTE i;			// loop iterator

	// initiate write slot
	if(bIndex == 0)
		SENSOR_A = 0;
	else
		SENSOR_B = 0;

	// wait at least 1 us to give the sensor chance to see it
	for(i = 0; i < 1; i++);	// 5 us per iteration @ 12 MHz clock

	// output data
	if(bIndex == 0)
		SENSOR_A = bData;
	else
		SENSOR_B = bData;

	// the sensor will start sampling after first 15 us, but the write slot length must be at least 60 us
	// so make a delay before return
	for(i = 0; i < 12; i++);	// 5 us per iteration @ 12 MHz clock

	// release I/O line
	if(bIndex == 0)
		SENSOR_A = 1;
	else
		SENSOR_B = 1;
}

/*
	Function name: DS1820_ReadBit()

	Description:
		This function reads a bit from DS1820 via specified port index

	Input:
		bIndex - index of connected sensor. The valid values are 0 and 1

	Output:
		None

	Return Value:
		Bit read
*/
bit DS1820_ReadBit(BYTE bIndex)
{
	BYTE i;			// loop iterator
	bit bResult;	// result of port read

	// initiate read slot
	if(bIndex == 0)
		SENSOR_A = 0;
	else
		SENSOR_B = 0;

	// wait at least 1 us to give the sensor chance to see it
	for(i = 0; i < 1; i++);	// 5 us per iteration @ 12 MHz clock

	// release I/O line
	if(bIndex == 0)
		SENSOR_A = 1;
	else
		SENSOR_B = 1;

	// the data will settle in 15 us, so wait more time before reading data
	for(i = 0; i < 1; i++);	// 5 us per iteration @ 12 MHz clock

	// read port value
	if(bIndex == 0)
		bResult = SENSOR_A;
	else
		bResult = SENSOR_B;

	// make a delay to continue the read slot. Read slot duration is 60 us
	for(i = 0; i < 12; i++);	// 5 us per iteration @ 12 MHz clock

	// return result
	return bResult;
}

/*
	Function name: DS1820_WriteByte()

	Description:
		This function writes a byte to the DS1820

	Input:
		bIndex - index of connected sensor. The valid values are 0 and 1
		bData - data byte to write

	Output:
		None

	Return Value:
		None
*/
void DS1820_WriteByte(BYTE bIndex, BYTE bData)
{
	BYTE i;		// loop iterator
	BYTE bTemp;	// temporary storage

	bTemp = bData;

	// transmit the byte bit by bit
	for(i = 0; i < 8; i++)
	{
		DS1820_WriteBit(bIndex, (bit)(bTemp & 0x01));	// send bit
		bTemp >>= 1;									// shift data right	
	}
}

/*
	Function name: DS1820_ReadByte()

	Description:
		This function reads a byte from DS1820

	Input:
		bIndex - index of connected sensor. The valid values are 0 and 1

	Output:
		None

	Return Value:
		Byte read
*/
BYTE DS1820_ReadByte(BYTE bIndex)
{
	BYTE i;			// loop iterator
	BYTE bTemp = 0;	// temporary storage

	// read eight bits in the loop
	for(i = 0; i < 8; i++)
	{
		bTemp >>= 1;	// shift data right
		if(DS1820_ReadBit(bIndex))
			bTemp |= 0x80;	// set high bit to 1
		else
			bTemp &= 0x7F;	// set high bit to 0;
	}

	// return collected value
	return bTemp;
}

/*
	Function name: DS1820_ReadTemp()

	Description:
		This function performs a temperature measurement. It requests the selected sensor
		to start converting, waits for completion and reads the result.

	Input:
		bIndex - index of connected sensor. The valid values are 0 and 1
		bTemperature - pointer to the variable which will accept temperature measurement
		bNegative - pointer to the variable which accepts sign of measured temperature

	Output:
		None

	Return Value:
		TRUE - valid temperature was read
			bTemperature contain the module of temperature read in terms of 0.5 ^C
			bNegative specify the temperature sign. 0x00 means '+', 0xFF means '-'
		FALSE - timeout error occured during temperature conversion

    Notes:
		It takes about 500 ms to execute this function

		The scenario looks as following:
		- we request from sensor to start temperature conversion
		- it takes up to 500 ms so we patiently wait
		- then we will read results.
*/
BOOL DS1820_ReadTemp(BYTE bIndex, WORD* wTemperature, BYTE* bNegative)
{
	BYTE bTmp;	// this variable will collect temperature bytes read from the sensor

	// reset the sensor
	DS1820_Init(bIndex);

	// send "skip ROM" command
	DS1820_WriteByte(bIndex, DS1820_SKIP_ROM);

	// send "convert temperature" command
	DS1820_WriteByte(bIndex, DS1820_CONVERT_TEMPERATURE);

	// the temperature conversion can take up to 500 ms, so we'll just wait 500 ms before
	// checking results
	Sleep(500);

	// at this point the temperature measurement must be completed. Check that the sensor
	// really finished with this. If it's still busy, notify of timeout
	if(DS1820_ReadByte(bIndex) != DS1820_DONE_CONVERTING)
	{
		printf("not ready yet\n");
		Sleep(100);

		if(DS1820_ReadByte(bIndex) != DS1820_DONE_CONVERTING)
		{
			printf("not ready yet\n");
			Sleep(100);

			if(DS1820_ReadByte(bIndex) != DS1820_DONE_CONVERTING)
			{
				printf("not ready yet\n");
				Sleep(100);
			}
		}
	}

	if(DS1820_ReadByte(bIndex) == DS1820_DONE_CONVERTING)
	{
		// **** on success ****

		// prepare to read temperature
		// ---------------------------

		// reset the sensor
		DS1820_Init(bIndex);
		
		// send "skip ROM" command
		DS1820_WriteByte(bIndex, DS1820_SKIP_ROM);
		
		// send "read scratchpad registers" command
		DS1820_WriteByte(bIndex, DS1820_READ_SCRATCHPAD);

		// read and convert temperature
		// ----------------------------

		// The first byte read from sensor is LSB. It will contain the reading of
		// temperature
		bTmp = DS1820_ReadByte(bIndex);

		// The second byte read is MSB and it contain temperature sign. Store
		// it directly in the variable specified by pointer
		*bNegative = DS1820_ReadByte(bIndex);

		// In case of negative temperature take its module
		if(*bNegative == 0xFF)
			bTmp = (0xFF - bTmp) + 1;

		// Format temperature to the decimal (t*10) representation
		// Value of 5 means 0.5^C, 235 means 23.5^C, etc
		*wTemperature = (bTmp >> 1)*10 + ((bTmp & 0x01)*5);

		// reset the sensor
		DS1820_Init(bIndex);

		return TRUE;
	}
	else
	{
		// **** error case ****

		// reset temperature value
		*wTemperature = 0;

		return FALSE;
	}
}

/*

	Function name: DS1820_ReadTemp()

	Description:
		This function performs a temperature measurement. It requests the selected sensor
		to start converting, waits for completion and reads the result.

	Input:
		bIndex - index of connected sensor. The valid values are 0 and 1
		wTemp - pointer to the variable which will accept temperature measurement

	Output:
		None

	Return Value:
		TRUE - valid temperature was read
		FALSE - timeout error occured during temperature conversion

    Notes:
		It takes about 500 ms to execute this function

BOOL DS1820_ReadTemp(BYTE bIndex, WORD* wTemp)
{
	WORD wTmpAcc;	// this variable will collect temperature bytes read from the sensor

	// reset the sensor
	DS1820_Init(bIndex);

	// send "skip ROM" command
	DS1820_WriteByte(bIndex, DS1820_SKIP_ROM);

	// send "convert temperature" command
	DS1820_WriteByte(bIndex, DS1820_CONVERT_TEMPERATURE);

	// the temperature conversion can take up to 500 ms, so we'll just wait 500 ms before
	// checking results
	Sleep(500);

	// at this point the temperature measurement must be completed. Check that the sensor
	// really finished with this. If it's still busy, notify of timeout
	if(DS1820_ReadByte(bIndex) != DS1820_DONE_CONVERTING)
	{
		printf("not ready yet\n");
		Sleep(100);

		if(DS1820_ReadByte(bIndex) != DS1820_DONE_CONVERTING)
		{
			printf("not ready yet\n");
			Sleep(100);

			if(DS1820_ReadByte(bIndex) != DS1820_DONE_CONVERTING)
			{
				printf("not ready yet\n");
				Sleep(100);
			}
		}
	}

	if(DS1820_ReadByte(bIndex) == DS1820_DONE_CONVERTING)
	{
		// **** on success ****

		// prepare to read temperature
		// ---------------------------

		// reset the sensor
		DS1820_Init(bIndex);
		
		// send "skip ROM" command
		DS1820_WriteByte(bIndex, DS1820_SKIP_ROM);
		
		// send "read scratchpad registers" command
		DS1820_WriteByte(bIndex, DS1820_READ_SCRATCHPAD);

		// read and convert temperature
		// ----------------------------
		wTmpAcc = DS1820_ReadByte(bIndex);			// read temperature LSB

		if(wTmpAcc == 0)
			printf("first byte = 0\n");

		wTmpAcc += DS1820_ReadByte(bIndex) << 8;	// read temperature MSB

		if(wTmpAcc == 0)
			printf("second byte = 0\n");

		// convert received value and set up pointer
		*wTemp = (wTmpAcc >> 1)*10 + ((wTmpAcc & 0x0001)*5);

		// reset the sensor
		DS1820_Init(bIndex);

		return TRUE;
	}
	else
	{
		// **** error case ****

		// reset temperature value
		*wTemp = 0;

		return FALSE;
	}
}

*/