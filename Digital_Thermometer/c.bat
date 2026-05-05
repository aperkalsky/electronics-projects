REM This Batch-File generates the THERMOMT program
REM 
del *.obj
C51 main.c
C51 lcd.c
C51 ds1820.c
pause
BL51 @THERMOMT.LIN
pause
oh51 main
