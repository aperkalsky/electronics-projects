// HexBuff.cpp: implementation of the HexBuff class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Programmer.h"
#include "HexBuff.h"
#include "Utils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*
	Object constructor
*/
HexBuff::HexBuff(ULONG ulRequestedSize)
{
	// allocate the buffer requested
	pDataBank = new BYTE[ulRequestedSize];

	// check that the allocation was OK
	if(pDataBank)
	{
		ulSize = ulRequestedSize;
		ClearBuffer();
	}
	else
		ulSize = 0;
}

/*
	Object destructor
*/
HexBuff::~HexBuff()
{
	delete pDataBank;
}

/*
	ReadHexLine - reads line from hex file

  Return Values:
  RL_OK			- line was read
  RL_INV		- the line doesn't start with ':'
  RL_BAD_CS		- bad line checksum
  RL_READ_ERR	- error when reading from file
*/
BYTE HexBuff::ReadHexLine()
{
	BYTE  Buffer[100];		// temporary buffer
	BYTE  Checksum = 0;		// checksum read from file
	WORD LineChecksum = 0;	// calculated checksum of data from line
	DWORD NumRead = 0;		// num bytes actually read
	DWORD NumToRead = 0;	// num bytes to read

	// start reading from file
	if(!::ReadFile(Hnd, (LPVOID)Buffer, 1, &NumRead, NULL))
		return RL_READ_ERR;

	// check that line starts with ":"
	if(Buffer[0] != ':')
		return RL_INV;

	// read line header
	if(!::ReadFile(Hnd, (LPVOID)Buffer, 8, &NumRead, NULL))
		return RL_READ_ERR;

	// interprete it
	ByteHexToBin(&Buffer[0], &LinBuff.NumBytes);
	HexWordToNum(&Buffer[2], &LinBuff.StartAddr);
	ByteHexToBin(&Buffer[6], &LinBuff.LineType);

	// read the rest of data
	if(LinBuff.LineType != 0x01)
	{
		if(!::ReadFile(Hnd, (LPVOID)Buffer, LinBuff.NumBytes*2, &NumRead, NULL))
			return RL_READ_ERR;
		
		for(BYTE Index = 0; Index < LinBuff.NumBytes; Index++)
		{
			ByteHexToBin(&Buffer[Index*2], &LinBuff.Data[Index]);
			LineChecksum += LinBuff.Data[Index];	// calculate partial checksum
		}
	}

	// read line checksum
	if(!::ReadFile(Hnd, (LPVOID)Buffer, 2, &NumRead, NULL))
		return RL_READ_ERR;
	else
		ByteHexToBin(&Buffer[0], &Checksum);

	// read end-of-line char
	if(!::ReadFile(Hnd, (LPVOID)Buffer, 2, &NumRead, NULL))
		return RL_READ_ERR;
	
	// calculate rest of checksum and compare it to desired
	LineChecksum += LinBuff.NumBytes + 
					LinBuff.LineType + 
					LOBYTE(LinBuff.StartAddr) +
					HIBYTE(LinBuff.StartAddr);
	if((LineChecksum + Checksum) & 0x00FF)
		return RL_BAD_CS;
	else
		return RL_OK;
}

/*
	CalcChecksum - calculates the checksum to place at the end of hex line

	Inputs:
		BinBuff - pointer to binary buffer for output data
		NumBytes - number of bytes used in checksum calculation

	Return Value:
		The checksum calculated
*/
BYTE HexBuff::CalcChecksum(BYTE* BinBuff, BYTE NumBytes)
{
	WORD CurrCs = 0;

	// sum all the bytes
	for(int i = 0; i < NumBytes; i++)
		CurrCs += BinBuff[i];

	// return the complemented checksum
	return (0 - LOBYTE(CurrCs));
}

