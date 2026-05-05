; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CProgrammerDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Programmer.h"

ClassCount=5
Class1=CProgrApp
Class2=CProgrammerDlg
Class3=CAboutDlg

ResourceCount=5
Resource1=IDD_PROGRAMMER_DIALOG
Resource2=IDR_MAINFRAME
Resource3=IDD_ABOUTBOX
Resource4=CG_IDD_PROGRESS
Class4=CFillDlg
Class5=CFillBuffDlg
Resource5=IDD_FILL_DIALOG

[CLS:CProgrApp]
Type=0
HeaderFile=Programmer.h
ImplementationFile=Programmer.cpp
Filter=N
LastObject=CProgrApp

[CLS:CProgrammerDlg]
Type=0
HeaderFile=ProgrammerDlg.h
ImplementationFile=ProgrammerDlg.cpp
Filter=D
LastObject=CProgrammerDlg
BaseClass=CDialog
VirtualFilter=dWC

[CLS:CAboutDlg]
Type=0
HeaderFile=ProgrammerDlg.h
ImplementationFile=ProgrammerDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_PROGRAMMER_DIALOG]
Type=1
Class=CProgrammerDlg
ControlCount=20
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1073807360
Control3=IDC_PROGRESS,msctls_progress32,1350565888
Control4=IDC_STATIC_EEPROM,button,1342177287
Control5=IDC_STATIC_BUFFER,button,1342177287
Control6=IDC_STATIC_STATUS_LINE,static,1350696961
Control7=IDC_STATIC_INPUT_FILE,button,1342177287
Control8=IDC_STATIC_OUTPUT_FILE,button,1342177287
Control9=IDC_STATIC_IF_TXT,static,1350696960
Control10=IDC_STATIC_OF_TXT,static,1350696960
Control11=IDC_BUTTON_CHECK_BLANK,button,1342242816
Control12=IDC_BUTTON_READ,button,1342242816
Control13=IDC_BUTTON_BURN,button,1342242816
Control14=IDC_BUTTON_VERIFY,button,1342242816
Control15=IDC_BUTTON_LOAD_BUFFER,button,1342242816
Control16=IDC_BUTTON_SAVE_BUFFER,button,1342242816
Control17=IDC_BUTTON_CLEAR_BUFFER,button,1342242816
Control18=IDC_BUTTON_FILL_BUFFER,button,1342242816
Control19=IDC_BUTTON_DATA_SIZE,button,1342242816
Control20=IDC_BUTTON_CHECKSUM,button,1342242816

[DLG:CG_IDD_PROGRESS]
Type=1
Class=?
ControlCount=4
Control1=IDCANCEL,button,1342242817
Control2=CG_IDC_PROGDLG_PROGRESS,msctls_progress32,1350565888
Control3=CG_IDC_PROGDLG_PERCENT,static,1342308352
Control4=CG_IDC_PROGDLG_STATUS,static,1342308352

[CLS:CFillDlg]
Type=0
HeaderFile=FillDlg.h
ImplementationFile=FillDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CFillDlg

[DLG:IDD_FILL_DIALOG]
Type=1
Class=CFillBuffDlg
ControlCount=5
Control1=IDC_RADIO_RANDOM,button,1342308361
Control2=IDC_RADIO_PATTERN,button,1342177289
Control3=IDC_EDIT_PATTERN,edit,1350762625
Control4=IDOK,button,1342373889
Control5=IDCANCEL,button,1342373888

[CLS:CFillBuffDlg]
Type=0
HeaderFile=FillBuffDlg.h
ImplementationFile=FillBuffDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CFillBuffDlg
VirtualFilter=dWC

