// ProgInterface.h: interface for the ProgInterface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROGINTERFACE_H__CFD36704_E139_11D3_8E5B_00105A92A724__INCLUDED_)
#define AFX_PROGINTERFACE_H__CFD36704_E139_11D3_8E5B_00105A92A724__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "..\..\VtoolsD\Prog_Lpt\IoDefs.h"

const DWORD MAX_EPROM_ADDR	= 0xFFFF;

class ProgInterface  
{
public:
	BOOL PrCheckBlankState(CProgressCtrl& cProgress);
	BOOL PrSelfTest();
	BOOL PrReadData(BYTE* bData);
	BOOL PrSetData(BYTE bData);
	BOOL PrSetAddress(WORD wAddr);
	BOOL PrBurn(BURN_STRUCT* pBurnStruct);
	BOOL IsReady();
	ProgInterface();
	virtual ~ProgInterface();

private:
	BOOL m_bIoctlStatus;	// status of IOCTL call
	DWORD m_dwByteCnt;		// counter of bytes received from IOCTL call
	BOOL LoadDriver();
	HANDLE m_hDriver;
};

#endif // !defined(AFX_PROGINTERFACE_H__CFD36704_E139_11D3_8E5B_00105A92A724__INCLUDED_)