/*
	HaveValidData - checks that hex buffer contain non-0xFF data

  Return value:
		TRUE - isn't blank
		FALSE - blank
*/
BOOL HexBuff::CheckForValidData(BYTE* BinBuff, BYTE NumBytes)
{
	BOOL RetVal = FALSE;

	// check for blank
	for(int i = 0; i < NumBytes; i++)
	{
		if(BinBuff[i+4] != 0xFF)
			RetVal = TRUE;
	}
	
	return RetVal;
}

/*
	FormatSegAddrLine - prepares the data in binary buffer for printing
						the segment address line in the hex file

	Inputs:
		BinBuff - pointer to binary buffer for output data
		Addr - full address
*/
void HexBuff::FormatSegAddrLine(BYTE* BinBuff, DWORD Addr)
{
	// get segment address
	WORD SegmAddr = (WORD)(((--Addr) >> 4) & (~0xFFF));
//	WORD SegmAddr = (WORD)((Addr - 0x10000) >> 4);

	// FORMAT THE BINARY BUFFER
	
	// put data length
	BinBuff[0] = 0x02;

	// put address (not in use)
	BinBuff[1] = BinBuff[2] = 0;

	// put line type (segment address)
	BinBuff[3] = 0x02;

	// put segment address
	BinBuff[4] = HIBYTE(SegmAddr);
	BinBuff[5] = LOBYTE(SegmAddr);

	// put checksum
	BinBuff[6] = CalcChecksum(BinBuff, 6);
}

/*
	FormatHexDataLine - prepares the data in binary buffer for printing
						the data line in the hex file
	Inputs:
		BinBuff - pointer to binary buffer for output data
		StartAddr - address of the first data member
		NumBytes - number of data bytes to put into the line

    Return value:
		TRUE - hex line contains non-FF data
		FALSE - hex line is blank
*/
BOOL HexBuff::FormatHexDataLine(BYTE* BinBuff, DWORD StartAddr, BYTE NumBytes)
{
	// get address within the segment
	WORD Addr = (WORD)StartAddr;

	// FORMAT THE BINARY BUFFER
	
	// put data length
	BinBuff[0] = NumBytes;

	// put start address
	BinBuff[1] = HIBYTE(Addr);
	BinBuff[2] = LOBYTE(Addr);

	// put line type (data)
	BinBuff[3] = 0x00;

	// put data
	for(int i = 0; i < NumBytes; i++)
		BinBuff[i+4] = pDataBank[i + StartAddr];

	// put checksum
	BinBuff[i+4] = CalcChecksum(BinBuff, NumBytes + 4);

	// test the line for blank state
	return CheckForValidData(BinBuff, NumBytes);
}

/*
	PrintHexLine - converts the binary line data for the hex file to the
				   text form

	Inputs:
		TxtBuff - pointer to buffer for output data
		BinBuff - pointer to binary buffer to take the data from
		NumBytes - number of bytes to print
		Cnt - pointer to byte counter of TxtBuff
*/
void HexBuff::PrintHexLine(char* TxtBuff, BYTE* BinBuff, BYTE NumBytes, DWORD* Cnt)
{
	// start hex line start mark
	*Cnt += wsprintf((LPTSTR)(TxtBuff + *Cnt),":");

	// print here the data from binary buffer
	for(int i = 0; i < NumBytes; i++)
		*Cnt += wsprintf((LPTSTR)(TxtBuff + *Cnt),"%02X", BinBuff[i]);

	// print line break characters
	*Cnt += wsprintf((LPTSTR)(TxtBuff + *Cnt),"\r\n");
}

/*
	PrintEOF - prints the termination line into the hex file
*/
void HexBuff::PrintEOF(char* TxtBuff, DWORD* Cnt)
{
	*Cnt += wsprintf((LPTSTR)(TxtBuff + *Cnt),":00000001FF");
}

/*
	ClearBuffer - clears data bank
*/
void HexBuff::ClearBuffer()
{
	// fill buffer with 0xFF pattern
	FillBuffer(0xFF);

	// set empty flag
	m_bEmpty = TRUE;
}

