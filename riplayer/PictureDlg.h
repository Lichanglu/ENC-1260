#if !defined(AFX_PICTUREDLG_H__EF7D173B_FEB2_4876_A81A_09E1DD37D98B__INCLUDED_)
#define AFX_PICTUREDLG_H__EF7D173B_FEB2_4876_A81A_09E1DD37D98B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PictureDlg.h : header file
//
#include "RiplayerDlg.h"
#include "Picture.h"

/////////////////////////////////////////////////////////////////////////////
// CPictureDlg dialog

class CPictureDlg : public CDialog
{
// Construction
public:
	CPictureDlg(CWnd* pParent = NULL);   // standard constructor
	CPicture m_DlgPic;
	CRect m_rect,m_rtbutton,m_rtclose,m_rtcaption,m_rtcancel,m_rtensure;
	bool m_button,m_close,m_ensure,m_cancel;
	int m_numbtn, m_nInRect;
	CString m_PathFileName;
	CString	m_IniFileName;		//ini文件名
	CString	m_PsPathName;		//拍照文件路径名
	int		m_CurNum;			//当前最近文件数
	CFont m_font;
	CString m_strLanguage[10];

	void InitRect();
	void InitLanguage();
	void ReadMru();
	void WriteMru();
	void EnSure();

// Dialog Data
	//{{AFX_DATA(CPictureDlg)
	enum { IDD = IDD_PICTURE };
	CXPEdit	m_ctredit1;
	CString	m_strPath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPictureDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPictureDlg)
	afx_msg void OnButton1();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICTUREDLG_H__EF7D173B_FEB2_4876_A81A_09E1DD37D98B__INCLUDED_)
