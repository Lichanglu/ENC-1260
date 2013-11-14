// UserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Riplayer.h"
#include "UserDlg.h"
#include "Stream/Stream.h"
#include "MsgBoxDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUserDlg dialog


CUserDlg::CUserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUserDlg)
	m_user = _T("");
	m_password = _T("");
	m_ipaddress = _T("");
	m_bConnect= FALSE;
	m_bflag = false;
	m_bInRect = false;
	//}}AFX_DATA_INIT
}


void CUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUserDlg)
	DDX_Control(pDX, IDC_EDIT5, m_ctrPwd);
	DDX_Control(pDX, IDC_EDIT1, m_ctrUser);
	DDX_Text(pDX, IDC_EDIT1, m_user);
	DDX_Text(pDX, IDC_EDIT5, m_password);
	DDX_Text(pDX, IDC_IPADDRESS, m_ipaddress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUserDlg, CDialog)
	//{{AFX_MSG_MAP(CUserDlg)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUserDlg message handlers
BOOL CUserDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	InitLanguage();
	GetClientRect(&m_rect);
	InitRect();
	// TODO: Add extra initialization here
	m_brush.CreateSolidBrush(RGB(239, 255, 255));

	SetDlgItemText(IDC_IPADDRESS, g_strIpAddress);
	SetDlgItemText(IDC_EDIT1, g_strUserName);
	SetDlgItemText(IDC_EDIT5, g_strPassword);
	/****** 字体设置 ***********/
	if (g_Language)
	{
		m_font.CreatePointFont(89, "Verdana");
	} 
	else
	{
		m_font.CreatePointFont(100,"宋体");
	}
	SetFont(&m_font); 
	CWnd   *pw   =   GetWindow(GW_CHILD); 
	while(pw   !=   NULL) 
	{ 
		pw-> SetFont(&m_font); 
		pw   =   pw-> GetWindow(GW_HWNDNEXT); 
	};
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUserDlg::InitLanguage()
{

	m_strLanguage[0] = GetStringLanguage("Login","Static1","用户名:");
	m_strLanguage[1] = GetStringLanguage("Login","Static2","  密码:");
	m_strLanguage[2] = GetStringLanguage("Login","Static3","IP地址:");

	SetDlgItemText(IDC_STATIC1,m_strLanguage[0]);
	SetDlgItemText(IDC_STATIC2,m_strLanguage[1]);
	SetDlgItemText(IDC_STATIC3,m_strLanguage[2]);
}


void CUserDlg::InitRect()
{
	m_rtbutton.SetRect(m_rect.Width()-146,m_rect.bottom - 67,m_rect.Width()-77,m_rect.bottom - 45);	
}

int  CUserDlg::OnConnect()
{
	UpdateData(true);
	CString strIp;
	OutputVideoInfo VideoInfo;
	int ret = 0;
	AudioParam      AudioInfo;
	char user[64]={0};
	char password[64]={0};
	char ipaddress[64] = {0};

	GetDlgItemText(IDC_IPADDRESS,strIp);

	sprintf(ipaddress,strIp);
	sprintf(user,m_user);
	sprintf(password,m_password);
	
	client_socket = sdkTcpConnect(ipaddress);
	if (client_socket == -1)
	{
		CMsgBoxDlg dlg;
		dlg.SetMsg(GetStringLanguage("MsgBox", "Caption1", "警告"), GetStringLanguage("MsgBox", "Tip1", "无法连接编码器!"));
		dlg.DoModal();
		return -1;
	}
	if( client_socket > 0 )
	{
		ret = sdkSetLogin(client_socket,user,password);
		if (ret == SERVER_RET_SOCK_MAX_NUM)
		{
			CMsgBoxDlg dlg;
			dlg.SetMsg(GetStringLanguage("MsgBox", "Caption1", "警告"), GetStringLanguage("MsgBox", "Tip1", "无法连接编码器!"));
			dlg.DoModal();
			return -1;
		}
		else if (ret  != 0 )
		{
			CMsgBoxDlg dlg;
			dlg.SetMsg(GetStringLanguage("MsgBox", "Caption1", "警告"), GetStringLanguage("MsgBox", "Tip2", "用户名或密码错误"));
			dlg.DoModal();
			return -1;
		}
		else
		{
			CMsgBoxDlg dlg;
			dlg.SetMsg(GetStringLanguage("MsgBox", "Caption2", "提示"), GetStringLanguage("MsgBox", "Tip3", "成功连接上编码器!"));
			dlg.DoModal();
		}
		m_bConnect = TRUE;
		g_strIpAddress = strIp;
		g_strPassword = m_password;
		g_strUserName = m_user;
		ret = sdkGetVideoparam(client_socket,&VideoInfo);
		if(ret != 0)
		{
			TRACE("ERROR,the error id =0x%x\n",ret);
			return -1;
		}


		ret = sdkGetAudioParam(client_socket,&AudioInfo);
		if(ret != 0)
		{
			TRACE("ERROR,the error id =0x%x\n",ret);
			return -1;
		}
	
	}
	return 0;
}

void CUserDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtbutton.PtInRect(point))
	{
		m_button=true;

	}
	if(m_rtclose.PtInRect(point))
	{
		m_close=true;

	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CUserDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtbutton.PtInRect(point))
	{
		m_bflag = true;
		m_bInRect = true;
		
	}
	else
	{
		m_bflag = false;
		
	}
	if (m_bflag)//进入矩形区域，刷新
	{
		InvalidateRect(&m_rtbutton);
	}
	else
	{
		if (m_bInRect)//进入矩形区域内后离开，刷新
		{
			InvalidateRect(&m_rtbutton);
			m_bInRect = false;
		}
		
	}


	if(m_button)
	{
		if(!m_rtbutton.PtInRect(point))
		{
			m_button=false;
		}
	}


	CDialog::OnMouseMove(nFlags, point);
}

void CUserDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default


	if(m_rtbutton.PtInRect(point)&&m_button)
	{
	
		if (!m_bConnect)
		{		
			OnConnect();			 
		}
		else
		{
			m_bConnect = false;
			sdkCloseTCPConnet(client_socket);

		}
		InvalidateRect(m_rtbutton);

	}
	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CUserDlg::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	GetWindowRect(&rect);
	GetClientRect(&m_rect);
	CDC memdc;     //定义一个显示设备对象 
	CBitmap bmp;       //定义一个位图对象 
	memdc.CreateCompatibleDC(pDC);     //建立与屏幕显示兼容的内存显示设备 
	bmp.CreateCompatibleBitmap(pDC,rect.Width(),rect.Height());//建立一个与屏幕显示兼容的位图
	memdc.SetBkMode(TRANSPARENT);
	CBrush bru(RGB(239,255,255));
	CRect cr(0,0,rect.Width(),rect.Height());
	CBitmap* pOldBmp = memdc.SelectObject(&bmp);            //将位图选入到内存显示设备中		
	
	DWORD style = DT_CENTER|DT_SINGLELINE|DT_VCENTER;
	memdc.FillRect(&cr,&bru); 	//用背景色将位图清除干净
		
	if (m_bflag)
	{
		m_DlgPic.Load(IDR_JPG_BTN1_1);
		
	} 
	else
	{
		m_DlgPic.Load(IDR_JPG_BTN1);
	}
	m_DlgPic.Render(&memdc,m_rtbutton);

	if (!m_bConnect)
	{
		m_strLanguage[3] = GetStringLanguage("Login","Button1","连接");
	}
	else
	{
		m_strLanguage[3] = GetStringLanguage("Login","Button2","断开连接");
	}

	CFont   *pOldfont = (CFont *)memdc.SelectObject(&m_font); 
    memdc.SetTextColor(RGB(0, 0, 0)); 
	memdc.DrawText(m_strLanguage[3],m_rtbutton,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pDC->BitBlt(0,0,m_rect.Width(),m_rect.Height(),&memdc,0,0,SRCCOPY);
	bmp.DeleteObject();
	memdc.DeleteDC();
	ReleaseDC(pDC);
	return true;
}



HBRUSH CUserDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if(nCtlColor == CTLCOLOR_STATIC) 
	{ 
		if(pWnd->GetDlgCtrlID()== IDC_STATIC1||IDC_STATIC2||IDC_STATIC3)
        {
            pDC->SetBkMode(TRANSPARENT);   
			return   (HBRUSH)GetStockObject(NULL_BRUSH);
        }
	 
	} 
	if(nCtlColor ==CTLCOLOR_DLG)
	{
		return m_brush; //返加刷子
	}
	// TODO: Return a different brush if the default is not desired
	return hbr;
}



BOOL CUserDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message==WM_KEYDOWN&&pMsg->wParam==VK_RETURN)
	{
		
		return NULL;
	}

	return CDialog::PreTranslateMessage(pMsg);
}



void CUserDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	delete this;
	CDialog::PostNcDestroy();
}
