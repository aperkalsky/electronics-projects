// ProgrammerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Programmer.h"
#include "ProgrammerDlg.h"
#include "FillBuffDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgrammerDlg dialog

CProgrammerDlg::CProgrammerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProgrammerDlg::IDD, pParent),
	m_Buff(0x10000)	// 64K buffer
{
	//{{AFX_DATA_INIT(CProgrammerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CProgrammerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgrammerDlg)
	DDX_Control(pDX, IDC_STATIC_STATUS_LINE, m_csStatusLine);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_STATIC_OF_TXT, m_csOutputFilename);
	DDX_Control(pDX, IDC_STATIC_IF_TXT, m_csInputFilename);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CProgrammerDlg, CDialog)
	//{{AFX_MSG_MAP(CProgrammerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_BURN, OnBurnEeprom)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_BLANK, OnCheckBlankState)
	ON_BN_CLICKED(IDC_BUTTON_CHECKSUM, OnCalculateChecksum)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_BUFFER, OnClearBuffer)
	ON_BN_CLICKED(IDC_BUTTON_FILL_BUFFER, OnFillBuffer)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_BUFFER, OnLoadBuffer)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnReadEeprom)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_BUFFER, OnSaveBuffer)
	ON_BN_CLICKED(IDC_BUTTON_VERIFY, OnVerifyEeprom)
	ON_BN_CLICKED(IDC_BUTTON_DATA_SIZE, OnDataSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgrammerDlg message handlers

BOOL CProgrammerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	// set progress bar control range
//	m_Progress.SetRange(0,5);	// 6 steps
//	m_Progress.SetStep(1);

	// check if the interface to the programmer is valid
	if(!m_Prog.IsReady())
	{
		AfxMessageBox("Unable to load driver PROG_LPT.VXD");

		//TODO: grey out programmer controls
	}
	else	// handle to driver was opened
	{
		if(!m_Prog.PrSelfTest())
		{
			AfxMessageBox("Programmer self test failed. Make sure that the programmer is attached.");

			//TODO: grey out programmer controls
		}
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CProgrammerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CProgrammerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CProgrammerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CProgrammerDlg::OnBurnEeprom() 
{
	BURN_STRUCT m_BS;	
	CString csTxtBuff;
	BOOL bResult = TRUE;
	WORD wAddr;

	// TODO : warn user before start programming

	// check that buffer is not empty before we start
	if(m_Buff.GetDataSize() == 0)
	{
		csTxtBuff.Format("Memory buffer is empty, nothing to program");
		m_csStatusLine.SetWindowText(csTxtBuff);
		return;
	}

	// set progress bar control range
	m_Progress.SetRange(0,257);	// 258 steps
	m_Progress.SetStep(1);

	// display wait cursor
	CWaitCursor cWait;

	// notify user to be patient
	csTxtBuff.Format("Programming EPROM ...");
	m_csStatusLine.SetWindowText(csTxtBuff);

	// read EPROM byte by byte
	for(DWORD wCurrAddr = 0; wCurrAddr <= MAX_EPROM_ADDR; wCurrAddr++)
	{
		// set address
		wAddr = LOWORD(wCurrAddr);

		// advance progress bar
		if(wCurrAddr % 0xFF == 0)
			m_Progress.StepIt();

		if(!m_Prog.PrSetAddress(wAddr))
		{
			bResult = FALSE;
			break;
		}

		// set data to burn
		if(!m_Prog.PrSetData(m_Buff.GetMember(wAddr)))
		{
			bResult = FALSE;
			break;
		}

		// clear number of retries
		m_BS.bNumRetries = 0;

		// burn data
		if(m_Prog.PrBurn(&m_BS) && m_BS.bSuccess)	// burn succeeded
			continue;
		else
		{
			bResult = FALSE;
			break;
		}
	}

	// reset progress bar
	m_Progress.SetPos(0);

	if(bResult)
		csTxtBuff.Format("EPROM was programmed");
	else
	{
		// check the failure reason
		if(m_BS.bNumRetries == 0)	// this is IOCTL failure, since it haven't try to burn yet
			csTxtBuff.Format("Failed programming EPROM");
		else
			csTxtBuff.Format("Burn failure at %04Xh : wrote %02Xh, got %02Xh after %d attempts",
				wAddr, m_BS.bDataToBurn, m_BS.bActualResult, m_BS.bNumRetries);
	}

	// display results
	m_csStatusLine.SetWindowText(csTxtBuff);
}

/*
	Function name: OnCheckBlankState()

	Description:
		This function verifies that the EEPROM is blank by checking that it have 0xFF
		value in each address

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void CProgrammerDlg::OnCheckBlankState() 
{
	CString csTxtBuff;

	// display wait cursor
	CWaitCursor cWait;

	// notify user to be patient
	csTxtBuff.Format("Checking blank state ...");
	m_csStatusLine.SetWindowText(csTxtBuff);

	if(m_Prog.PrCheckBlankState(m_Progress))
		csTxtBuff.Format("EPROM is blank");
	else
		csTxtBuff.Format("EPROM isn't blank");

	// display results
	m_csStatusLine.SetWindowText(csTxtBuff);
}

void CProgrammerDlg::OnCalculateChecksum() 
{
	CString csTxtBuff;
	WORD wCs = 0;

	// check that buffer is not empty
	if(m_Buff.GetDataSize() == 0)
	{
		csTxtBuff.Format("Memory buffer is empty, unable to calculate checksum");
		m_csStatusLine.SetWindowText(csTxtBuff);
		return;
	}

	// perform a calculation
	for(DWORD wCurrAddr = 0; wCurrAddr <= MAX_EPROM_ADDR; wCurrAddr++)
		wCs += m_Buff.GetMember(LOWORD(wCurrAddr));

	// display results
	csTxtBuff.Format("EPROM checksum is %04Xh", wCs);
	m_csStatusLine.SetWindowText(csTxtBuff);
}

void CProgrammerDlg::OnClearBuffer() 
{
	CString csTxtBuff;

	// clear contents of memory buffer
	m_Buff.ClearBuffer();

	// display status
	csTxtBuff.Format("Memory buffer was cleared");
	m_csStatusLine.SetWindowText(csTxtBuff);
}

void CProgrammerDlg::OnFillBuffer() 
{
	CFillBuffDlg Dlg;
	BYTE bPattern;
	LPTSTR pString;

	// Open a fill dialog window
	if(Dlg.DoModal() == IDOK)
	{
		// user really want to fill buffer with some pattern
		// check if this is a request for random fill or pattern fill
		if(Dlg.m_Index == 0)
			m_Buff.RandomFill();
		else
		{
			// read pattern value
			pString = Dlg.m_csPattern.GetBuffer(0);
			sscanf(pString, "%x", &bPattern);

			// fill buffer
			m_Buff.FillBuffer(bPattern);
		}
	}
}

void CProgrammerDlg::OnLoadBuffer() 
{
	BYTE bRet;
	CString csTxtBuff;

	// define a filename filter
	static char m_Filter[] = "Intel hex files (*.hex)|*.hex||";

	// create a dialog
	CFileDialog m_OpenDlg(TRUE, _T(".hex"), _T("default.hex"), NULL, m_Filter, this);

	// display it
	if(m_OpenDlg.DoModal() == IDOK)
	{
		// retrieve the filename and display it in the dialog
		m_csInFullPath = m_OpenDlg.GetPathName();
		m_csInputFilename.SetWindowText(m_OpenDlg.GetFileName());

		// attempt to load file
		bRet = m_Buff.ReadHexFile(m_csInFullPath);

		// update status
		switch(bRet)
		{
		case BUFF_OK:
			csTxtBuff.Format("File %s was loaded into memory buffer", m_OpenDlg.GetFileName());
			break;

		case BUFF_NO_FILE:
			csTxtBuff.Format("File %s was not found", m_OpenDlg.GetFileName());
			break;

		default:
			csTxtBuff.Format("Data error in file %s. Buffer was not loaded", m_OpenDlg.GetFileName());
			break;
		}

		// display load status
		m_csStatusLine.SetWindowText(csTxtBuff);
	}
}

void CProgrammerDlg::OnReadEeprom() 
{
	CString csTxtBuff;
	BOOL bResult = TRUE;
	BYTE bDataRead;
	WORD wAddr;

	// set progress bar control range
	m_Progress.SetRange(0,257);	// 258 steps
	m_Progress.SetStep(1);

	// display wait cursor
	CWaitCursor cWait;

	// notify user to be patient
	csTxtBuff.Format("Reading EPROM ...");
	m_csStatusLine.SetWindowText(csTxtBuff);

	// clear memory buffer
	m_Buff.ClearBuffer();

	// read EPROM byte by byte
	for(DWORD wCurrAddr = 0; wCurrAddr <= MAX_EPROM_ADDR; wCurrAddr++)
	{
		// set address
		wAddr = LOWORD(wCurrAddr);

		if(!m_Prog.PrSetAddress(wAddr))
		{
			bResult = FALSE;
			break;
		}

		// read data
		if(!m_Prog.PrReadData(&bDataRead))
		{
			bResult = FALSE;
			break;
		}

		// store it in memory buffer
		m_Buff.SetMember(wAddr, bDataRead);

		// advance progress bar
		if(wCurrAddr % 0xFF == 0)
			m_Progress.StepIt();
	}

	// reset progress bar
	m_Progress.SetPos(0);

	if(bResult)
		csTxtBuff.Format("EPROM was read");
	else
		csTxtBuff.Format("EPROM was not read");

	// display results
	m_csStatusLine.SetWindowText(csTxtBuff);
}

void CProgrammerDlg::OnSaveBuffer() 
{
	BYTE bRet;
	CString csTxtBuff;

	// define a filename filter
	static char m_Filter[] = "Intel hex files (*.hex)|*.hex||";

	// create a dialog
	CFileDialog m_SaveDlg(FALSE, _T(".hex"), _T("default.hex"), NULL, m_Filter, this);

	// display it
	if(m_SaveDlg.DoModal() == IDOK)
	{
		// retrieve the filename and display it in the dialog
		m_csOutFullPath = m_SaveDlg.GetPathName();
		m_csOutputFilename.SetWindowText(m_SaveDlg.GetFileName());

		// attempt to save buffer
		bRet = m_Buff.SaveHexFile(m_csOutFullPath, FALSE, TRUE);

		// update status
		switch(bRet)
		{
		case BUFF_OK:
			csTxtBuff.Format("Memory buffer was saved to file %s", m_SaveDlg.GetFileName());
			break;

		case SL_EMPTY:
			csTxtBuff.Format("Buffer is empty, no data was saved");
			break;

		default:
			csTxtBuff.Format("Unable to create file %s", m_SaveDlg.GetFileName());
			break;
		}

		// display load status
		m_csStatusLine.SetWindowText(csTxtBuff);
	}
}

/*
	Function name: OnVerifyEeprom()

	Description:
		This function compares the contents of EPROM to the contents of memory buffer

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void CProgrammerDlg::OnVerifyEeprom() 
{
	CString csTxtBuff;
	BOOL bResult = TRUE;
	BYTE bDataRead;
	WORD wAddr;
	BOOL bPassed = TRUE;

	// set progress bar control range
	m_Progress.SetRange(0,257);	// 258 steps
	m_Progress.SetStep(1);

	// check that buffer is not empty before we start
	if(m_Buff.GetDataSize() == 0)
	{
		csTxtBuff.Format("Memory buffer is empty, unable to verify.");
		m_csStatusLine.SetWindowText(csTxtBuff);
		return;
	}

	// display wait cursor
	CWaitCursor cWait;

	// notify user to be patient
	csTxtBuff.Format("Comparing EPROM contents to memory buffer...");
	m_csStatusLine.SetWindowText(csTxtBuff);

	// read EPROM byte by byte and compare to the mem. buffer
	for(DWORD wCurrAddr = 0; wCurrAddr <= MAX_EPROM_ADDR; wCurrAddr++)
	{
		// set address
		wAddr = LOWORD(wCurrAddr);

		if(!m_Prog.PrSetAddress(wAddr))
		{
			bResult = FALSE;
			break;
		}

		// for testing
//		Sleep(10);

		// read data
		if(!m_Prog.PrReadData(&bDataRead))
		{
			bResult = FALSE;
			break;
		}

		// compare it
		if(m_Buff.GetMember(wAddr) != bDataRead)
		{
			bPassed = FALSE;
			break;
		}

		// advance progress bar
		if(wCurrAddr % 0xFF == 0)
			m_Progress.StepIt();
	}

	// reset progress bar
	m_Progress.SetPos(0);

	if(bResult)
	{
		// check verify status
		if(bPassed)
			csTxtBuff.Format("The contents of EPROM and memory buffer are equal");
		else
			csTxtBuff.Format("Error at %04Xh, EPROM = %02Xh Buffer = %02Xh", wAddr, bDataRead, m_Buff.GetMember(wAddr));
	}
	else
		csTxtBuff.Format("Failed reading EPROM");

	// display results
	m_csStatusLine.SetWindowText(csTxtBuff);
}

void CProgrammerDlg::OnCancel() 
{
	//
	
	CDialog::OnCancel();
}

void CProgrammerDlg::OnOK() 
{
	//

	CDialog::OnOK();
}

/*
	Function name: ()

	Description:
		This function 

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void CProgrammerDlg::GreyOutProgControls()
{

}

/*
	Function name: OnDataSize()

	Description:
		This function prints the actual size of data in buffer (the highest address before 0xFF)

	Input:
		None

	Output:
		None

	Return Value:
		None
*/
void CProgrammerDlg::OnDataSize() 
{
	CString csTxtBuff;

	csTxtBuff.Format("Data size = %d", m_Buff.GetDataSize());
	
	// display results
	m_csStatusLine.SetWindowText(csTxtBuff);
}
