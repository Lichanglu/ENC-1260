#if !defined(AFX_MSGBOXDLG_H__DC0727F2_898D_462A_A634_41ED58BF55B3__INCLUDED_)
#define AFX_MSGBOXDLG_H__DC0727F2_898D_462A_A634_41ED58BF55B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MsgBoxDlg.h : header file
//
#include "Picture.h"
/////////////////////////////////////////////////////////////////////////////
// CMsgBoxDlg dialog

class CMsgBoxDlg : public CDialog
{
// Construction
public:
	CMsgBoxDlg(CWnd* pParent = NULL);   // standard constructor
	CPicture m_DlgPic;
	CRect m_rect,m_rtbutton,m_rtclose,m_rtcaption,m_rtcancel,m_rtmsg;
	bool m_button,m_close,m_bCancel;
	int m_numbtn;
	bool m_bInRect;
	CString m_strText[5];
	CFont m_font;
	CString m_StrMsg;
	void SetMsg(CString caption, CString str);
	void InitRect();
	void InitLanguage();

// Dialog Data
	//{{AFX_DATA(CMsgBoxDlg)
	enum { IDD = IDD_MESSAGEBOXDLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgBoxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMsgBoxDlg)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGBOXDLG_H__DC0727F2_898D_462A_A634_41ED58BF55B3__INCLUDED_)
