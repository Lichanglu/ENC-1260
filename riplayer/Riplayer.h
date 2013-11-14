// Riplayer.h : main header file for the RIPLAYER application
//

#if !defined(AFX_RIPLAYER_H__85972539_AA0A_410B_B55D_F9FBB4AC051C__INCLUDED_)
#define AFX_RIPLAYER_H__85972539_AA0A_410B_B55D_F9FBB4AC051C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CRiplayerApp:
// See Riplayer.cpp for the implementation of this class
//
extern HANDLE m_handle; 
class CRiplayerApp : public CWinApp
{
public:
	CRiplayerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRiplayerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL
	
// Implementation

	//{{AFX_MSG(CRiplayerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RIPLAYER_H__85972539_AA0A_410B_B55D_F9FBB4AC051C__INCLUDED_)
