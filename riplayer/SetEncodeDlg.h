#if !defined(AFX_SETENCODEDLG_H__E1C9E1E5_2B05_4C79_BE3A_8A653BF45618__INCLUDED_)
#define AFX_SETENCODEDLG_H__E1C9E1E5_2B05_4C79_BE3A_8A653BF45618__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetEncodeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetEncodeDlg dialog

class CSetEncodeDlg : public CDialog
{
// Construction
public:
	CSetEncodeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetEncodeDlg)
	enum { IDD = IDD_SETENCODE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetEncodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetEncodeDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETENCODEDLG_H__E1C9E1E5_2B05_4C79_BE3A_8A653BF45618__INCLUDED_)
