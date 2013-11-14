// RiplayerDlg.h : header file

#include "Picture.h"
#include "Stream\\Stream.h"
#include "Stream\\read_freame.h"
#include "Stream\\mpegts.h"
#include "Stream\\record.h"
#include "SaveAsJpg.h"
#include  "resource.h"//���IDD_XXX δ����
#include "InfoDlg.h"
#include "PlayVedioDlg.h"
#include "MsgBoxDlg.h"
#include "TipDlg.h "

#include "Tools/XPEdit.h"
#include "Tools/MenuXP.h"
#include "Tools/ToolBarXP.h"
#include "Tools/StatusBarXP.h"
#include "Tools/ComboBoxXP.h"

#if !defined(AFX_RIPLAYERDLG_H__4A00236F_98FC_4EF2_83AF_D09F688B11AF__INCLUDED_)
#define AFX_RIPLAYERDLG_H__4A00236F_98FC_4EF2_83AF_D09F688B11AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define MAXNUM 16    
#define MAX_MENU 10   //�ɴ���ʷ��¼���ֵ
#define HISTORY_INDEX   IDR_CLEARMENU - 32 // ��̬�˵�ID
#define   IDC_HAND   MAKEINTRESOURCE(32649) //�궨���������
#define WM_NEW_MSG WM_USER+1   
/////////////////////////////////////////////////////////////////////////////
// CRiplayerDlg dialog
class CRiplayerDlg : public CDialog
{
// Construction
public:
	CRiplayerDlg(CWnd* pParent = NULL);	// standard constructor
    CRect m_rectc;//c�Ի����������
	
	static CRiplayerDlg *m_staiicthis;  //��̬��ָ��;
	static int m_nSampleRate;
	CRect m_rcLastMoveRect;
	CPoint m_point;
	CString m_strInIName;//ini�ļ���
	
	CStdioFile m_LogFile;
    CTipDlg m_dlgTip;
	CInfoDlg m_dlgInfo;	
    CPicture m_DlgPic;
	DEVMODE m_CurrentDM;
	CFont m_hFont;
	CToolTipCtrl    m_Mytip; //��ʾ��Ϣ
	CRiplayerDlg *pdlg;

	~CRiplayerDlg();
	void InitRect();
	void GetConfig();//��ȡ������Ϣ
	void OnMin();
	void OnMax();
//	void MsgBox();
	int OnPlay(CString strURL);  // ���ӱ�����

	void Handler (CString StrMenuName);  //��ʷ��¼
	void Tipmsg(CString str);  //��ʾ�鲥��ַ
	CRect m_rcWind,m_rcview;// ��Ļ��������
	CRect m_rect,m_rc,m_rthtry,m_rthelp,m_rttool,m_rtvedio,m_rtdisplay,m_rtcenterbar1,m_rtcenterbar2,m_rtcenterbar3;
	CRect m_rtleftop,m_rtleftb,m_rtrightop,m_rtrightb,m_rttopbar,m_rtbottombar,m_rtleftbar,m_rtrightbar;

	CRect m_rtplay,m_rtstop,m_rtpicture,m_rtrecord,m_rtvoice,m_rtbk,m_rtball,m_rtmax,m_rtmin,m_rtclose,m_rticon,m_rtvolume;

	bool  m_htry,m_help,m_tool,m_vedio,m_bView;
	bool m_display,m_rectplay;
	bool m_moveball; //�����Ƿ��ƶ� 
	bool m_ballup; //�����Ƿ��ɿ�
	bool m_pause;//�Ƿ���ͣ
	bool m_voice;//�Ƿ���
	bool m_max,m_min,m_close,m_normal;
	bool m_play,m_mute,m_targ,m_stop,m_picture,m_record;
	int m_nCount, m_nInRect; //��껮���������,����ȥ��־
	bool m_bInRect[12];
	int m_nMenuDown; //�˵��Ƿ���
	int m_nMenuCount;
	int m_nScreenCx, m_nScreenCy; // ��Ļ����Ŀ���

	int m_nVol; //����ֵ(0-100)
	CTipDlg *dlgTip;

	bool m_bOpenAudio;	//�򿪹ر�����
	bool m_bFullScreen;  //
	bool m_bMax;

	
	CRect rect[MAXNUM];
	
	CString m_strVolume;        //����ֵ����
	CString m_strURL;
	CString	m_IniFileName;		//��ʷ�鲥ַַ�ļ���
	CString m_strHistroyFile;   
	CString	m_PathName[MAXNUM];	//��ʷ�鲥ַַ�洢��
	int		m_CurNum;			//��ʷ�鲥ַַ����

	CString m_strLangueInfo0,m_strLangueInfo1; 
	CString m_strCaption[MAXNUM];

	CMenu m_rKeyMenu ; //�Ҽ��˵�
    CMenu *m_rKeyMenuSub;//�˵�ָ��
	
	CMenu m_hMruMenu,m_HelpMenu,m_MediaMenu,m_ToolMenu;//��ʷ�˵�
	CMenu m_Menu;//ý��˵�
	CMenu *m_hHistoryMenu, *m_pHelpMenu, *m_pToolMenu, *m_pMediaMenu; //�˵�ָ��

	void SetLang();
	void ReadMru();			//��ȡini�ļ��е�����ļ�
	void WriteMru();		//����ļ�д��ini�ļ�
	void AddMru(CString nPathName);		//�������ļ�
	void ClearMru();		//�������ļ�
	void OnRightMenu();   //�Ҽ��˵�
	void Display ();
	void OnNormal();
	void StopPlay();
	void PausePlay();
// Dialog Data
	//{{AFX_DATA(CRiplayerDlg)
	enum { IDD = IDD_RIPLAYER_DIALOG };
	CStatic	m_staticView;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRiplayerDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
public:
	static void GetVideoCallBack(unsigned char *buff,int buflen,Frame_Info* media_info); //��Ƶ�ص�
	static void GetAudioCallBack(uint8_t *pBuf,int32_t iBufLen, Frame_Info* media_info); //��Ƶ�ص�
	static void GetEncoderCallBack(Encoder_Info* info);
	static void cb_msg_parse(Stream_Protocol protocol,int err);
	SDP_Info *sdp_info;
	
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CRiplayerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnOpen();
	afx_msg void OnSetencode();
	afx_msg void OnInfo();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnAbout();
	afx_msg void OnSetp();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMruClr();
	afx_msg void OnBtnPicture();
	afx_msg void OnSetr();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnExit();
	afx_msg void OnBtnRecord();
	afx_msg void OnBtnPause();
	afx_msg void OnBtnStop();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnFullscreen();
	afx_msg void OnOriginal();
	afx_msg void OnSmall();
	afx_msg void OnLarge();
	afx_msg void OnLicense();
	afx_msg void OnClearmenu();
	afx_msg void OnChinese();
	afx_msg void OnEnglish();
	afx_msg void OnBtnPlay();
	afx_msg void OnStoprecrod();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnOpenfile();
	afx_msg void OnDestroy();

	afx_msg void OnMove(int x, int y);
	//}}AFX_MSG
	afx_msg void OnStaticDblClick();   //˫��static�ؼ���Ӧ��������
	afx_msg void OnStaticClick();   //˫��static�ؼ���Ӧ��������
	afx_msg void MsgBox();
	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RIPLAYERDLG_H__4A00236F_98FC_4EF2_83AF_D09F688B11AF__INCLUDED_)
