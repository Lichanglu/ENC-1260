#if !defined(AFX_INFODLG_H__5751C458_5125_46F0_B7FA_4B3C08648477__INCLUDED_)
#define AFX_INFODLG_H__5751C458_5125_46F0_B7FA_4B3C08648477__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InfoDlg.h : header file
//
#include "Picture.h"
//#include "RiplayerDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CInfoDlg dialog
class CInfoDlg : public CDialog
{
// Construction
public:
	CInfoDlg(CWnd* pParent = NULL);   // standard constructor
	~CInfoDlg();
	CPicture m_DlgPic;
	CRect m_rect,m_rtbutton,m_rtclose,m_rtcaption;
	CRect m_rctext[10],m_rcaption[10];
	bool m_close,m_bflag;

	CString m_strText[10], m_strInfo[10];
	CString m_strCaption;

	bool m_bInRect;

	void InitRect();
	void InitLanguage();
// Dialog Data
	//{{AFX_DATA(CInfoDlg)
	enum { IDD = IDD_INFO };
	CListCtrl	m_List1;
	CString	m_strVedio,m_strRect;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInfoDlg)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFODLG_H__5751C458_5125_46F0_B7FA_4B3C08648477__INCLUDED_)
