// HexBuff.h: interface for the HexBuff class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HEXBUFF_H__CFD36706_E139_11D3_8E5B_00105A92A724__INCLUDED_)
#define AFX_HEXBUFF_H__CFD36706_E139_11D3_8E5B_00105A92A724__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

const ULONG BANK_SIZE = 0x40000;		// 256K buffer

// return codes
const BYTE BUFF_NO_FILE	= 0;	// unable to open specified file
const BYTE BUFF_OK		= 1;	// command done

const BYTE RL_INV		= 2;	// invalid line in hex file
const BYTE RL_BAD_CS	= 3;	// bad checksum in hex file
const BYTE RL_READ_ERR	= 4;	// file reading error
const BYTE RL_OK		= 5;	// line was read OK

const BYTE SL_EMPTY		= 6;	// buffer is empty, no data was saved
const BYTE SL_FILE_ERR	= 7;	// unable to create specified file

typedef struct
{
	BYTE NumBytes;	// number of data bytes in line
	WORD StartAddr;	// start address of data
	BYTE LineType;	// type of line (data, EOF, etc)
	BYTE Data[40];	// data itself
	BYTE Checksum;	// line checksum
}HEX_LINE;

class HexBuff
{
private:
	BOOL m_bEmpty;
	BYTE* pDataBank;			// pointer to dynamically allocated mem. buffer
	HEX_LINE LinBuff;			// buffer for hex strings
	HANDLE Hnd;					// handle to input hex file
	ULONG ulSize;				// the size of allocated memory buffer

	BYTE ReadHexLine();
	BYTE CalcChecksum(BYTE* BinBuff, BYTE NumBytes);
	BOOL CheckForValidData(BYTE* BinBuff, BYTE NumBytes);
	void FormatSegAddrLine(BYTE* BinBuff, DWORD Addr);
	BOOL FormatHexDataLine(BYTE* BinBuff, DWORD StartAddr, BYTE NumBytes);
	void PrintHexLine(char* TxtBuff, BYTE* BinBuff, BYTE NumBytes, DWORD* Cnt);
	void PrintEOF(char* TxtBuff, DWORD* Cnt);

public:
	void ClearBuffer();
	void FillBuffer(BYTE Filler);
	void RandomFill();
	void SetMember(DWORD Address, BYTE Value);
	BYTE GetMember(DWORD Address);
	BYTE ReadHexFile(LPCTSTR Filename);
	BYTE SaveHexFile(LPCTSTR Filename = "default.hex",
					 BOOL Long = FALSE, BOOL Optimize = FALSE);
	ULONG GetTotalSize();
	ULONG GetDataSize();

	// constructor
	HexBuff(ULONG ulRequestedSize = BANK_SIZE);

	// destructor
	~HexBuff();
};

#endif // !defined(AFX_HEXBUFF_H__CFD36706_E139_11D3_8E5B_00105A92A724__INCLUDED_)
