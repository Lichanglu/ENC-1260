#include "UserDlg.h"
#include "VedioDlg.h"
#include "VoiceDlg.h"
#include "StreamDlg.h"
#include "Picture.h"
#include "Stream/Stream.h"
#include "MsgBoxDlg.h"
#if !defined(AFX_SETDLG_H__AA56C60E_664F_4A7C_9105_9119B433C23F__INCLUDED_)
#define AFX_SETDLG_H__AA56C60E_664F_4A7C_9105_9119B433C23F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetDlg dialog

class CSetDlg : public CDialog
{
// Construction
public:
	CSetDlg(CWnd* pParent = NULL);   // standard constructor
	~CSetDlg();
	CImageList m_LargeIcon;
    CUserDlg *user;
	CVedioDlg *vedio;
	CVoiceDlg *voice;
	CStreamDlg *stream;
	OutputVideoInfo val; // 视频参数
	AudioParam      audio; //音频参数

	CFont m_font;
	CPicture m_DlgPic;
	CRect m_rect,m_rtensure,m_rtcancal,m_rtapp,m_rtcaption,m_rtclose;
	bool m_button,m_close,m_cancal,m_app,m_ensure;
	int m_numbtn, m_nInRect;
	int m_dlgFlag;  //切换界面
    void InitRect();
	void OnEnsure();

	bool m_user;
	bool m_bensure,	m_bclose, m_bcancel,m_bIncancel, m_bInclose, m_bInensure;
// Dialog Data
	//{{AFX_DATA(CSetDlg)
	enum { IDD = IDD_SET };
	CListCtrl	m_ListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void OnUser();
	void OnVedio();
	void OnVoice();
	void OnStream();
	void InitLanguage();

	CString m_strLanguage[10];
	// Generated message map functions
	//{{AFX_MSG(CSetDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClickList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETDLG_H__AA56C60E_664F_4A7C_9105_9119B433C23F__INCLUDED_)
