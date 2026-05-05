// PROG_LPT.h - include file for VxD PROG_LPT

#include <vtoolscp.h>
#include "IoDefs.h"

#define DEVICE_CLASS		ProgDevice
#define PROG_LPT_DeviceID		UNDEFINED_DEVICE_ID
#define PROG_LPT_Init_Order	UNDEFINED_INIT_ORDER
#define PROG_LPT_Major		1
#define PROG_LPT_Minor		0

// Programmer interface definitions
//---------------------------------

// register addresses of LPT1
#define DataPort		0x378
#define	StatusPort		0x379
#define	ControlPort		0x37A

// port addresses of 82C55
#define PortA		0x00
#define PortB		0x01
#define	PortC		0x02
#define Control		0x03

#define MAX_NUM_ATTEMPTS	25

// data conversion definitions
#define LOBYTE(w)   ((BYTE) (w))
#define HIBYTE(w)	((BYTE) (((WORD) (w) >> 8) & 0xFF))

#define LONIBBLE(b)	((BYTE) (((BYTE) (b)) & 0x0F))
#define HINIBBLE(b)	((BYTE) (((BYTE) (b) >> 4) & 0x0F))

class ProgDevice : public VDevice
{
public:
	virtual BOOL OnSysDynamicDeviceInit();
	virtual BOOL OnSysDynamicDeviceExit();
	virtual DWORD OnW32DeviceIoControl(PIOCTLPARAMS pDIOCParams);
private:
	void TickWait(DWORD dwNumTicks);
	DWORD IoSelfTest(PIOCTLPARAMS p);
	DWORD IoBurn(PIOCTLPARAMS p);
	DWORD IoReadData(PIOCTLPARAMS p);
	DWORD IoSetData(PIOCTLPARAMS p);
	DWORD IoSetAddress(PIOCTLPARAMS p);
	BYTE m_bDataToBurn;
	BYTE m_bControlRegInitialVal;
	BYTE m_bDataRegInitialVal;
	void Delay(DWORD dwNumTicks = 5);
	BOOL SelfTest();
	void Burn(BURN_STRUCT* pIoBuf);
	BYTE ReadData();
	void SetData(BYTE bData);
	void SetAddress(WORD wAddr);
	void ProgPulse();
	void Init8255();
	void BurnVoltageOff();
	void BurnVoltageOn();
	void EpromEnableOff();
	void EpromEnableOn();
	void Strobe8255();
	void SetAddr8255(BYTE bAddr);
	BYTE GetNibble();
	void PutNibble(BYTE bOutNibble);
	void SetLowFlag();
	void SetHighFlag();
	BYTE m_bControlReg;	// initial value of LPT1 Control Register
	BYTE m_bStatusReg;	// initial value of LPT1 Status Register
	BYTE m_bDataReg;	// initial value of LPT1 Data Register
};

/*
class ProgVM : public VVirtualMachine
{
public:
	ProgVM(VMHANDLE hVM);
};

class ProgThread : public VThread
{
public:
	ProgThread(THREADHANDLE hThread);
};

*/

