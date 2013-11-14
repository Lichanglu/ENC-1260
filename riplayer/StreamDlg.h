#if !defined(AFX_STREAMDLG_H__52667121_FBBF_4000_AC28_A89B6A055D27__INCLUDED_)
#define AFX_STREAMDLG_H__52667121_FBBF_4000_AC28_A89B6A055D27__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StreamDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStreamDlg dialog
#include "Picture.h"
class CStreamDlg : public CDialog
{
// Construction
public:
	CStreamDlg(CWnd* pParent = NULL);   // standard constructor
		CPicture m_DlgPic;
	CRect m_rect,m_rtbutton,m_rtbutton1,m_rtbutton2,m_rtbutton3,m_rtbutton4,m_rtbutton5,m_rtbutton6,m_rtclose,m_rtcaption;
	bool m_button,m_close,m_button1,m_button2,m_button3,m_button4,m_button5,m_button6;
	int m_numbtn;

	void InitRect();
	void InitLanguage();
	CString m_strLanguage[20];
// Dialog Data
	//{{AFX_DATA(CStreamDlg)
	enum { IDD = IDD_STREAM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStreamDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStreamDlg)
	afx_msg void OnTsart();
	afx_msg void OnButton4();
	afx_msg void OnButton5();
	afx_msg void OnButton6();
	afx_msg void OnButton7();
	afx_msg void OnButton8();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnFieldchangedIpaddress1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STREAMDLG_H__52667121_FBBF_4000_AC28_A89B6A055D27__INCLUDED_)
