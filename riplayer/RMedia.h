// RMedia.h : main header file for the RMEDIA DLL
//

#if !defined(AFX_RMEDIA_H__9D4D7619_5354_4282_8948_D4BD07F67B47__INCLUDED_)
#define AFX_RMEDIA_H__9D4D7619_5354_4282_8948_D4BD07F67B47__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CRMediaApp
// See RMedia.cpp for the implementation of this class
//

class CRMediaApp : public CWinApp
{
public:
	CRMediaApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRMediaApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CRMediaApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern "C"  _declspec (dllexport) HANDLE Init();
extern "C"  _declspec (dllexport) bool SetShowRect(HANDLE handle, HWND hwnd, RECT rcVideo);
extern "C"  _declspec (dllexport) bool SetVideoData(HANDLE handle, BYTE* pData, int nLen, int nWidth, int nHeight);
extern "C"  _declspec (dllexport) bool SetAudioData(HANDLE handle, BYTE *pData, int nLen, int nSample);
extern "C"  _declspec (dllexport) const BYTE* GetYUVData(HANDLE handle, int *pLen, int *pnWidth, int *pnHeight);
extern "C"  _declspec (dllexport) bool SetMute(HANDLE handle, bool bMute);
extern "C"  _declspec (dllexport) bool SetScale(HANDLE handle, bool bScale);
extern "C"  _declspec (dllexport) bool SetVolume(HANDLE handle, int nVol);//nvol range form 0 to 100
extern "C"  _declspec (dllexport) int  GetVolume(HANDLE handle);
extern "C"  _declspec (dllexport) bool UnInit(HANDLE handle);
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RMEDIA_H__9D4D7619_5354_4282_8948_D4BD07F67B47__INCLUDED_)
