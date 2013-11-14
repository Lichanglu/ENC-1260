#if !defined(AFX_PLAYVEDIODLG_H__1C8B7A5A_B768_446D_8429_0E90E3D43C7C__INCLUDED_)
#define AFX_PLAYVEDIODLG_H__1C8B7A5A_B768_446D_8429_0E90E3D43C7C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlayVedioDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPlayVedioDlg dialog
#include "RiplayerDlg.h"
#include "Picture.h"
class CPlayVedioDlg : public CDialog
{
// Construction
public:
	CPlayVedioDlg(CWnd* pParent = NULL);   // standard constructor
    void OpenMruFile(int nMru);
    void InitRect();
	void InitLanguage();
	CFont m_font;
	CString m_strLanguage[10];
	CPicture m_DlgPic;
	CRect m_rect,m_rtbutton,m_rtbutton1,m_rtclose,m_rtcaption,m_rtText;
	bool m_button,m_close,m_button1,m_cancel;
	int  m_nInRect;
	int m_numbtn;
// Dialog Data
	//{{AFX_DATA(CPlayVedioDlg)
	enum { IDD = IDD_PLAYVEDIO };
	CXPEdit	m_editUrl;
	CString	m_edit1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayVedioDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPlayVedioDlg)
	afx_msg void OnPlay();
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnChangeUrl();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYVEDIODLG_H__1C8B7A5A_B768_446D_8429_0E90E3D43C7C__INCLUDED_)
