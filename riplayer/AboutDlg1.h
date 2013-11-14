#if !defined(AFX_ABOUTDLG1_H__FFE665A8_CD19_4113_9322_BE0CCB892D1C__INCLUDED_)
#define AFX_ABOUTDLG1_H__FFE665A8_CD19_4113_9322_BE0CCB892D1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AboutDlg1.h : header file
//
#include "Picture.h"
/////////////////////////////////////////////////////////////////////////////
// AboutDlg dialog

class AboutDlg : public CDialog
{
// Construction
public:
	AboutDlg(CWnd* pParent = NULL);   // standard constructor

	CPicture m_DlgPic;
	CRect m_rect,m_rtbutton,m_rtclose,m_rtcaption;
	bool m_button,m_close;
	int m_numbtn;
	bool m_bInRect;
	CFont m_font;

	void InitRect();
// Dialog Data
	//{{AFX_DATA(AboutDlg)
	enum { IDD = IDD_ABOUTDLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

   
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AboutDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ABOUTDLG1_H__FFE665A8_CD19_4113_9322_BE0CCB892D1C__INCLUDED_)
