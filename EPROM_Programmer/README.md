This is a software for the hobby project developed back in year 2000

The project is a programmer for UV erased EPROMs, for example 27C512.
Interfacing is via PC parallel port.

The hardware part consists of the board connected to the PC parallel port, it uses 82C55 to
set up address and data signals of the EEPROM.

The software part was originally written in Borland C++ for MS-DOS, using its window library.
This version of software did not remain, here's the porting of this code to the Windows MFC app.

As the parallel port in Windows cannot be interfaced directly, a VxD driver was written in C++,
using the Numega Driver Studio for MS Visual C++. The debugging was done using the legendary
SoftICE/W