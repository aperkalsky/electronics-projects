#ifndef _EXPORTS_H_
#define _EXPORTS_H_

// from DS1820.c
extern bit DS1820_Init(BYTE bPortAddr);
extern void DS1820_WriteByte(BYTE bIndex, BYTE bData);
extern BYTE DS1820_ReadByte(BYTE bIndex);
extern BOOL DS1820_ReadTemp(BYTE bIndex, WORD* wTemperature, BYTE* bNegative);

// export definition of functions from LCD.C file
extern void Init_LCD();
extern void ClearDisplay();
extern void DisplayOnOff(BYTE bFlags);
extern void Set_CGRAM_Addr(BYTE bAddr);
extern void Set_DDRAM_Addr(BYTE bAddr);
extern void Set_RAM_Data(BYTE bData);
extern void Sleep(WORD msec);

#endif