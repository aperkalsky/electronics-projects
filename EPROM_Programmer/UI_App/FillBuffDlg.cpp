// FillBuffDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Programmer.h"
#include "FillBuffDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFillBuffDlg dialog


CFillBuffDlg::CFillBuffDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFillBuffDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFillBuffDlg)
	m_Index = 0;
	m_csPattern = _T("FF");
	//}}AFX_DATA_INIT
}


void CFillBuffDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFillBuffDlg)
	DDX_Radio(pDX, IDC_RADIO_RANDOM, m_Index);
	DDX_Text(pDX, IDC_EDIT_PATTERN, m_csPattern);
	DDV_MaxChars(pDX, m_csPattern, 2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFillBuffDlg, CDialog)
	//{{AFX_MSG_MAP(CFillBuffDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFillBuffDlg message handlers

void CFillBuffDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CFillBuffDlg::OnOK() 
{
	UpdateData();
	
	CDialog::OnOK();
}
