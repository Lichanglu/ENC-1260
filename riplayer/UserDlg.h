#if !defined(AFX_USERDLG_H__557735BD_EDDF_4A97_A310_65DC6A4CF75C__INCLUDED_)
#define AFX_USERDLG_H__557735BD_EDDF_4A97_A310_65DC6A4CF75C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UserDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUserDlg dialog
#include "RiplayerDlg.h"
#include "Picture.h"
class CUserDlg : public CDialog
{
// Construction
public:
	CUserDlg(CWnd* pParent = NULL);   // standard constructor
	CPicture m_DlgPic;
	CRect m_rect,m_rtbutton,m_rtclose,m_rtcaption;
	bool m_button,m_close;
	bool m_bInRect, m_bflag;
	void InitLanguage();
	CString m_strLanguage[10];
	int OnConnect();
	void InitRect();
	bool m_bConnect;
	CFont m_font;
// Dialog Data
	//{{AFX_DATA(CUserDlg)
	enum { IDD = IDD_LOGIN };
	CXPEdit	m_ctrIP;
	CXPEdit	m_ctrPwd;
	CXPEdit	m_ctrUser;
	CString	m_user;
	CString	m_password;
	CString	m_ipaddress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	CBrush m_brush; 
	// Generated message map functions
	//{{AFX_MSG(CUserDlg)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USERDLG_H__557735BD_EDDF_4A97_A310_65DC6A4CF75C__INCLUDED_)
