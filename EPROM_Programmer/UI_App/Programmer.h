// Programmer.h : main header file for the PROGRAMMER application
//

#if !defined(AFX_PROGRAMMER_H__367F20E6_D436_11D3_8E5B_00105A92A724__INCLUDED_)
#define AFX_PROGRAMMER_H__367F20E6_D436_11D3_8E5B_00105A92A724__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CProgrApp:
// See Programmer.cpp for the implementation of this class
//

class CProgrApp : public CWinApp
{
public:
	CProgrApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgrApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CProgrApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRAMMER_H__367F20E6_D436_11D3_8E5B_00105A92A724__INCLUDED_)
