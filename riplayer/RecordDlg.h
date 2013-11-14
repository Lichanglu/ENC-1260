#if !defined(AFX_RECORDDLG_H__2C115B0A_497C_4C56_97BC_977B31DA6A15__INCLUDED_)
#define AFX_RECORDDLG_H__2C115B0A_497C_4C56_97BC_977B31DA6A15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RecordDlg.h : header file
//
#include "RiplayerDlg.h"
#include "Picture.h"
/////////////////////////////////////////////////////////////////////////////
// CRecordDlg dialog

class CRecordDlg : public CDialog
{
// Construction
public:
	CRecordDlg(CWnd* pParent = NULL);   // standard constructor
	CPicture m_DlgPic;
	CRect m_rect,m_rtbutton,m_rtclose,m_rtcaption,m_rtcancel,m_rtensure;
	bool m_button,m_close,m_ensure,m_cancel;
	int m_numbtn,m_nInRect;


	CString m_strLanguage[10];  //语言切换
	CString m_PathFileName;
	CString	m_IniFileName;		//ini文件名
	CString	m_PsPathName;		//拍照文件路径名
	int		m_CurNum;			//当前最近文件数
	CFont m_font;

	void InitRect();
	void ReadMru();
	void WriteMru();
	void EnSure(); 
	void InitLanguage();

// Dialog Data
	//{{AFX_DATA(CRecordDlg)
	enum { IDD = IDD_RECORD };
	CXPEdit	m_ctredit;
	CString	m_edit2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRecordDlg)
	afx_msg void OnBrowse();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECORDDLG_H__2C115B0A_497C_4C56_97BC_977B31DA6A15__INCLUDED_)
