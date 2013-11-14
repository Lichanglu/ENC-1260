#if !defined(AFX_VOICEDLG_H__EA5865DC_5DF4_4B9F_A425_46AA23E2CFAE__INCLUDED_)
#define AFX_VOICEDLG_H__EA5865DC_5DF4_4B9F_A425_46AA23E2CFAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VoiceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVoiceDlg dialog
#include "RiplayerDlg.h"
#include "Picture.h"
class CVoiceDlg : public CDialog
{
// Construction
public:
	CVoiceDlg(CWnd* pParent = NULL);   // standard constructor
	CPicture m_DlgPic;
	CRect m_rect,m_rtbutton,m_rtclose,m_rtcaption;
	bool m_button,m_close;
	int m_numbtn;

	void InitRect();
	void InitLanguage();
	void AddString(CString str);
    void AddCombox(CString str);
	void SettBox(CString str);
	void setcombox(CString str);
	void SetView(CString str);
	void setLVolume(CString str);
	void setRVolume(CString str);
	CString m_strLanguage[10];
	CFont m_font;
// Dialog Data
	//{{AFX_DATA(CVoiceDlg)
	enum { IDD = IDD_VOICE };
	CComboBoxXP	m_ctrCombox5;
	CComboBoxXP	m_ctrCombox4;
	CComboBoxXP	m_ctrCombox3;
	CComboBoxXP	m_ctrCombox2;
	CComboBoxXP	m_ctrCombox1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVoiceDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVoiceDlg)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VOICEDLG_H__EA5865DC_5DF4_4B9F_A425_46AA23E2CFAE__INCLUDED_)
