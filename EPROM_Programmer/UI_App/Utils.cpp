// Utils.cpp: implementation of the Utils class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Programmer.h"
#include "Utils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/*
	ByteHexToBin - translates text representation of hex byte to its binary value
*/
BOOL ByteHexToBin(BYTE* hex_data, BYTE* bin_data)
{
    unsigned char   digit;

    /* First hex digit. */
    digit = *hex_data;
   digit = toupper(digit);
    if ((digit >= '0') && (digit <= '9'))
        *bin_data = (digit - 0x30) << 4;
    else
        if ((digit >= 'A') && (digit <= 'F'))
            *bin_data = (digit - 55) << 4;
        else
            return FALSE;

    /* Second hex digit. */
    digit = *(hex_data+1);
   digit = toupper(digit);
    if ((digit >= '0') && (digit <= '9'))
        *bin_data += (digit - 0x30);
    else
	{
        if ((digit >= 'A') && (digit <= 'F'))
            *bin_data += (digit - 55);
		else
			return FALSE;
	}
	return TRUE;
}

/*
  HexWordToNum - converts hex word represented as string to its binary value
*/
BOOL HexWordToNum(BYTE* String, WORD* Result)
{
	BYTE HighByte = 0;
	BYTE LowByte = 0;

	if(ByteHexToBin(String, &HighByte))
	{
		if(ByteHexToBin((BYTE*)(String + 2), &LowByte))
		{
			*Result = (HighByte << 8) + LowByte;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}
