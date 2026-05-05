#if !defined(AFX_FILLBUFFDLG_H__72CE0563_2E9B_11D4_8E5B_00105A92A724__INCLUDED_)
#define AFX_FILLBUFFDLG_H__72CE0563_2E9B_11D4_8E5B_00105A92A724__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FillBuffDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFillBuffDlg dialog

class CFillBuffDlg : public CDialog
{
// Construction
public:
	CFillBuffDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFillBuffDlg)
	enum { IDD = IDD_FILL_DIALOG };
	int		m_Index;
	CString	m_csPattern;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFillBuffDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFillBuffDlg)
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILLBUFFDLG_H__72CE0563_2E9B_11D4_8E5B_00105A92A724__INCLUDED_)
