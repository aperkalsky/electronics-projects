#ifndef _LCDEXP_H_
#define _LCDEXP_H_

// export definition of functions from LCD.C file

//extern void WriteCommand(BYTE bCommand);
//extern void WriteData(BYTE bData);
extern void Init_LCD();
extern void ClearDisplay();
extern void DisplayOnOff(BYTE bFlags);
extern void Set_CGRAM_Addr(BYTE bAddr);
extern void Set_DDRAM_Addr(BYTE bAddr);
extern void Set_RAM_Data(BYTE bData);
extern void Sleep(WORD msec);



#endif