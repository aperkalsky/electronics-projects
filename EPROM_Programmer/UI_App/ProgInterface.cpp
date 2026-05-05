// ProgInterface.cpp: implementation of the ProgInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Programmer.h"
#include "ProgInterface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ProgInterface::ProgInterface()
{
	// connect to the VxD
	LoadDriver();
}

ProgInterface::~ProgInterface()
{
	// disconnect from VxD
	if(m_hDriver)
		CloseHandle(m_hDriver);
}

BOOL ProgInterface::LoadDriver()
{
	// create a handle to the device driver
	m_hDriver = CreateFile("\\\\.\\PROG_LPT.VXD",0,0,0,0,FILE_FLAG_DELETE_ON_CLOSE,0);
	
	// check for errors
	if(m_hDriver == INVALID_HANDLE_VALUE)
	{
		m_hDriver = 0;
		return FALSE;
	}
	else
		return TRUE;
}

/*
	Function name: IsReady()

	Description:
		This function is called by the object who uses the ProgInterface object in order to
		check that the communication to the programmer is possible

	Input:
		None

	Output:
		None

	Return Value:
		TRUE - OK
		FALSE - no driver was open
*/
BOOL ProgInterface::IsReady()
{
	// check for valid handle to driver and report accordingly
	return (BOOL)m_hDriver;
}

/*
	Function name: PrBurn()

	Description:
		This function requests a driver to burn a byte

	Input:
		pBurnStruct - pointer to the burn structure

	Output:
		None

	Return Value:
		None
*/
BOOL ProgInterface::PrBurn(BURN_STRUCT* pBurnStruct)
{
	m_dwByteCnt = 0;

	// check that the handle is valid
	if(!IsReady())
		return FALSE;

	// send an IOCTL call
	m_bIoctlStatus = DeviceIoControl(m_hDriver, 
									 PROG_IOCTL_BURN,
									 NULL,
									 0,
									 (LPVOID)pBurnStruct,
									 sizeof(BURN_STRUCT),
									 &m_dwByteCnt, 
									 NULL);

	return m_bIoctlStatus;
}

/*
	Function name: PrSetAddress()

	Description:
		This function requests driver to set a requested EPROM address

	Input:
		wAddr - address to set

	Output:
		None

	Return Value:
		None
*/
BOOL ProgInterface::PrSetAddress(WORD wAddr)
{
	m_dwByteCnt = 0;

	// check that the handle is valid
	if(!IsReady())
		return FALSE;

	// send an IOCTL call
	m_bIoctlStatus = DeviceIoControl(m_hDriver, 
									 PROG_IOCTL_SET_ADDRESS,
									 (LPVOID)&wAddr,
									 sizeof(wAddr),
									 NULL,
									 0,
									 &m_dwByteCnt, 
									 NULL);

	return m_bIoctlStatus;
}

/*
	Function name: PrSetData()

	Description:
		This function requests driver to set a requested EPROM data

	Input:
		bData - data to set

	Output:
		None

	Return Value:
		None
*/
BOOL ProgInterface::PrSetData(BYTE bData)
{
	m_dwByteCnt = 0;

	// check that the handle is valid
	if(!IsReady())
		return FALSE;

	// send an IOCTL call
	m_bIoctlStatus = DeviceIoControl(m_hDriver, 
									 PROG_IOCTL_SET_DATA,
									 (LPVOID)&bData,
									 sizeof(bData),
									 NULL,
									 0,
									 &m_dwByteCnt, 
									 NULL);

	return m_bIoctlStatus;
}

/*
	Function name: PrReadData()

	Description:
		This function requests the driver to read data byte from the EEPROM

	Input:
		None

	Output:
		bData - pointer modified with data read

	Return Value:
		None
*/
BOOL ProgInterface::PrReadData(BYTE* bData)
{
	BYTE bDataRead;	// used as I/O buffer for driver call

	m_dwByteCnt = 0;

	// check that the handle is valid
	if(!IsReady())
		return FALSE;

	// send an IOCTL call
	m_bIoctlStatus = DeviceIoControl(m_hDriver, 
									 PROG_IOCTL_READ_DATA,
									 NULL,
									 0,
									 (LPVOID)&bDataRead,
									 sizeof(bDataRead),
									 &m_dwByteCnt, 
									 NULL);

	// if I/O call succeeded, update data
	if(m_bIoctlStatus)
		*bData = bDataRead;

	return m_bIoctlStatus;
}

/*
	Function name: PrSelfTest()

	Description:
		This function requests a programer to perform a self test

	Input:
		None

	Output:
		None

	Return Value:
		TRUE - OK
		FALSE - failed
*/
BOOL ProgInterface::PrSelfTest()
{
	BOOL bResult;

	m_dwByteCnt = 0;

	// check that the handle is valid
	if(!IsReady())
		return FALSE;

	// send an IOCTL call
	m_bIoctlStatus = DeviceIoControl(m_hDriver, 
									 PROG_IOCTL_SELF_TEST,
									 NULL,
									 0,
									 (LPVOID)&bResult,
									 sizeof(bResult),
									 &m_dwByteCnt, 
									 NULL);

	if(m_bIoctlStatus)
		return bResult;	// self test results
	else
		return FALSE;	// IOCTL call failed


}

/*
	Function name: PrCheckBlankState()

	Description:
		This function reads each EPROM value and checks that it's 0xFF

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
BOOL ProgInterface::PrCheckBlankState(CProgressCtrl& cProgress)
{
	BOOL bResult = TRUE;
	BYTE bDataRead;
	WORD wAddr;

	// check that the handle is valid
	if(!IsReady())
		return FALSE;

	// set progress bar control range
	cProgress.SetRange(0,257);	// 258 steps
	cProgress.SetStep(1);

	for(DWORD wCurrAddr = 0; wCurrAddr <= MAX_EPROM_ADDR; wCurrAddr++)
	{
		m_dwByteCnt = 0;

		// set address
		// -----------
		wAddr = LOWORD(wCurrAddr);

		m_bIoctlStatus = DeviceIoControl(m_hDriver, 
										 PROG_IOCTL_SET_ADDRESS,
										 (LPVOID)&wAddr,
										 sizeof(wAddr),
										 NULL,
										 0,
										 &m_dwByteCnt,
										 NULL);

		// check for errors
		if(!m_bIoctlStatus)
		{
			AfxMessageBox("PrCheckBlankState() - IOCTL call failed");
			bResult = FALSE;
			break;
		}

		// read data
		// ---------
		m_bIoctlStatus = DeviceIoControl(m_hDriver, 
										 PROG_IOCTL_READ_DATA,
										 NULL,
										 0,
										 (LPVOID)&bDataRead,
										 sizeof(bDataRead),
										 &m_dwByteCnt,
										 NULL);

		// check for errors
		if(!m_bIoctlStatus)
		{
			AfxMessageBox("PrCheckBlankState() - IOCTL call failed");
			bResult = FALSE;
			break;
		}

		// verify that this is a default value
		if(bDataRead != 0xFF)
		{
			bResult = FALSE;
			break;
		}

		// advance progress bar
		if(wCurrAddr % 0xFF == 0)
			cProgress.StepIt();
	}

	// reset progress bar
	cProgress.SetPos(0);

	return bResult;
}
