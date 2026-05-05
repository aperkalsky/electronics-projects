# PROG_LPT.mak - makefile for VxD PROG_LPT

DEVICENAME = PROG_LPT
DYNAMIC = 1
FRAMEWORK = CPP
DEBUG = 0
BROWSE = 1
OBJECTS = prog_lpt.OBJ

!include $(VTOOLSD)\include\vtoolsd.mak
!include $(VTOOLSD)\include\vxdtarg.mak

prog_lpt.OBJ:	prog_lpt.cpp prog_lpt.h