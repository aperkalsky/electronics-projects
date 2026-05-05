// PROG_LPT.cpp - main module for VxD PROG_LPT

#define DEVICE_MAIN
#include "prog_lpt.h"
//#include "IoDefs.h"
Declare_Virtual_Device(PROG_LPT)
#undef DEVICE_MAIN

//ProgVM::ProgVM(VMHANDLE hVM) : VVirtualMachine(hVM) {}

//ProgThread::ProgThread(THREADHANDLE hThread) : VThread(hThread) {}

/*
	Function name: OnSysDynamicDeviceInit()

	Description:
		This function is called when the VXD is loaded. It is responsible for initialization
		of the driver

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
BOOL ProgDevice::OnSysDynamicDeviceInit()
{
	dprintf("OnSysDynamicDeviceInit() : Enter\n");

	// read the LPT1 port status and store the values needed to be restored on exit.
	m_bDataReg = m_bDataRegInitialVal = _inp(DataPort);
	m_bControlReg = m_bControlRegInitialVal = _inp(ControlPort);
	m_bStatusReg = _inp(StatusPort);

	// initialize 82C55
	Init8255();

	dprintf("OnSysDynamicDeviceInit() : Exit\n");

	return TRUE;
}

/*
	Function name: OnSysDynamicDeviceExit()

	Description:
		This function is called when the VXD is unloaded. It is responsible for cleanup.

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
BOOL ProgDevice::OnSysDynamicDeviceExit()
{
	dprintf("OnSysDynamicDeviceExit() : Enter\n");

	// restore LPT1 initial state
	_outp(DataPort, m_bDataRegInitialVal);
	_outp(ControlPort, m_bControlRegInitialVal);

	dprintf("OnSysDynamicDeviceExit() : Exit\n");

	return TRUE;
}

/*
	Function name: OnW32DeviceIoControl()

	Description:
		This is an entry point to the driver for user IOCTL calls

	Input:
		p

	Output:
		p

	Return Value:
		DEVIOCTL_NOERROR or other
*/
DWORD ProgDevice::OnW32DeviceIoControl(PIOCTLPARAMS p)
{
	dprintf("OnW32DeviceIoControl : IOCtl code received is %08X\n", p->dioc_IOCtlCode);

    switch(p->dioc_IOCtlCode)
    {
        case DIOC_OPEN:             // = 0
        case DIOC_CLOSEHANDLE:      // = -1
            return DEVIOCTL_NOERROR;

        case PROG_IOCTL_SET_ADDRESS: // = 1
			return IoSetAddress(p);

        case PROG_IOCTL_SET_DATA:	// = 2
			return IoSetData(p);

        case PROG_IOCTL_READ_DATA:	// = 3
			return IoReadData(p);

        case PROG_IOCTL_BURN:	// = 4
			return IoBurn(p);

        case PROG_IOCTL_SELF_TEST:	// = 5
			return IoSelfTest(p);

        default:
			dprintf("PROG_LPT: Invalid IOCtl code\n");
            return -1;  // error code
    }
}


