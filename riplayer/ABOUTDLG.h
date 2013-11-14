#if !defined(AFX_ABOUTDLG_H__CA8C5AF0_4298_4261_A991_081401371D41__INCLUDED_)
#define AFX_ABOUTDLG_H__CA8C5AF0_4298_4261_A991_081401371D41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ABOUTDLG.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CABOUTDLG dialog

class CABOUTDLG : public CDialog
{
// Construction
public:
	CABOUTDLG(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CABOUTDLG)
	enum { IDD = IDD_ABOUTDLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CABOUTDLG)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CABOUTDLG)
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ABOUTDLG_H__CA8C5AF0_4298_4261_A991_081401371D41__INCLUDED_)