/*
	FillBuffer - fills data bank with specified value
*/
void HexBuff::FillBuffer(BYTE Filler)
{
	// fill buffer
	for(DWORD Addr = 0; Addr < ulSize; Addr++)
		pDataBank[Addr] = Filler;

	// clear empty flag
	m_bEmpty = FALSE;
}

/*
	RandomFill - fills buffer with the random pattern
*/
void HexBuff::RandomFill()
{
	// fill buffer
	for(DWORD Addr = 0; Addr < ulSize; Addr++)
		pDataBank[Addr] = (BYTE)(rand());

	// clear empty flag
	m_bEmpty = FALSE;
}
/*
	SetMember - sets specified data bank member with desired value
*/
void HexBuff::SetMember(DWORD Address, BYTE Value)
{
	// check that the address is valid
	if(Address > ((DWORD)ulSize - 1))
		return;
	else
		pDataBank[Address] = Value;

	// clear empty flag in assumption that the actual data was set
	m_bEmpty = FALSE;
}

/*
	GetMember - returns value of specified data bank member
*/
BYTE HexBuff::GetMember(DWORD Address)
{
	// check that the address is valid
	if(Address > ((DWORD)ulSize - 1))
		return 0xFF;
	else
		return pDataBank[Address];
}

/*
	ReadHexFile - loads data bank with specified HEX file contents

  Return Values:
	
	  General:

  BUFF_NO_FILE - unable to open input file
  BUFF_OK - file was read

	  Error diagnostics:
  
  RL_INV		- the line doesn't start with ':'
  RL_BAD_CS		- bad line checksum
  RL_READ_ERR	- error when reading from file
*/
BYTE HexBuff::ReadHexFile(LPCTSTR Filename)
{
	DWORD FileSize = 0;		// file size in bytes
	DWORD NumBytesRead = 0;	// counter of bytes read
	DWORD CurrAddress = 0;	// current address in data bank
	DWORD SegmentAddress = 0;	// start address of segment
	BYTE ReturnStatus = 0;

	// open hex file
	Hnd = ::CreateFile(Filename, GENERIC_READ, NULL, NULL,
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// check that file exists
	if(Hnd == INVALID_HANDLE_VALUE)
		return BUFF_NO_FILE;

	// start processing it
	FileSize = ::GetFileSize(Hnd, NULL);

	do
	{	// read every line
		if((ReturnStatus = ReadHexLine()) != RL_OK)
		{
			::CloseHandle(Hnd);
			return ReturnStatus;
		}
		else
		{
			switch(LinBuff.LineType)
			{
			case 0x00:
				{
				CurrAddress = SegmentAddress + LinBuff.StartAddr;
				for(BYTE Index = 0; Index < LinBuff.NumBytes; Index++, CurrAddress++)
					SetMember(CurrAddress, LinBuff.Data[Index]);
				}
				break;
			case 0x02:
				SegmentAddress = (((LinBuff.Data[0] << 8) + LinBuff.Data[1]) << 4);
				break;
			default:
				break;
			}
		}

		NumBytesRead += LinBuff.NumBytes*2 + 13;	// 13 - header length + end of line

	}while(NumBytesRead < FileSize);

	// close file
	::CloseHandle(Hnd);

	// clear Empty flag
	m_bEmpty = FALSE;

	return BUFF_OK;
}

/*
	SaveHexFile - saves the memory buffer contents into the file
				  Supports 16-byte and 32-byte string lengths

  Inputs:
		Filename - file name to create
		Long - TRUE: use 32-byte lines, otherwise 16-byte
		Optimize - TRUE: write to file only the lines with data
	Return Values:
		BUFF_OK		- buffer was saved
		SL_EMPTY	- buffer is empty, no data was saved
		SL_FILE_ERR - unable to create specified file
*/
BYTE HexBuff::SaveHexFile(LPCTSTR Filename, BOOL Long, BOOL Optimize)
{
	DWORD CurrAddr = 0;				// current data byte address
	DWORD LastAddr;					// last data address to save
	DWORD LastSegmAddr = 0x10000;	// max last address in current segment
	DWORD LastCurrSegmAddr;			// actual last address within current segment
	DWORD Cnt = 0;					// counter of bytes in the output buffer
	DWORD NumBytesWritten = 0;		// counter of bytes written to file
	BYTE MaxNumInLine;				// max. number of bytes in hex line
	BYTE NumToWrite;				// number of bytes to write into one line

	LPSTR OutBuff = 0;				// buffer for collecting output data
	BYTE BinLineBuff[40];			// binary buffer holding hex line contents
	BYTE RetStatus = BUFF_OK;		// return status of the function
	BOOL HaveData;					// flag specifying if hex line contains valid data

	HANDLE LogHnd;					// Handle to the output file

	// check first that the buffer isn't empty
	LastAddr = (DWORD)GetDataSize();
	
	if(LastAddr == 1)
		return SL_EMPTY;

	// set the appropriate hex line length
	if(Long)
		MaxNumInLine = 0x20;
	else
		MaxNumInLine = 0x10;

	// create the output buffer
	OutBuff = new char[(LastAddr * 3) + 100];

	// start formatting the output buffer
	do
	{
		// PROCESS EACH 64K SEGMENT SEPARATELY

		// define the last address within current segment
		LastCurrSegmAddr = min(LastSegmAddr, LastAddr);
		
		// print segment address info into the buffer
		FormatSegAddrLine(BinLineBuff, LastCurrSegmAddr);
		PrintHexLine(OutBuff, BinLineBuff, 7, &Cnt);

		// fill the segment with data
		while(CurrAddr < LastCurrSegmAddr)
		{
			// PRINT IT STRING BY STRING

			// define number of bytes in the string
			NumToWrite = (BYTE)min((DWORD)MaxNumInLine, (LastCurrSegmAddr - CurrAddr));
			
			// format the line and check its contents
			HaveData = FormatHexDataLine(BinLineBuff, CurrAddr, NumToWrite);

			// check if optimization is needed
			if(Optimize)
			{
				if(HaveData)	// print only lines with data
					PrintHexLine(OutBuff, BinLineBuff, NumToWrite + 5, &Cnt);
			}
			else	// print every line
				PrintHexLine(OutBuff, BinLineBuff, NumToWrite + 5, &Cnt);

			// increment the current address
			CurrAddr += NumToWrite;
		}

		// switch to the next segment
		LastSegmAddr += 0x10000;

	}while(CurrAddr < LastAddr);

	// append the EOF string
	PrintEOF(OutBuff, &Cnt);

	// create the output file
	LogHnd = CreateFile(Filename, GENERIC_WRITE, FILE_SHARE_READ, NULL,
						CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// if OK, flush the buffer into it
	if(LogHnd)
		{
		WriteFile(LogHnd,(LPCVOID)OutBuff, Cnt, &NumBytesWritten, NULL);
		CloseHandle(LogHnd);
		RetStatus =  BUFF_OK;
		}
	else
		RetStatus =  SL_FILE_ERR;

	// delete allocated buffer
	delete OutBuff;
		
	// return the status
	return RetStatus;
}

/*
	GetTotalSize() - returns the total size of data buffer
*/
ULONG HexBuff::GetTotalSize()
{
	return ulSize;
}

/*
	GetDataSize() - returns the size of actual data stored in the buffer
*/
ULONG HexBuff::GetDataSize()
{
	ULONG DataSize = ulSize; // actual data size, set it to maximum first 

	// check first that the buffer contains data
	if(m_bEmpty)
		return 0;

	// decrement the size until reaching non-FF value
	do
	{
		DataSize--;
	}
	while((DataSize > 0) && (GetMember(DataSize) == 0xFF));

	// at the moment DataSize equals the last address.
	// We need to consider also the zero address, so we
	// need to increment this variable
	return (DataSize + 1);
}