/*
	Function name: SetHighFlag()

	Description:
		This function instructs programmer to treat the next nibble sent as a high nibble

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::SetHighFlag()
{
	m_bDataReg |= 0x10;		// set bit #4

	_outp(DataPort, m_bDataReg);
	
	Delay();
}

/*
	Function name: SetLowFlag()

	Description:
		This function instructs programmer to treat the next nibble sent as a low nibble

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::SetLowFlag()
{
	m_bDataReg &= 0xEF;		// clear bit #4

	_outp(DataPort, m_bDataReg);
	
	Delay();
}

/*
	Function name: PutNibble()

	Description:
		This function sets a nibble to be set on the parallel port pins and requests a programmer
		to latch it.

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::PutNibble(BYTE bOutNibble)
{
	// clear previous data
	m_bDataReg &= 0xF0;

	// set nibble. To be sure, mask off the higher nibble from the requested value
	m_bDataReg |= (bOutNibble & 0x0F);
	
	_outp(DataPort, m_bDataReg);

	Delay();
}

/*
	Function name: GetNibble()

	Description:
		This function reads a nibble from the programmer.

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
BYTE ProgDevice::GetNibble()
{
	BYTE Result = _inp(StatusPort);

	return ((Result >> 3) & 0x0F);
}

/*
	Function name: SetAddr8255()

	Description:
		This functions selects the active port of 82C55

	Input:
		bAddr - address of the port

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::SetAddr8255(BYTE bAddr)
{
	// clear bits #5 and #6
	m_bDataReg &= 0x9F;

	// decode address
	switch(bAddr)
	{
	case PortA:
		_outp(DataPort, m_bDataReg);
		break;

	case PortB:
		m_bDataReg |= 0x20;
		_outp(DataPort, m_bDataReg);
		break;

	case PortC:
		m_bDataReg |= 0x40;
		_outp(DataPort, m_bDataReg);
		break;

	case Control:
		m_bDataReg |= 0x60;
		_outp(DataPort, m_bDataReg);
		break;

	default:
		dprintf("Invalid 8255 address\n");
		break;
	}

	Delay();
}

/*
	Function name: Strobe8255()

	Description:
		This function strobes 82C55 chip

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::Strobe8255()
{
	// set output to 0 (active level)
	m_bControlReg |= 0x01;
	_outp(ControlPort, m_bControlReg);
	Delay();

	// set output to 1
	m_bControlReg &= 0xFE;
	_outp(ControlPort, m_bControlReg);
	Delay();
}

/*
	Function name: EpromEnableOn()

	Description:
		This function sets #CE pin of EPROM to 0, thus enabling the chip.

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::EpromEnableOn()
{
	m_bControlReg |= 0x02;	// set output to 0;
	_outp(ControlPort, m_bControlReg);
	Delay();
}

/*
	Function name: EpromEnableOff()

	Description:
		This function sets #CE pin of EPROM to 1, thus disabling the chip.

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::EpromEnableOff()
{
	m_bControlReg &= 0xFD;	// set output to 1;
	_outp(ControlPort, m_bControlReg);
	Delay();
}

/*
	Function name: BurnVoltageOn()

	Description:
		This function turns on programming voltage on programmer

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::BurnVoltageOn()
{
	dprintf("BurnVoltageOn() : Enter\n");

	m_bControlReg &= 0xF7;	// set output to 1;

	_outp(ControlPort, m_bControlReg);

	Delay();

	dprintf("BurnVoltageOn() : Exit\n");
}

/*
	Function name: BurnVoltageOff()

	Description:
		This function turns off programming voltage on programmer

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::BurnVoltageOff()
{
	dprintf("BurnVoltageOff() : Enter\n");

	m_bControlReg |= 0x08;	// set output to 0;

	_outp(ControlPort, m_bControlReg);

	Delay();

	dprintf("BurnVoltageOff() : Exit\n");
}

/*
	Function name: Init8255()

	Description:
		This function initializes 82C55 I/O port of the programmer to the desired state.

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::Init8255()
{
	dprintf("Init8255() : Enter\n");

	// select Control Register of 8255
	SetAddr8255(Control);

	// set high nibble
	SetHighFlag();
	PutNibble(0x08);

	// set low nibble
	SetLowFlag();
	PutNibble(0x00);

	// send a strobe to transmit data
	Strobe8255();

	dprintf("Init8255() : Exit\n");
}

/*
	Function name: ProgPulse()

	Description:
		This function generates a programmer's program pulse.

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::ProgPulse()
{
	// apply programming voltage
	BurnVoltageOn();
//	Delay();	// ??? it's present in previous function

	// set EPROM enable pin
	EpromEnableOn();

	// make a delay required for programming data (~ 120 uS)
	// TODO
//	VTD_Delay(50);	// at the moment do this in such a quick and dirty way, and later
					// use the function based on time measurements
//	CONFIGMG_Yield(120, CM_YIELD_NO_RESUME_EXEC);
	Delay(200);	// make it larger than 120 us

	// disale EPROM
	EpromEnableOff();
//	Delay();

	// turn off programming voltage
	BurnVoltageOff();
//	Delay();
}

/*
	Function name: SetAddress()

	Description:
		This function sets the requested address on the EPROM's address bus

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::SetAddress(WORD wAddr)
{
	// new variant
	BYTE AddrLow = LOBYTE(wAddr);	// low part of address
	BYTE AddrHigh = HIBYTE(wAddr);	// high part of address

	// set low byte of address
	SetAddr8255(PortA);

	SetLowFlag();
	PutNibble(LONIBBLE(AddrLow));
	
	SetHighFlag();
	PutNibble(HINIBBLE(AddrLow));
	
	Strobe8255();

	// set high byte of address
	SetAddr8255(PortB);

	SetLowFlag();
	PutNibble(LONIBBLE(AddrHigh));
	
	SetHighFlag();
	PutNibble(HINIBBLE(AddrHigh));
	
	Strobe8255();
}

/*
	Function name: SetData()

	Description:
		This function sets the desired byte on the data bus of EPROM. This byte will be
		programmed into it.

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::SetData(BYTE bData)
{
	dprintf("SetData() : Enter\n");

	// store the transferred data in internal variable for later comparison by burn routine
	m_bDataToBurn = bData;

	// set the data byte on programmer
	SetAddr8255(PortC);

	SetLowFlag();
	PutNibble(LONIBBLE(bData));
	
	SetHighFlag();
	PutNibble(HINIBBLE(bData));
	
	Strobe8255();

	dprintf("SetData() : Exit\n");
}

/*
	Function name: ReadData()

	Description:
		This function reads the data byte from the EPROM. It assumes that the address was already set before.

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
BYTE ProgDevice::ReadData()
{
	BYTE HighPart, LowPart, bDataRead;

	// set low flag before we activate EPROM. This will eliminate read errors
	SetLowFlag();

	// activate EPROM
	EpromEnableOn();

	Delay();	// let the data set up

	LowPart = GetNibble();

	SetHighFlag();

	Delay();	// let the data set up
	
	HighPart = GetNibble();

	// disable EPROM
	EpromEnableOff();

	// join two nibbles together
	bDataRead = LowPart | (HighPart << 4);

	dprintf("Data read = %02Xh\n", bDataRead);

	return bDataRead;
}

/*
	Function name: Burn()

	Description:
		This function assumes that the desired address and data were already set. It initiates
		a program pulse, reads the result back and verifies that the data was programmed
		correctly. In the case of failure it retries up to 25 times and then exits

	Input:
		bSuccess - pointer to result flag
		bDataToBurn - desired data value is copied to this location by this function
		bActualResult - 

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::Burn(BURN_STRUCT* pIoBuf)
{
	BYTE NumAttempts = 0;	// number of burn attempts
	BYTE Result;			// burned value

	// attempt to burn a bute and verify result
	do
	{
		ProgPulse();
		Delay(10);	// wait before reading data back
		Result = ReadData();
		NumAttempts++;
	}
	while((Result != m_bDataToBurn) && (NumAttempts < MAX_NUM_ATTEMPTS));

	// Fill I/O structure according to results
	pIoBuf->bSuccess = (Result == m_bDataToBurn);	// TRUE on success
	pIoBuf->bDataToBurn = m_bDataToBurn;
	pIoBuf->bActualResult = Result;
	pIoBuf->bNumRetries = NumAttempts;
}

/*
	Function name: SelfTest()

	Description:
		This function checks the presence of the programmer and correctness of its
		operation by setting the programmer into loopback mode and sending to it
		all possible data patterns

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
BOOL ProgDevice::SelfTest()
{
	BYTE Response, TestByte;

	dprintf("SelfTest() : Enter\n");

	TestByte = 0;

	// imitate EPROM programming
	BurnVoltageOn();

	// send all possible patterns in the loop
	for(WORD i = 0; i < 0xFF; i++, TestByte++)
	{
		SetData(TestByte);
		Response = ReadData();

		dprintf("Expected = %02X, received %02X\n", TestByte, Response);

		// verify response
		if(Response != TestByte)
			break;
	}

	// disconnect loopback
	BurnVoltageOff();

	dprintf("SelfTest() : Exit. NumTests = %d\n", i);

	return (BOOL)(Response == --TestByte);	// TRUE if last check passed. We decrement the
											// TestByte value since it was incremented on exit from
											// the loop

}

/*
	Function name: Delay()

	Description:
		This function performs a short delay (1 uS or less). The actual delay time is
		not PC - dependent.

	Input:
		None

	Output:
		None

	Return Value:
		None

	Notes: Try to use VTD_Delay()
*/
void ProgDevice::Delay(DWORD dwNumTicks)
{
	TickWait(dwNumTicks);
}

