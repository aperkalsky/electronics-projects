#ifndef _IODEFS_H_
#define _IODEFS_H_

// IOCTL definitions for PROG_LPT.VXD
typedef struct
{
	BOOL bSuccess;
	BYTE bDataToBurn;
	BYTE bActualResult;
	BYTE bNumRetries;
} BURN_STRUCT;

// IOCTL calls
#define PROG_IOCTL_SET_ADDRESS		1
#define PROG_IOCTL_SET_DATA			2
#define PROG_IOCTL_READ_DATA		3
#define PROG_IOCTL_BURN				4
#define PROG_IOCTL_SELF_TEST		5

// error return code definitions
#define VXD_ERR_INPUT_PARAMS		1   // incorrect input parameter
#define VXD_ERR_OUTPUT_PARAMS		2	// incorrect output parameters

#endif