// ProgrammerDlg.h : header file
//

#if !defined(AFX_PROGRAMMERDLG_H__367F20E8_D436_11D3_8E5B_00105A92A724__INCLUDED_)
#define AFX_PROGRAMMERDLG_H__367F20E8_D436_11D3_8E5B_00105A92A724__INCLUDED_

#include "HexBuff.h"	// Added by ClassView
#include "ProgInterface.h"	// Added by ClassView
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CProgrammerDlg dialog

class CProgrammerDlg : public CDialog
{
// Construction
public:
	CProgrammerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CProgrammerDlg)
	enum { IDD = IDD_PROGRAMMER_DIALOG };
	CStatic	m_csStatusLine;
	CProgressCtrl	m_Progress;
	CStatic	m_csOutputFilename;
	CStatic	m_csInputFilename;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgrammerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CProgrammerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBurnEeprom();
	afx_msg void OnCheckBlankState();
	afx_msg void OnCalculateChecksum();
	afx_msg void OnClearBuffer();
	afx_msg void OnFillBuffer();
	afx_msg void OnLoadBuffer();
	afx_msg void OnReadEeprom();
	afx_msg void OnSaveBuffer();
	afx_msg void OnVerifyEeprom();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnDataSize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void GreyOutProgControls();
	CString m_csOutFullPath;
	CString m_csInFullPath;
	ProgInterface m_Prog;
	HexBuff m_Buff;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRAMMERDLG_H__367F20E8_D436_11D3_8E5B_00105A92A724__INCLUDED_)