/*
	Function name: IoSetAddress()

	Description:
		This function performs a parameters check and internally calls SetAddress() function.

	Input:
		p->dioc_InBuf

	Output:
		None

	Return Value:
		DEVIOCTL_NOERROR
		VXD_ERR_INPUT_PARAMS
*/
DWORD ProgDevice::IoSetAddress(PIOCTLPARAMS p)
{
    WORD* pIn;          // pointer to the input buffer

    // check first that the input destination is valid
    if(!p->dioc_InBuf || p->dioc_cbInBuf < sizeof(WORD))
        return VXD_ERR_INPUT_PARAMS;

    // initialize pointer to I/O buffer
    pIn = (WORD*)(p->dioc_InBuf);

	// set address
	SetAddress(*pIn);

    // return OK
    return DEVIOCTL_NOERROR;
}

/*
	Function name: IoSetData()

	Description:
		This function performs a parameters check and internally calls SetData() function.

	Input:
		p->dioc_InBuf

	Output:
		None

	Return Value:
		DEVIOCTL_NOERROR
		VXD_ERR_INPUT_PARAMS
*/
DWORD ProgDevice::IoSetData(PIOCTLPARAMS p)
{
    BYTE* pIn;          // pointer to the input buffer

    // check first that the input destination is valid
    if(!p->dioc_InBuf || p->dioc_cbInBuf < sizeof(BYTE))
        return VXD_ERR_INPUT_PARAMS;

    // initialize pointer to I/O buffer
    pIn = (BYTE*)(p->dioc_InBuf);

	// set data
	SetData(*pIn);

    // return OK
    return DEVIOCTL_NOERROR;
}

