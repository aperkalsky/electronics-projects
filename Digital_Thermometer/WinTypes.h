#ifndef _WINTYPES_H_
#define _WINTYPES_H_

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef bit BOOL;

// data conversion definitions
#define LOBYTE(w)   ((BYTE) (w))
#define HIBYTE(w)	((BYTE) (((WORD) (w) >> 8) & 0xFF))

#define LONIBBLE(b)	((BYTE) (((BYTE) (b)) & 0x0F))
#define HINIBBLE(b)	((BYTE) (((BYTE) (b) >> 4) & 0x0F))

// boolean definitions
#define TRUE	(BOOL)1
#define FALSE	(BOOL)0

#endif