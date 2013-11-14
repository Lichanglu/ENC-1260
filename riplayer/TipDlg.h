#if !defined(AFX_TIPDLG_H__44DDC882_C18B_43B2_8699_CE0F6983625E__INCLUDED_)
#define AFX_TIPDLG_H__44DDC882_C18B_43B2_8699_CE0F6983625E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TipDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTipDlg dialog

class CTipDlg : public CDialog
{
// Construction
public:
	CTipDlg(CWnd* pParent = NULL);   // standard constructor
	void OnTip(CString str);	
	HICON m_hIcon;

	COLORREF m_layerColor;	//Ã·–—≤„—’…´
	CString m_tip;
	


// Dialog Data
	//{{AFX_DATA(CTipDlg)
	enum { IDD = IDD_TIPDIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTipDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	// Generated message map functions
	//{{AFX_MSG(CTipDlg)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIPDLG_H__44DDC882_C18B_43B2_8699_CE0F6983625E__INCLUDED_)