/*
	Function name: IoReadData()

	Description:
		This function performs a parameters check and internally calls ReadData() function.

	Input:
		p->dioc_InBuf

	Output:
		None

	Return Value:
		DEVIOCTL_NOERROR
		VXD_ERR_INPUT_PARAMS
*/
DWORD ProgDevice::IoReadData(PIOCTLPARAMS p)
{
	BYTE* pOut;	// pointer to the output buffer

    // check first that the output destination is valid
    if(!p->dioc_OutBuf || p->dioc_cbOutBuf < sizeof(BYTE))
        return VXD_ERR_INPUT_PARAMS;

    // initialize pointer to I/O buffer
    pOut = (BYTE*)(p->dioc_OutBuf);

	// perform data read
	*pOut = ReadData();

    // return OK
    return DEVIOCTL_NOERROR;
}

/*
	Function name: IoBurn()

	Description:
		This function performs a parameters check and internally calls Burn() function.

	Input:
		p->dioc_InBuf

	Output:
		None

	Return Value:
		DEVIOCTL_NOERROR
		VXD_ERR_INPUT_PARAMS
*/
DWORD ProgDevice::IoBurn(PIOCTLPARAMS p)
{
//	BURN_STRUCT* pIn;	// pointer to the input and output buffer
	BURN_STRUCT* pOut;

    // check first that the input destination is valid
//    if(!p->dioc_InBuf || p->dioc_cbInBuf < sizeof(BURN_STRUCT))
//        return VXD_ERR_INPUT_PARAMS;

    // check also that the output destination is valid
    if(!p->dioc_OutBuf || p->dioc_cbOutBuf < sizeof(BURN_STRUCT))
        return VXD_ERR_INPUT_PARAMS;

    // initialize pointers to I/O buffers
//    pIn = (BURN_STRUCT*)(p->dioc_InBuf);
    pOut = (BURN_STRUCT*)(p->dioc_OutBuf);

	// perform programming of a byte
	Burn(pOut);

    // return OK
    return DEVIOCTL_NOERROR;
}

/*
	Function name: IoSelfTest()

	Description:
		This function performs a parameters check and internally calls SelfTest() function.

	Input:
		p->dioc_InBuf

	Output:
		None

	Return Value:
		DEVIOCTL_NOERROR
		VXD_ERR_INPUT_PARAMS
*/
DWORD ProgDevice::IoSelfTest(PIOCTLPARAMS p)
{
	BOOL* pOut;	// pointer to the output buffer

    // check first that the output destination is valid
    if(!p->dioc_OutBuf || p->dioc_cbOutBuf < sizeof(BOOL))
        return VXD_ERR_OUTPUT_PARAMS;

    // initialize pointer to I/O buffer
    pOut = (BOOL*)(p->dioc_OutBuf);

	// perform self test
	*pOut = SelfTest();

    // return OK
    return DEVIOCTL_NOERROR;
}

/*
	Function name: TickWait()

	Description:
		This function returns after desired number of ticks elapsed (1tick ~ 0.8uS)

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void ProgDevice::TickWait(DWORD dwNumTicks)
{
    DWORD RtcHi, RtcLo;     // high and low dwords holding RTC tick count (since Win95 start)
    DWORD StartTime = 0;    // timer start time
    DWORD Delta = 0xFFFFFFFF - StartTime;
    DWORD StopTime;         // timer stop time

    // measure start time
    VTD_Get_Real_Time(&RtcHi, &RtcLo);
    StartTime = RtcLo;

    // enter wait loop
    if(Delta >= dwNumTicks)   // timer doesn't come across zero
    {
        StopTime = StartTime + dwNumTicks;

        do
            {VTD_Get_Real_Time(&RtcHi, &RtcLo);}
        while (RtcLo < StopTime);
    }
    else                    // timer comes across zero
    {
        StopTime = dwNumTicks - Delta;
        do
            {VTD_Get_Real_Time(&RtcHi, &RtcLo);}
        while ((RtcLo > StartTime) || (RtcLo <= StopTime));
    }
}
