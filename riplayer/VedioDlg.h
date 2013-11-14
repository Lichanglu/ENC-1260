#if !defined(AFX_VEDIODLG_H__C303B630_E8BE_48C7_825D_AB5C1F08D130__INCLUDED_)
#define AFX_VEDIODLG_H__C303B630_E8BE_48C7_825D_AB5C1F08D130__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VedioDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVedioDlg dialog
#include "RiplayerDlg.h"
#include "Picture.h"
class CVedioDlg : public CDialog
{
// Construction
public:
	CVedioDlg(CWnd* pParent = NULL);   // standard constructor
	CPicture m_DlgPic;
	CRect m_rect,m_rtbutton,m_rtclose,m_rtcaption;
	bool m_button,m_close;
	int m_numbtn;

	void InitRect();
	void InitLanguage();
	void AddString(CString str);
	void SettBox(CString str);
	CString m_strLanguage[10];
	CFont m_font;
// Dialog Data
	//{{AFX_DATA(CVedioDlg)
	enum { IDD = IDD_VEDIO };
	CComboBoxXP	m_ctrCombox1;
	CXPEdit	m_ctrEdit3;
	CXPEdit	m_ctrEdit1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVedioDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVedioDlg)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VEDIODLG_H__C303B630_E8BE_48C7_825D_AB5C1F08D130__INCLUDED_)
