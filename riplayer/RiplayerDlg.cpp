// RiplayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Riplayer.h"
#include "RiplayerDlg.h"
#include "RecordDlg.h"
#include "PictureDlg.h"
#include "SetEncodeDlg.h"
#include "SetDlg.h"
#include "InfoDlg.h"
#include "PlayVedioDlg.h"
#include "AboutDlg1.h"
#include "RMedia.h"
#include <DIRECT.H>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define  MIN_WIDTH    600
#define  MIN_HEIGHT   450
#define  INIFILE  _T("SavePathName.ini");

struct Play_Info{
 char flag;
 char protocol;
 char multicast; 
 unsigned short port;
 char ip[16];
}play_info,prev_play_info;

static char sdp_pBuf[1024];
static char rtsp_url[32]={0};
#define CONFIG_FILE _TEXT("Config.ini")
const int MAX_LANGUAGE_MENU = 16;
CString strLngFile[MAX_LANGUAGE_MENU];
bool m_bDragFullWindow;


/****************************************
* 与内核通信公共变量

*****************************************/
static uint64_t g_lost_frame_cnt=0u;
static bool  lost_frame_flag = true;

static 	bool m_recording=0;
bool record_status = false;
static uint64_t record_s=0;
/****************************************/

int rctemp; //音量变化量



DWORD sdp_FileLen;
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
extern HANDLE m_handle; 
RTP_Handle *rtp_handle;
RTP_Handle *rtp_audio_handle;
TS_Handle *ts_handle;
TS_Handle *ts_rtp_handle;
RTSP_HANDLE *rtsp_handle;
Encoder_Handle* encoder_handle = NULL;
	
CRiplayerDlg * CRiplayerDlg::m_staiicthis = NULL;


record_info_t info;
unsigned int timetick = 0;
static bool vedio_bResolution = false;
HANDLE hMutex;
static int nStatus = 0; // 1: play, 0: stop
/////////////////////////////////////////////////////////////////////////////
// CRiplayerDlg dialog
static void StopRecrod(void);

CRiplayerDlg::CRiplayerDlg(CWnd* pParent /*=NULL*/)
: CDialog(CRiplayerDlg::IDD, pParent)
{
	m_display=false;
//	m_recording=false;
	m_play=false;
	m_voice=false;
	m_pause=false;
	m_normal=false;
	m_close=false;
	m_max=false;
	m_min=false;
	m_vedio=false;
	m_htry=false;
	m_tool=false;
	m_help=false;
	m_bDragFullWindow=false;
	m_moveball=false;
	rctemp=0;
	m_targ=false;
	m_bOpenAudio=true;
	m_bFullScreen=false;
	m_rectplay=false;
	m_nCount=0;
	m_nInRect = 0;
	m_nVol = 0;
	m_bMax = FALSE;
	for (int i = 0; i < 12; i++)
	{
		m_bInRect[i]= false;
	}
	
	m_IniFileName = _T("HistoryMenu.ini");	//设置ini文件名
	for( i=0; i<MAXNUM; i++ )
		m_PathName[i] = _T("");
	m_CurNum = 0;
    m_hMruMenu.LoadMenu(IDR_MENU2);//加载菜单ID
	m_HelpMenu.LoadMenu(IDR_HELPMENU);
	m_MediaMenu.LoadMenu(IDR_MEDIAMENU);
	m_ToolMenu.LoadMenu(IDR_TOOLMENU);
	m_rKeyMenu.LoadMenu(IDR_POPMENU);
	m_hHistoryMenu = m_hMruMenu.GetSubMenu(0);
	m_pHelpMenu = m_HelpMenu.GetSubMenu(0);
	m_pMediaMenu = m_MediaMenu.GetSubMenu(0);
	m_pToolMenu = m_ToolMenu.GetSubMenu(0);
	m_rKeyMenuSub = m_rKeyMenu.GetSubMenu(0);

	rtp_handle = NULL;
	rtp_audio_handle = NULL;
	ts_handle  = NULL;
	ts_rtp_handle=NULL;
	encoder_handle=NULL;
	sdp_info = NULL;
	dlgTip = NULL; //构造函数一定要把dlgTip置NULL
	m_nMenuDown = 0;

	m_staiicthis = this;
	m_strInIName = strAppDir +INIFILE; //保存文件名
	//{{AFX_DATA_INIT(CRiplayerDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
CRiplayerDlg::~CRiplayerDlg()
{
//	DestroyMenu()

}
void CRiplayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRiplayerDlg)
	DDX_Control(pDX, IDC_STATIC_VIEW, m_staticView);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRiplayerDlg, CDialog)
//{{AFX_MSG_MAP(CRiplayerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_COMMAND(IDM_OPEN, OnOpen)
	ON_COMMAND(IDM_SETENCODE, OnSetencode)
	ON_COMMAND(IDM_INFO, OnInfo)
	ON_WM_SIZE()
	ON_COMMAND(IDM_ABOUT, OnAbout)
	ON_COMMAND(IDM_SETP, OnSetp)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_MRU_CLR, OnMruClr)
	ON_BN_CLICKED(IDC_BTN_PICTURE, OnBtnPicture)
	ON_COMMAND(IDM_SETR, OnSetr)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_NCHITTEST()
	ON_WM_MOUSEMOVE()
	ON_WM_GETMINMAXINFO()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_COMMAND(IDM_EXIT, OnExit)
	ON_COMMAND(IDC_BTN_RECORD, OnBtnRecord)
	ON_COMMAND(IDC_BTN_PAUSE, OnBtnPause)
	ON_COMMAND(IDC_BTN_STOP, OnBtnStop)
	ON_WM_CTLCOLOR()
	ON_COMMAND(IDM_FULLSCREEN, OnFullscreen)
	ON_COMMAND(IDM_ORIGINAL, OnOriginal)
	ON_COMMAND(IDM_SMALL, OnSmall)
	ON_COMMAND(IDM_LARGE, OnLarge)
	ON_COMMAND(IDM_LICENSE, OnLicense)
	ON_COMMAND(IDR_CLEARMENU, OnClearmenu)
	ON_COMMAND(IDR_CHINESE, OnChinese)
	ON_COMMAND(IDR_ENGLISH, OnEnglish)
	ON_COMMAND(IDC_BTN_PLAY, OnBtnPlay)
	ON_COMMAND(IDM_STOPRECROD, OnStoprecrod)
	ON_WM_SETCURSOR()
	ON_COMMAND(IDM_OPENFILE, OnOpenfile)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_MOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_COMMAND(IDC_BTN_PICTURE, OnBtnPicture)
	//}}AFX_MSG_MAP
	ON_STN_DBLCLK(IDC_STATIC_VIEW, OnStaticDblClick) //static双击事件消息映射
	ON_STN_CLICKED(IDC_STATIC_VIEW, OnStaticClick)
	ON_COMMAND( WM_NEW_MSG , MsgBox)


END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRiplayerDlg message handlers

/****************************************************
********************  YUB转RGB **********************
****************************************************/
#define GET_R16(c1,su,sv) (((c1)+409*(sv))>>8)
#define GET_G16(c1,su,sv) (((c1)-100*(su)-208*(sv))>> 8)
#define GET_B16(c1,su,sv) (((c1)+516*(su))>> 8)

#define GET_R32(c1,su,sv) (((c1)+359*(sv))>>8)
#define GET_G32(c1,su,sv) (((c1)-88*(su)-183*(sv))>> 8)
#define GET_B32(c1,su,sv) (((c1)+454*(su))>> 8)

#define GET_R(c1,su,sv) (((c1)+359*(sv))>>8)
#define GET_G(c1,su,sv) (((c1)-88*(su)-183*(sv))>> 8)
#define GET_B(c1,su,sv) (((c1)+454*(su))>> 8)

static __inline int CLIP(int value)
{

	if (value & (~255))
		value = (-value) >> 31;
	return value;
}

static void ConvertYUV2RGB( unsigned char *src, int stride[3],
						   unsigned char *dst, int dst_stride,
						   int width, int height)
{
	int wrap,wrap2,x,y;
	int tmp=0;
	int sy,su,sv;
	int c1;
	unsigned char *py;
	unsigned char *pu;
	unsigned char *pv;
	
	wrap=stride[0];
	wrap2=dst_stride;
	py=src;
	pu=src + width*height;
	pv=src + width*height + width*(height>>2);
	
	for(y=0;y<height;y+=2)
	{
		for(x=0;x<width;x+=2)
		{
			sy=py[0];
			su=*(pu++)-128;
			sv=*(pv++)-128;
			
			c1=(sy<<8);
			//dst[3]=0xff;
			dst[2]=CLIP(GET_R(c1,su,sv));
			dst[1]=CLIP(GET_G(c1,su,sv));
			dst[0]=CLIP(GET_B(c1,su,sv));
			//*((short*)dst) = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b ) >> 3);
			
			sy=py[1];
			c1=(sy<<8);
			//dst[7]=0xff;
			dst[5]=CLIP(GET_R(c1,su,sv));
			dst[4]=CLIP(GET_G(c1,su,sv));
			dst[3]=CLIP(GET_B(c1,su,sv));
			//*((short*)(dst+2)) = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b ) >> 3);
			
			sy=py[wrap];
			c1=(sy<<8);
			//dst[wrap2+3]=0xff;
			dst[wrap2+2]=CLIP(GET_R(c1,su,sv));
			dst[wrap2+1]=CLIP(GET_G(c1,su,sv));
			dst[wrap2]=CLIP(GET_B(c1,su,sv));
			//*((short*)(dst+wrap2)) = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b ) >> 3);
			
			sy=py[wrap+1];
			c1=(sy<<8);
			//dst[wrap2+7]=0xff;
			dst[wrap2+5]=CLIP(GET_R(c1,su,sv));
			dst[wrap2+4]=CLIP(GET_G(c1,su,sv));
			dst[wrap2+3]=CLIP(GET_B(c1,su,sv));
			//*((short*)(dst+wrap2+2)) = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b ) >> 3);
			
			py+=+2;
			dst+=+6;
		}
		py+=(stride[0]<<1)-width;
		pu+=stride[1]-(width>>1);
		pv+=stride[2]-(width>>1);
		//dst+=(wrap2<<1)-(width<<2);
		dst+=(wrap2<<1)-(width*3);
		
	}
}

BOOL CRiplayerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetSystemMetrics(SM_CYFULLSCREEN); 
	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	SetWindowText("RiPlayer"); //设置任务栏标题
	// TODO: Add extra initialization here
	GetClientRect(&m_rect);//获取当前对话框的大小
	GetConfig();
	ReadMru();		//读取组播历史记录
	
	InitRect();
	m_dlgInfo.Create(IDD_INFO);
	
	//使静态显示框支持鼠标消息响应
	DWORD dwStyle = m_staticView.GetStyle();
	dwStyle |= SS_NOTIFY;
	SetWindowLong(m_staticView.GetSafeHwnd(), GWL_STYLE, dwStyle);

	m_nScreenCx = GetDC()->GetDeviceCaps(HORZRES);   //获取屏幕的水平尺寸
	m_nScreenCy = GetDC()->GetDeviceCaps(VERTRES);   //获取屏幕的垂直尺寸
	SetRect(m_rcLastMoveRect, 0, 0, 0, 0);
	if (GetFileAttributes("D:\\My documents") == 0xFFFFFFFF)
	{
		mkdir("D:\\My documents");
	}
	dlgTip = new CTipDlg;
	dlgTip->Create(IDD_TIPDIALOG,this); 
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRiplayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
	
		;
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRiplayerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting
		
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);
		
		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		
		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


void CRiplayerDlg::OnAbout() 
{
	// TODO: Add your command handler code here
	AboutDlg dlgAbout;
	dlgAbout.DoModal();

}

void CRiplayerDlg::GetConfig()
{
	CString strConfig = strAppDir + CONFIG_FILE;
	CString strTemp;
	char buf[100];

	CString strLang;
	memset(buf,0,100);	
	strLang.Format("Language%d",g_Language);
	GetPrivateProfileString("Language",strLang,"ChineseGB.dll",buf,100,strConfig);
	strLanguageFile = buf;
	strLanguageFile = strAppDir + strLanguageFile;
	strTemp.Format("%d",g_Language);
	WritePrivateProfileString("Language","Default",strTemp,strConfig);
	SetLang();
}
//语言配置信息
void CRiplayerDlg::SetLang()
{

	UINT nFlag = MF_BYCOMMAND | MF_STRING;	
	if (g_Language)
	{
		m_pToolMenu->GetSubMenu(6)->GetSubMenu(2)->CheckMenuItem(0,MF_BYPOSITION|MF_UNCHECKED); 
		m_pToolMenu->GetSubMenu(6)->GetSubMenu(2)->CheckMenuItem(1,MF_BYPOSITION|MF_CHECKED); 
	}
	else
	{
		m_pToolMenu->GetSubMenu(6)->GetSubMenu(2)->CheckMenuItem(0,MF_BYPOSITION|MF_CHECKED); 
		m_pToolMenu->GetSubMenu(6)->GetSubMenu(2)->CheckMenuItem(1,MF_BYPOSITION|MF_UNCHECKED); 
	}

	m_hHistoryMenu->ModifyMenu(IDR_CLEARMENU,nFlag,IDR_CLEARMENU,GetStringLanguage("Main","Menu10","清除"));//修改清除菜单项名 
	
	m_pHelpMenu->ModifyMenu(IDM_ABOUT,nFlag,IDM_ABOUT, GetStringLanguage("Main","Menu30","关于"));//修改清除菜单项名 
	m_pHelpMenu->ModifyMenu(IDM_LICENSE,nFlag,IDM_LICENSE,GetStringLanguage("Main","Menu31","许可证"));
	
	m_pMediaMenu->ModifyMenu(IDM_OPEN,nFlag,IDM_OPEN,GetStringLanguage("Main","Menu00","打开媒体")); 
	m_pMediaMenu->ModifyMenu(IDM_EXIT,nFlag,IDM_EXIT,GetStringLanguage("Main","Menu01","退出"));
	m_pMediaMenu->ModifyMenu(IDM_OPENFILE,nFlag,IDM_OPENFILE,GetStringLanguage("Main","Menu02","打开文件"));
	
	m_pToolMenu->ModifyMenu(IDC_BTN_PICTURE,nFlag,IDC_BTN_PICTURE,GetStringLanguage("Main","Menu21","拍照"));
	m_pToolMenu->ModifyMenu(IDC_BTN_RECORD,nFlag,IDC_BTN_RECORD,GetStringLanguage("Main","Menu22","录制")); 
	m_pToolMenu->ModifyMenu(IDM_SETENCODE,nFlag,IDM_SETENCODE,GetStringLanguage("Main","Menu24","设置编码器"));	
	m_pToolMenu->ModifyMenu(IDM_INFO,nFlag,IDM_INFO,GetStringLanguage("Info","Caption","媒体中心")); 	
	m_pToolMenu->ModifyMenu(1,MF_BYPOSITION|MF_STRING,0,GetStringLanguage("Main","Menu20","视频缩放"));
	m_pToolMenu->ModifyMenu(6,MF_BYPOSITION|MF_STRING,3,GetStringLanguage("Main","Menu23","设置"));
	
	m_pToolMenu->GetSubMenu(1)->ModifyMenu(IDM_FULLSCREEN,nFlag,IDM_FULLSCREEN,GetStringLanguage("Main","SubMenu00","全屏"));
	m_pToolMenu->GetSubMenu(1)->ModifyMenu(IDM_ORIGINAL,nFlag,IDM_ORIGINAL,GetStringLanguage("Main","SubMenu01","原始"));
	m_pToolMenu->GetSubMenu(6)->ModifyMenu(IDM_SETP,nFlag,IDM_SETP,GetStringLanguage("Main","SubMenu10","Full Screen"));
	m_pToolMenu->GetSubMenu(6)->ModifyMenu(IDM_SETR,nFlag,IDM_SETR,GetStringLanguage("Main","SubMenu11","拍照路径"));
	m_pToolMenu->GetSubMenu(6)->ModifyMenu(2,MF_BYPOSITION|MF_STRING,2,GetStringLanguage("Main","SubMenu12","录制路径"));
	
	m_rKeyMenuSub->ModifyMenu(IDM_OPEN,nFlag,IDM_OPEN,GetStringLanguage("RMenu","RMenu0","打开媒体"));
	m_rKeyMenuSub->ModifyMenu(1,MF_BYPOSITION|MF_STRING,1,GetStringLanguage("RMenu","RMenu1","视频缩放"));
	m_rKeyMenuSub->ModifyMenu(IDC_BTN_PLAY,nFlag,IDC_BTN_PLAY,GetStringLanguage("RMenu","RMenu2","播放"));
	m_rKeyMenuSub->ModifyMenu(IDC_BTN_PAUSE,nFlag,IDC_BTN_PAUSE,GetStringLanguage("RMenu","RMenu3","暂停"));
	m_rKeyMenuSub->ModifyMenu(IDC_BTN_STOP,nFlag,IDC_BTN_STOP,GetStringLanguage("RMenu","RMenu4","停止"));
	m_rKeyMenuSub->ModifyMenu(IDC_BTN_RECORD,nFlag,IDC_BTN_RECORD,GetStringLanguage("RMenu","RMenu5","录制"));
	m_rKeyMenuSub->ModifyMenu(IDM_STOPRECROD,nFlag,IDM_STOPRECROD,GetStringLanguage("RMenu","RMenu6","停止录制"));
	m_rKeyMenuSub->ModifyMenu(IDC_BTN_PICTURE,nFlag,IDC_BTN_PICTURE,GetStringLanguage("RMenu","RMenu7","拍照"));
	m_rKeyMenuSub->ModifyMenu(IDM_SETENCODE,nFlag,IDM_SETENCODE,GetStringLanguage("RMenu","RMenu8","设置编码器"));
	m_rKeyMenuSub->ModifyMenu(IDM_INFO,nFlag,IDM_INFO,GetStringLanguage("Info","Caption","媒体中心"));

	m_rKeyMenuSub->GetSubMenu(1)->ModifyMenu(IDM_FULLSCREEN,nFlag,IDM_FULLSCREEN,GetStringLanguage("Main","SubMenu00","全屏"));
	m_rKeyMenuSub->GetSubMenu(1)->ModifyMenu(IDM_ORIGINAL,nFlag,IDM_ORIGINAL,GetStringLanguage("Main","SubMenu01","原始"));
	  
	InvalidateRect(m_rtvedio);
	InvalidateRect(m_rthtry);
	InvalidateRect(m_rttool);
	InvalidateRect(m_rthelp);
	
	
}

void CRiplayerDlg::InitRect()
{
	/*******   初始化矩形区域 ****************/

	m_rtleftop.SetRect(m_rect.left,m_rect.top,m_rect.left+8,m_rect.top+44);//左上角
	m_rtrightop.SetRect(m_rect.right-12,0,m_rect.right,48);//右上角
	m_rtleftb.SetRect(0,m_rect.bottom-69,12,m_rect.bottom);//左下角
	m_rtrightb.SetRect(m_rect.right-17,m_rect.bottom-55,m_rect.right,m_rect.bottom);//右下角
	m_rtbottombar.SetRect(12,m_rect.bottom-51,m_rect.right-17,m_rect.bottom);//下面控件条
	m_rttopbar.SetRect(m_rect.left+8,m_rect.top,m_rect.right - 10,m_rect.top+32);//上面控制条
	m_rtleftbar.SetRect(m_rect.left,m_rect.top+44,m_rect.left+6,m_rect.bottom-55);//左边控制条
	m_rtrightbar.SetRect(m_rect.right-8,m_rect.top+44,m_rect.right,m_rect.bottom-43);//右边控制条
	m_rtplay.SetRect((m_rect.Width()/2)-7,m_rect.bottom-42,(m_rect.Width()/2)+32,m_rect.bottom-1);// 播放
	m_rtstop.SetRect((m_rect.Width()/2)+41,m_rect.bottom-37,(m_rect.Width()/2)+71,m_rect.bottom-7);//停止
	m_rtpicture.SetRect((m_rect.Width()/2)-47,m_rect.bottom-37,(m_rect.Width()/2)-17,m_rect.bottom-7);//拍照
	m_rtrecord.SetRect((m_rect.Width()/2)-86,m_rect.bottom-37,(m_rect.Width()/2)-56,m_rect.bottom-7);//录制
	m_rtvoice.SetRect((m_rect.Width()/2)+108,m_rect.bottom-31,(m_rect.Width()/2)+128,m_rect.bottom-12);//音量
	m_rtbk.SetRect((m_rect.Width()/2)+132,m_rect.bottom-27,(m_rect.Width()/2)+235,m_rect.bottom-19);//音量条
	m_rtball.SetRect(m_rtbk.left+rctemp,m_rect.bottom-31,m_rtbk.left+14+rctemp,m_rect.bottom-14);//滑块位置
	m_rtclose.SetRect(m_rect.Width()-34,2,m_rect.Width()-7,22);// 关闭
	m_rtmax.SetRect(m_rect.right-64,2,m_rect.right-37,22);//最大化
	m_rtmin.SetRect(m_rect.right-94,2,m_rect.right-67,22);//最小化
	m_rtvedio.SetRect(m_rect.left+42,m_rect.top+3,m_rect.left+95,m_rect.top+25);//媒体
	m_rthtry.SetRect(m_rect.left+100,m_rect.top+3,m_rect.left+165,m_rect.top+25);//历史
	m_rttool.SetRect(m_rect.left+168,m_rect.top+3,m_rect.left+222,m_rect.top+25);//工具
	m_rthelp.SetRect(m_rect.left+226,m_rect.top+3,m_rect.left+280,m_rect.top+25);//帮助
	m_rc.SetRect(m_rect.left+m_rthelp.right,m_rect.top,m_rect.right-88,m_rect.top+42); //标题栏
	m_rtdisplay.SetRect(m_rect.left+4,m_rect.top+30,m_rect.right-8,m_rect.bottom-46);//显示
	m_rtcenterbar1.SetRect(m_rect.Width()/2-56,m_rect.bottom-26,m_rect.Width()/2-47,m_rect.bottom-17);
	m_rtcenterbar2.SetRect(m_rect.Width()/2-17,m_rect.bottom-26,m_rect.Width()/2-7,m_rect.bottom-17);
	m_rtcenterbar3.SetRect(m_rect.Width()/2+32,m_rect.bottom-26,m_rect.Width()/2+41,m_rect.bottom-17);
	m_rticon.SetRect(m_rect.left+10,m_rect.top+1,m_rect.left+41,m_rect.top+27);//左上角图标
	m_rtvolume.SetRect(m_rtbk.right+10,m_rect.bottom-30,m_rtbk.right+45,m_rect.bottom-14);//音量值位置
	m_rcview.SetRect(m_rect.Width()/2 - 30, m_rect.Height()/2 - 30, m_rect.Width()/2 + 30, m_rect.Height()/2 + 30);
	

}

//读取ini文件中的记录
void CRiplayerDlg::ReadMru()
{
	m_strHistroyFile = strAppDir + m_IniFileName;
	m_CurNum = ::GetPrivateProfileInt("Mru File", "FileNum", 0, m_strHistroyFile);  
	if (m_CurNum ==0)
	{
		return;
	}
	
	CString no;
	for( int i=0; i<m_CurNum; i++)
	{
		no.Format( "%d", i+1 );   //求项名
		::GetPrivateProfileString("Mru File", no, "", m_PathName[i].GetBuffer(MAX_PATH),MAX_PATH, m_strHistroyFile);   //读取历史记录名
		
		
	}
	for ( i = 0;  i < m_CurNum; i++)
	{
		m_hHistoryMenu->InsertMenu(i, MF_BYPOSITION|MF_STRING, HISTORY_INDEX+i, m_PathName[m_CurNum - i -1]);// 往下插入菜单
		
	}
			
}

//历史记录写入ini文件
void CRiplayerDlg::WriteMru()
{
	CString no;
	
	no.Format( "%d", m_CurNum );
	::WritePrivateProfileString( "Mru File", "FileNum", no, m_strHistroyFile );  //写当前文件数
	for( int i=0; i<m_CurNum; i++)
	{
		no.Format( "%d", i+1 );
		::WritePrivateProfileString("Mru File", no, m_PathName[i], m_strHistroyFile); //写路径名
	}
}

//添加历史记录，若记录已存在则不添加
void CRiplayerDlg::AddMru(CString nPathName)
{
	
	CString str1, str2;
	for(int i = 0; i<m_CurNum; i++)
	{
		if(nPathName.CompareNoCase(m_PathName[i])==0 ) //若记录已经存在则不添加
		{
			break;
		}
	}
	if(i < m_CurNum)
	{
		;
	}
	else
	{
		if(m_CurNum < MAX_MENU)
		{//历史记录未达到最大值
			m_PathName[m_CurNum] = nPathName;  //表未满
			m_CurNum++;
			for (int i = 0; i < m_CurNum; i++)
			{
				m_hHistoryMenu->DeleteMenu(HISTORY_INDEX+i,MF_BYCOMMAND);//清空历史记录菜单
				
			}
			for ( i = 0;  i < m_CurNum; i++)
			{
				m_hHistoryMenu->InsertMenu(i, MF_BYPOSITION|MF_STRING, HISTORY_INDEX+i, m_PathName[m_CurNum - i -1]);// 往下插入菜单
				
			}
			
			
		}	
	}

	WriteMru();     //历史记录写入ini文件
}


//清除历史记录
void CRiplayerDlg::ClearMru()
{	
	for( int i=0; i < m_CurNum; i++)
	{
		m_PathName[i]="";
		m_hHistoryMenu->DeleteMenu(HISTORY_INDEX+i,MF_BYCOMMAND);//清空历史记录菜单
	}	
	m_CurNum = 0;
	WriteMru();	
}

/***** 打开URL界面*******/
void CRiplayerDlg::OnOpen() 
{
	// TODO: Add your command handler code here
	CPlayVedioDlg dlgOpen;
	PRINTF("Parent Wnd = 0x%x -- 0x%x\n",dlgOpen.m_hWnd,m_hWnd);
	dlgOpen.DoModal();
	if (dlgOpen.m_edit1=="")
	{
		return;
	}
	else
	{		
		m_strURL=dlgOpen.m_edit1;
		
    	int index = OnPlay(m_strURL);
		if (index ==1)//输入的组播放地址正确才保存
		{
			AddMru(m_strURL);
			Tipmsg(m_strURL);
			g_strUrl=m_strURL;
		}
		
	}	
}


/***** 打开编码器设置窗口 *******/
void CRiplayerDlg::OnSetencode() 
{
	// TODO: Add your command handler code here
	CSetDlg dlg;
	dlg.DoModal();
}

/****** INFO显示窗口 *********/
void CRiplayerDlg::OnInfo() 
{
	// TODO: Add your command handler code here
	m_dlgInfo.InitLanguage();
	m_dlgInfo.ShowWindow(SW_SHOW);

}

/****** 控件大小及位置变化  ********/
void CRiplayerDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if(nType==1)//大小没有变化则返回
		return;
	if(nType = SIZE_MINIMIZED )
	{
		GetClientRect(&m_rect); // 将变化后的对话框大小设为旧大小，否则无法还原控件原始位置
		InitRect();          
		
	}

	/******** 控件随窗口大小变化而变化 *************/
	static RECT LastWindowRect = {0, 0, 0, 0};
    CWnd *hWin;
    int ux = cx - LastWindowRect.right;
    int uy = cy - LastWindowRect.bottom; 
	
    if((hWin = GetDlgItem(IDC_STATIC_VIEW)) != NULL)
    {
        RECT rect;
        hWin->GetWindowRect(&rect);
        ScreenToClient(&rect);
        rect.right += ux;
        rect.bottom += uy;
        hWin->MoveWindow(&rect);
    }
    LastWindowRect.right = cx;
    LastWindowRect.bottom = cy;
	
	CRgn m_rgn;
    CRect rc;
	GetWindowRect(&rc);
	rc-=rc.TopLeft();
	m_rgn.CreateRoundRectRgn(rc.left,rc.top,rc.right,rc.bottom,3,3);
	SetWindowRgn(m_rgn,TRUE);
}

/**** 拍照路径设置界面  *****/
void CRiplayerDlg::OnSetp() 
{
	// TODO: Add your command handler code here
	CPictureDlg dlg;
	dlg.DoModal();
}

/*******鼠标双击左键**********/
void CRiplayerDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	if (!m_bFullScreen)
	{
		if(m_rc.PtInRect(point)) 
		{		
			OnMax();
		}

	}
	CDialog::OnLButtonDblClk(nFlags, point);
}


/*****清除历史记录菜单*********/
void CRiplayerDlg::OnMruClr() 
{
	// TODO: Add your command handler code here
	ClearMru();
}

/******** 拍照 ***********/
void CRiplayerDlg::OnBtnPicture() 
{
	// TODO: Add your control notification handler code here

	if (prv_nHieght||prv_nWidth)
	{
		int nOutLen=0;
		int nWidth=0;
		int nHeight=0;
		int nCount = 0;
		char imagename[64]={0};
		CString strPathName,str,strTemp,strCount,strPath;
		str=GetStringLanguage("Picture","Static2","图片已存至");
		CTime time=CTime::GetCurrentTime();
		CString strTime=time.Format("%Y-%m-%d");	//时间格式
		::GetPrivateProfileString("Picture", "PictureName", strTime, strTemp.GetBuffer(MAX_PATH),MAX_PATH, m_strInIName);   
		::GetPrivateProfileString("Picture", "PictureNum", "0", strCount.GetBuffer(MAX_PATH),MAX_PATH, m_strInIName);   
		nCount = atoi(strCount);
		if (strTemp != strTime)
		{
			strTemp = strTime; 
			nCount = 1;
		}
		else
		{
			nCount++;
			
		}
		strCount.Format("%d", nCount);
		::WritePrivateProfileString("Picture", "PictureName", strTemp, m_strInIName); 
		::WritePrivateProfileString("Picture", "PictureNum", strCount, m_strInIName); 
		strTemp.Format("%s %d", strTemp, nCount);
		sprintf(imagename,"%s.JPG", strTemp); //CString to char
		
		
		CPictureDlg dlg;
		dlg.ReadMru();
		strPath=dlg.m_PsPathName;
		if (GetFileAttributes(strPath) == 0xFFFFFFFF)
		{
			CMsgBoxDlg dlg;
			dlg.SetMsg(GetStringLanguage("MsgBox", "Caption3", "错误"), GetStringLanguage("MsgBox", "Tip12", "路径不存在"));
			dlg.DoModal();
			return;
		}
		strPathName.Format("%s\\%s", strPath, imagename);
		sprintf(imagename,"%s", strPathName);
		if(GetFileAttributes( "imagename ")!=0xFFFFFFFF) 
		{ 
			DeleteFile( "imagename "); 
		}

		const	BYTE* pTemp = GetYUVData(m_handle, &nOutLen, &nWidth, &nHeight);
		if (nWidth||nHeight)
		{
			int nStride[3] = {0};
			nStride[0] = nWidth;
			nStride[1] = nWidth>>1;
			nStride[2] = nWidth>>1;
			int nLength = nWidth * nHeight * 4;
			unsigned char *pRGBBuffer = NULL;
			pRGBBuffer = new unsigned char[nLength];
			ZeroMemory(pRGBBuffer, nLength);
			ConvertYUV2RGB((LPBYTE)pTemp, nStride, pRGBBuffer, nWidth*3, nWidth, nHeight);
			
			int nLen = 0;
			nLen = strlen((char*)pRGBBuffer);
			FILE *fp = fopen(imagename,"a+");
			if(fp)
			{
				fclose(fp);
			}
			else
			{
				CMsgBoxDlg dlg;
				dlg.SetMsg(GetStringLanguage("MsgBox", "Caption2", "提示"), GetStringLanguage("MsgBox", "Tip13", "文件创建失败!"));
				dlg.DoModal();
				record_status=false;
				m_recording=false;
				return;
				
			}
			SaveJpgImage((BYTE*)pRGBBuffer,nWidth,nHeight,24,imagename);
			strPath.Format("%s %s", str,strPath);
			Tipmsg(strPath);
			delete []pRGBBuffer;
			pRGBBuffer = NULL;
		}
		
	}
}

/******** 路径设置界面 **********/
void CRiplayerDlg::OnSetr() 
{
	// TODO: Add your command handler code here
	CRecordDlg dlg;
	dlg.DoModal();
}

BOOL CRiplayerDlg::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	GetWindowRect(&rect);
	GetClientRect(&m_rect);
	CDC memdc;       //定义一个显示设备对象 
	CBitmap bmp;			//定义一个位图对象 
	CFont font;
	memdc.CreateCompatibleDC(pDC);     //建立与屏幕显示兼容的内存显示设备 
	
	bmp.CreateCompatibleBitmap(pDC,rect.Width(),rect.Height());//建立一个与屏幕显示兼容的位图
	CBitmap* pOldBmp = memdc.SelectObject(&bmp);            //将位图选入到内存显示设备中		
	memdc.SetBkMode(TRANSPARENT);
	DWORD style = DT_CENTER|DT_SINGLELINE|DT_VCENTER;
	CBrush bru(RGB(0,0,0));
	CRect cr(0,0,rect.Width(),rect.Height());	
	
	memdc.FillRect(&cr,&bru); 	//用背景色将位图清除干净
	if (!m_bFullScreen)
	{
		m_DlgPic.Load(IDR_JPG2);//左上角
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtleftop);
		
		m_DlgPic.Load(IDR_JPG7);//右上角
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtrightop);
		
		m_DlgPic.Load(IDR_JPG4);//左下角
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtleftb);
		
		m_DlgPic.Load(IDR_JPG5);//右下角
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtrightb);
		
		m_DlgPic.Load(IDR_JPG6);//下面控制条
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtbottombar);
		
		m_DlgPic.Load(IDR_JPG3);//左边控制条
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtleftbar);
		
		m_DlgPic.Load(IDR_JPG8);//右边控制条
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtrightbar);
		
		m_DlgPic.Load(IDR_JPG1);//上面控制条
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rttopbar);

		m_DlgPic.Load(IDR_JPG_ICON);//close
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rticon);

		if (!m_display)
		{
			m_DlgPic.Load(IDR_JPG_DISPLAY);
			m_DlgPic.Render(&memdc,m_rtdisplay);
		}

	
		if(m_pause)
		{
			m_DlgPic.Load(IDR_JPG_PAUSE);//PAUSE
				
		}
		else
		{
			m_DlgPic.Load(IDR_JPG_PLAY);//PLAY
		}
	
		m_DlgPic.Render(&memdc,m_rtplay);
		
		if(m_voice)//为真静音，为假正常
		{
			m_DlgPic.Load(IDR_JPG_MUTE);//静音
			
		}
		else
		{
			m_DlgPic.Load(IDR_JPG_VOICE);//正常 
		}
		m_DlgPic.Render(&memdc,m_rtvoice);
		
		if(m_normal)
		{
			m_DlgPic.Load(IDR_JPG_ORIG);//
			
		}
		else
		{
			m_DlgPic.Load(IDR_JPG_MAX);//
		}
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtmax);
		
		m_DlgPic.Load(IDR_JPG_STOP);//STOP
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtstop);
		
		m_DlgPic.Load(IDR_JPG_PICTURE);//PICTURE
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtpicture);
		
		if (m_recording)
		{
			m_DlgPic.Load(IDR_RECSTOP);
		}
		else
		{
			m_DlgPic.Load(IDR_JPG_RECORD);
		}
		m_DlgPic.Render(&memdc,m_rtrecord);
		
		m_DlgPic.Load(IDR_JPG_SLIDER);//SLIDER 进度条
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtbk);
		
		m_DlgPic.Load(IDR_JPG_SLIDER0);//SLIDER 滑块
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtball);
		
		m_DlgPic.Load(IDR_JPG_CLOSE);//close
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtclose);
		
		
		m_DlgPic.Load(IDR_JPG_MIN);//min
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtmin);

		m_DlgPic.Load(IDR_JPG_BAR);//min
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtcenterbar1);//按钮间横条

		m_DlgPic.Load(IDR_JPG_BAR);//min
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtcenterbar2);

		m_DlgPic.Load(IDR_JPG_BAR);//min
		if(m_DlgPic != NULL)
			m_DlgPic.Render(&memdc,m_rtcenterbar3);
		
		
		switch (m_nCount)// 鼠标划过换图 
		{
		case 0:	 
			{
				
			}
			break;
		case 1:
			{
				if(m_pause)
				{
					m_DlgPic.Load(IDR_JPG_PAUSE1);//PAUSE
					
				}
				else
				{
					m_DlgPic.Load(IDR_JPG_PLAY1);//PLAY
				}
				m_DlgPic.Render(&memdc,m_rtplay);
			}	
			break;	
		case 2:   
			{
				if (m_display)
				{
					m_DlgPic.Load(IDR_JPG_STOP1);//STOP
					m_DlgPic.Render(&memdc,m_rtstop);
				}
			}
			break;
		case 3:   	
			{
				if (m_display)
				{
					m_DlgPic.Load(IDR_JPG_PICTURE1);//PICTURE
					m_DlgPic.Render(&memdc,m_rtpicture);
				}
							
			}
			break;
		case 4:		
			{
				if (m_display)
				{
					if (m_recording)
					{
						m_DlgPic.Load(IDR_RECSTOP1);
					}
					else
					{
						m_DlgPic.Load(IDR_JPG_RECORD1);
					}
					m_DlgPic.Render(&memdc,m_rtrecord);
				}
				
			}
			break;

		case 5:		
			{
				m_DlgPic.Load(IDR_JPG_CLOSE1);//close
				if(m_DlgPic != NULL)
					m_DlgPic.Render(&memdc,m_rtclose);
			}
			break;
		case 6:		
			{
				if(m_normal)
				{
					m_DlgPic.Load(IDR_JPG_ORIG1);
					
					
				}
				else
				{
					m_DlgPic.Load(IDR_JPG_MAX1);
					
				}
				m_DlgPic.Render(&memdc,m_rtmax);
			}
			break;
		case 7:	
			{
				m_DlgPic.Load(IDR_JPG_MIN1);//min
				if(m_DlgPic != NULL)
					m_DlgPic.Render(&memdc,m_rtmin);
				
			}
			break;
		case 8:	
			{
				m_DlgPic.Load(IDR_MENUBK);
				if(m_DlgPic != NULL)
					m_DlgPic.Render(&memdc,m_rtvedio);
				
			}
			break;
		case 9:	
			{
				m_DlgPic.Load(IDR_MENUBK);
				if(m_DlgPic != NULL)
					m_DlgPic.Render(&memdc,m_rthtry);
				
			}
			break;
		case 10:	
			{
				m_DlgPic.Load(IDR_MENUBK);
				if(m_DlgPic != NULL)
					m_DlgPic.Render(&memdc,m_rttool);
				
			}
			break;
		case 11:	
			{
				m_DlgPic.Load(IDR_MENUBK);
				if(m_DlgPic != NULL)
					m_DlgPic.Render(&memdc,m_rthelp);
				
			}
			break;
		default:
			break;
			
			
		}
		
		font.CreatePointFont(100,"Verdana");
		CFont   *pOldfont = (CFont *)memdc.SelectObject(&font); 
		memdc.SetTextColor(RGB(255, 255, 255)); 

		if (g_Language)
		{
			m_strCaption[0] = GetStringLanguage("Main","Menu0","Medio");
			m_strCaption[1] = GetStringLanguage("Main","Menu1","History");
			m_strCaption[2] = GetStringLanguage("Main","Menu2","Tool");
			m_strCaption[3] = GetStringLanguage("Main","Menu3","Help");
		}
		else
		{
			m_strCaption[0] = GetStringLanguage("Main","Menu0","媒体");
			m_strCaption[1] = GetStringLanguage("Main","Menu1","历史");
			m_strCaption[2] = GetStringLanguage("Main","Menu2","工具");
			m_strCaption[3] = GetStringLanguage("Main","Menu3","帮助");
		}
		CString strVol;
		strVol.Format("%d",m_nVol);
		strVol=strVol+"%";
		memdc.DrawText(m_strCaption[0],m_rtvedio,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		memdc.DrawText(m_strCaption[1],m_rthtry,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		memdc.DrawText(m_strCaption[2],m_rttool,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		memdc.DrawText(m_strCaption[3],m_rthelp,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		memdc.DrawText(strVol,m_rtvolume,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		
		
	}
    pDC->BitBlt(0,0,m_rect.Width(),m_rect.Height(),&memdc,0,0,SRCCOPY);

	font.DeleteObject();
	bru.DeleteObject();             
	bmp.DeleteObject();
	pOldBmp->DeleteObject();
	memdc.DeleteDC();
	ReleaseDC(pDC);
//	return CDialog::OnEraseBkgnd(pDC);
	return TRUE;   //本函数已经绘好背景,不需要系统重绘背景.可以有效防窗口闪烁.
}



void CRiplayerDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
//	CRect rc(m_rect.left+m_rthelp.right,0,m_rect.right-88,42);
	CRect rtMenu[4];
	rtMenu[0] = m_rtvedio;
	rtMenu[1] = m_rthtry;
	rtMenu[2] = m_rttool;
	rtMenu[3] = m_rthelp;
	if (!m_bMax)
	{
		if(m_rc.PtInRect(point)) 
		{
			::PostMessage(this->m_hWnd,WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x, point.y));	//向系统发送HTCAPTION消息，让系统以为鼠标点在标题栏上
		}
	}
	for (int i = 0; i < 4; i++)
	{
		if (rtMenu[i].PtInRect(point))
		{
			m_nMenuDown = i+1;
			SetCapture();
			break;
		}
		else
		{
			m_nMenuDown = 0;
		}
	}
	if(m_rtvoice.PtInRect(point))
	{
		m_mute=true;
		SetCapture();
	}
	if(m_rtbk.PtInRect(point))
	{
		m_moveball=true;//标志鼠标按下滑块
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
		SetCapture();
	}
	if(m_rtplay.PtInRect(point))
	{
		m_play=true;
		SetCapture();
	}
	if(m_rtclose.PtInRect(point))
	{
		m_close=true;
		SetCapture();
	}
	if(m_rtmax.PtInRect(point))
	{
		m_max=true;
		SetCapture();
	}
	if(m_rtmin.PtInRect(point))
	{
		m_min=true;
		SetCapture();
	}
	if(m_rtpicture.PtInRect(point))
	{
		m_picture=true;
		SetCapture();
	}
	if(m_rtrecord.PtInRect(point))
	{
		m_record=true;
		SetCapture();
	}
	if(m_rtstop.PtInRect(point))
	{
		m_stop=true;
		SetCapture();
	}
	if(m_rcview.PtInRect(point))
	{
		m_bView=true;
		SetCapture();
	}


	CDialog::OnLButtonDown(nFlags, point);
}

/****** 非客户区消息处理 ************/
UINT CRiplayerDlg::OnNcHitTest(CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	if (this->IsZoomed())
	{
		return CDialog::OnNcHitTest(point);
	}
	
	GetWindowRect(&rect);  
	
	if (!m_bFullScreen)
	{
		if (point.x <= rect.left+3)
		{     
			return HTLEFT;  
		}
		else if(point.x >= rect.right-3)
		{
			return HTRIGHT;
		}
		else if(point.y <= rect.top+3)
		{
			return HTTOP;
		}
		else if(point.y >= rect.bottom-3)
		{
			return HTBOTTOM;
		}    
		else if(point.x <= rect.left+10 && point.y <= rect.top+10)
		{
			return HTTOPLEFT;
		}
		else if(point.x >= rect.right-10 && point.y <= rect.top+10)
		{
			return HTTOPRIGHT;
		}
		else if(point.x <= rect.left+10 && point.y >= rect.bottom-10)
		{
			return HTBOTTOMLEFT;
		}
		else if(point.x >= rect.right-10 && point.y >= rect.bottom-10)
		{
			return HTBOTTOMRIGHT;
		}
	}

    return HTCLIENT;//指示当前鼠标在客户区，将响应OnLButtonDown消息。
	//	return CDialog::OnNcHitTest(point);
}



void CRiplayerDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	if (m_point == point)
	{
		return;
	}
	m_point = point;

	rect[0] = m_rtplay;
	rect[1] = m_rtstop;
	rect[2] = m_rtpicture;
	rect[3] = m_rtrecord;
	rect[4] = m_rtclose;
	rect[5] = m_rtmax;
	rect[6] = m_rtmin;
	rect[7] = m_rtvedio;
	rect[8] = m_rthtry;
	rect[9] = m_rttool;
	rect[10] = m_rthelp;

	CRect recttemp;
	recttemp.SetRect(m_rtbk.left,m_rtball.bottom,m_rtbk.right,m_rtball.top);

	if (!m_bFullScreen)//全屏的时候隐藏
	{

		InvalidateRect(&m_rcLastMoveRect);
		for(int i=0; i<11; i++)
		{
			if(rect[i].PtInRect(point))      
			{  
				m_nCount = i+1;
				m_nInRect = i +1;
				m_bInRect[i] = true;
				m_rcLastMoveRect =rect[i];
				InvalidateRect(rect[i]);
				break;
				
			}
			
		}
	
		if (i==11)
		{
			m_nCount=0;
			for(i=0; i<12; i++)
			{
				if (m_bInRect[i])
				{
					InvalidateRect(&rect[i]);
				}
				m_bInRect[i] = false;
			}
			
		}



	
	}
	if(m_play)
	{
		if(!m_rtplay.PtInRect(point))	
		{
			ReleaseCapture();
			m_play=false;
		}
	}
	if(m_mute)
	{
		if(!m_rtvoice.PtInRect(point))	
		{
			ReleaseCapture();
			m_mute=false;
		}
	}
	if(m_max)
	{
		if(!m_rtmax.PtInRect(point))	
		{
			ReleaseCapture();
			m_max=false;
		}
	}
	if(m_picture)
	{
		if(!m_rtpicture.PtInRect(point))	
		{
			ReleaseCapture();
			m_picture=false;
		}
	}
	if(m_stop)
	{
		if(!m_rtstop.PtInRect(point))	
		{
			ReleaseCapture();
			m_stop=false;
		}
	}
	if(m_record)
	{
		if(!m_rtrecord.PtInRect(point))	
		{
			ReleaseCapture();
			m_record=false;
		}
	}
	if(m_bView)
	{
		if(!m_rcview.PtInRect(point))	
		{
			ReleaseCapture();
			m_bView=false;
		}
	}
	if(m_moveball)	//鼠标拖动滚动条控制音量 
	{
		if (point.x<m_rtbk.left||point.x>m_rtbk.right||point.y>m_rtball.bottom||point.y<m_rtball.top)
		{
			m_moveball=false;
			ReleaseCapture();
		}
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));	
		if(point.x-6< m_rtbk.left)//滑块移动到滚动条最左边
		{
			m_rtball.left=m_rtbk.left;
			m_rtball.right=m_rtbk.left+12;
			m_nVol=0;
			//	ReleaseCapture();
		}
		else if(point.x+6 > m_rtbk.right)//滑块移动到滚动条最右边
		{
			m_rtball.left=m_rtbk.right-12;
			m_rtball.right=m_rtbk.right;
			m_nVol=100;
			//	ReleaseCapture();
		}
		else
		{
			m_rtball.left=point.x-6;
			m_rtball.right=m_rtball.left+12;
			m_nVol=(point.x-6-m_rtbk.left)*100/(m_rtbk.Width()-12);
		}
		
		SetVolume(m_handle,m_nVol);
		
		m_strVolume.Format("%d",m_nVol);
		//::GetPrivateProfileString("Volume", "nVolume", strVolume, strVolume.GetBuffer(MAX_PATH),MAX_PATH, m_strInIName);   
		::WritePrivateProfileString("Volume", "nVolume", m_strVolume, m_strInIName);

		InvalidateRect(&m_rtball); 
		InvalidateRect(&recttemp);
		InvalidateRect(&m_rtvolume);
		
	}
	
    if (m_rtbk.PtInRect(point)||m_rtvoice.PtInRect(point))
    {
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
    }

	CDialog::OnMouseMove(nFlags, point);
}

/*********　设置最小可缩放值 **************/
void CRiplayerDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	// TODO: Add your message handler code here and/or call default
	
	lpMMI-> ptMinTrackSize.x=600; 
	lpMMI-> ptMinTrackSize.y=450; 
	MINMAXINFO *pmmi = (MINMAXINFO*)lpMMI; 
	lpMMI->ptMaxSize.x = GetSystemMetrics(SM_CXFULLSCREEN) + GetSystemMetrics(SM_CXDLGFRAME); //窗口最大化时不挡住任务栏
    lpMMI->ptMaxSize.y = GetSystemMetrics(SM_CYFULLSCREEN) + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYDLGFRAME); //窗口最大化时不挡住任务栏

	CDialog::OnGetMinMaxInfo(lpMMI);
}



void CRiplayerDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if (m_bFullScreen)
	{
		return;
	}
	ReleaseCapture();
	if(m_rtclose.PtInRect(point)&&m_close)
    {
		OnExit();
	}
	
	if(m_rtmax.PtInRect(point)&&m_max)
    {
		OnMax();
	}
	if(m_rtmin.PtInRect(point)&&m_min)
    {
		OnMin();
	}
	
	if(m_rtrecord.PtInRect(point)&&m_record)
    {
		if (!m_recording)
		{
			OnBtnRecord();
		}
		else
		{
			OnStoprecrod();
		}
	}
	if(m_rtpicture.PtInRect(point)&&m_picture)
    {
		if (m_display)
		{
			OnBtnPicture();
		}
	}
	if(m_rtstop.PtInRect(point)&&m_stop)
    {
		if (m_display)
		{
			OnBtnStop();
		}
	}
	if(m_rcview.PtInRect(point)&&m_stop)
    {
		if (!m_display)
		{
			OnOpen();
		}
	}
	/************ 播放按钮 ***********/
	if(m_rtplay.PtInRect(point)&&m_play)
	{

		if(m_pause)//是否暂停
		{	
			OnBtnPause();
			
		}
		else
		{

			OnBtnPlay();
		}	


	}
	if(m_mute)
	{
		if(m_rtvoice.PtInRect(point))      
		{ 
			if(m_voice)//0设置静音，非0取消静音
			{	
				SetMute(m_handle, 0);// 为0表示取消静音；
				InvalidateRect(m_rtvoice);//重绘矩形区域
				m_voice=false;
			}
			else
			{				
				SetMute(m_handle, 1);// 非0表示设置静音
				InvalidateRect(m_rtvoice);//重绘矩形区域
				m_voice=true;
			}
			
		}
	}
	
	/******* 媒体 **********/
	if(m_rtvedio.PtInRect(point))
	{
		CRect rtcvert;
		rtcvert = m_rtvedio;
		ClientToScreen(rtcvert);
		m_pMediaMenu-> TrackPopupMenu(TPM_LEFTBUTTON ,rtcvert.left,rtcvert.bottom,this);
		
	}
	/******* 历史 **********/
	if(m_rthtry.PtInRect(point))
	{
		CRect rtcvert;
		rtcvert =m_rthtry;
		ClientToScreen(rtcvert);
		
		m_hHistoryMenu->TrackPopupMenu(TPM_LEFTBUTTON ,rtcvert.left,rtcvert.bottom,this);
		
	}
	
	/******* 工具 **********/
	if(m_rttool.PtInRect(point))
	{
		CRect rtcvert;
		rtcvert=m_rttool;
		if (m_display)
		{
			m_pToolMenu->EnableMenuItem(IDC_BTN_PICTURE,MF_ENABLED|MF_BYCOMMAND);
			
			if(m_recording)
			{
				m_pToolMenu->EnableMenuItem(IDC_BTN_RECORD,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
			}
			else
			{
				m_pToolMenu->EnableMenuItem(IDC_BTN_RECORD,MF_ENABLED|MF_BYCOMMAND);
			}
			m_pToolMenu->EnableMenuItem(1,MF_ENABLED|MF_BYPOSITION);
		}
		else
		{
			m_pToolMenu->EnableMenuItem(IDC_BTN_PICTURE,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
			m_pToolMenu->EnableMenuItem(IDC_BTN_RECORD,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
			m_pToolMenu->EnableMenuItem(1,MF_DISABLED|MF_GRAYED|MF_BYPOSITION);
		}
		
		ClientToScreen(rtcvert);
		m_pToolMenu-> TrackPopupMenu   (TPM_LEFTBUTTON ,rtcvert.left,rtcvert.bottom,this);
		
	}
	
	/******* 帮助 **********/
	if(m_rthelp.PtInRect(point))
	{
		CRect rtcvert;
		rtcvert=m_rthelp;
		ClientToScreen(rtcvert);
		m_pHelpMenu->EnableMenuItem(1,MF_DISABLED|MF_GRAYED|MF_BYPOSITION);
		m_pHelpMenu-> TrackPopupMenu   (TPM_LEFTBUTTON ,rtcvert.left,rtcvert.bottom,this);
		
		
	}
	/***********   音量滑块控件 **********/
    if(m_rtbk.PtInRect(point)&&m_moveball)
	{	
		m_voice=false;	//音量正常
		InvalidateRect(m_rtvoice);//重绘矩形区域
		SetMute(m_handle,0);//取消静音 
		
		m_moveball=false;
		rctemp=point.x-m_rtbk.left-6;//滑块的位移
		m_rtball.left=point.x-6;
		m_rtball.right=m_rtball.left+12;
		if(point.x-6< m_rtbk.left)//滑块在最左边
		{
			m_rtball.left=m_rtbk.left;
			m_rtball.right=m_rtbk.left+12;
		
			
			
		}
		if(point.x+6 > m_rtbk.right)//滑块在最右边
		{
			m_rtball.left=m_rtbk.right-12;
			m_rtball.right=m_rtbk.right;
		
		}
		m_nVol=rctemp*100/(m_rtbk.Width()-12);
		
		if(point.x-6-m_rtbk.left<0)//滑块移动到滚动条最左边
		{
			m_nVol=0;
		}
		else
		{
			m_nVol=(point.x-6-m_rtbk.left)*100/(m_rtbk.Width()-12);
		}
		if (m_nVol>100)
		{
			m_nVol=100;
		}
		SetVolume(m_handle,m_nVol);
		m_strVolume.Format("%d",m_nVol); 
		::WritePrivateProfileString("Volume", "nVolume", m_strVolume, m_strInIName);
		PRINTF("OnLButtonUp:vol==%d\n",m_nVol);

		InvalidateRect(m_rtball);
		InvalidateRect(CRect(m_rtbk.left,m_rtball.bottom,m_rtbk.right,m_rtball.top));
		InvalidateRect(&m_rtvolume);
		
	}
	else
	{
		ReleaseCapture();
		m_moveball=false;
	}
	CDialog::OnLButtonUp(nFlags, point);
}

void CRiplayerDlg::OnMax() 
{
	// TODO: Add your control notification handler code here
	
	if(m_bMax)//窗口最大化时恢复正常
	{
		this->ShowWindow(SW_SHOWNORMAL); 
		m_normal=false;
		m_bMax = FALSE;
		InvalidateRect(m_rtmax);
	}
	else 
	{
		m_normal=true;
	
		m_bMax = TRUE;
		this->ShowWindow(SW_SHOWMAXIMIZED);//窗口正常则最大化 
		InvalidateRect(m_rtmax);
		
	}
	InvalidateRect(m_rect);// 重绘窗口
	
}


void CRiplayerDlg::OnMin() 
{
	// TODO: Add your control notification handler code here
	GetClientRect(m_rect);
	WINDOWPLACEMENT lwndpl;
	WINDOWPLACEMENT * lpwndpl;
	lpwndpl=&lwndpl;
	GetWindowPlacement(lpwndpl);
	lpwndpl->showCmd=SW_SHOWMINIMIZED;
	SetWindowPlacement(lpwndpl);
}

/************* 处理系统窗口缩放机制  ****************/
void CRiplayerDlg::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &m_bDragFullWindow, NULL);   
	//如果需要修改设置，则在每次进入CDialog::OnNcLButtonDown默认处理之前修改
    if(m_bDragFullWindow)
	{
		SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, FALSE, NULL, NULL);
		m_normal=false;
		InvalidateRect(m_rtmax);//重绘最大化按钮
	}
    //默认处理，系统会自动绘制虚框
	CDialog::OnNcLButtonDown(nHitTest, point);    	
	//默认处理完毕后，还原系统设置
    if(m_bDragFullWindow)
    {
		SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, TRUE, NULL, NULL);
	}
	InvalidateRect(m_rect);
}



void CRiplayerDlg::OnExit() 
{
	// TODO: Add your command handler code here
	if (m_recording)
	{
		OnStoprecrod(); 
	}

	CRect rcTrayWnd;
	CWnd* pTaskWnd = CWnd::FindWindow("Shell_TrayWnd","");//得到任务栏主窗口指针
	pTaskWnd->GetWindowRect(&rcTrayWnd); 
	pTaskWnd->ShowWindow(SW_SHOW);
	CDialog::OnCancel();
	close_debug_file();
}

static bool record_start=false;
static int samplerate = 0;

/**** RTP视频回调 **********/
void CRiplayerDlg::GetVideoCallBack(unsigned char *buff,int buflen,Frame_Info* media_info)
{
	
	encoder_handle->total_byte_cnt += buflen;
	encoder_handle->total_frame_cnt++;
	encoder_handle->lost_frame_cnt = g_lost_frame_cnt;
	
	
	//g_total_byte_cnt=0;
//	g_total_frame_cnt=0;
	
	if(prv_nWidth != media_info->width)
	{
		prv_nWidth = media_info->width;
		if( m_recording ){
			if(true == record_status)
				record_status=false;
		}
	}
	if (prv_nHieght != media_info->high)
	{
		prv_nHieght = media_info->high;
	}

//	g_total_byte_cnt += buflen;
//	g_total_frame_cnt++;
	SetVideoData(m_handle, buff, buflen, prv_nWidth, prv_nHieght);
	
	
	//add mp4 record
	if(m_recording)
	{
		unsigned int tim  = 0;
		CTime time=CTime::GetCurrentTime();
		static unsigned int timetick = 0;
		record_info_t info;
		int i = 0;
		
		timetick= GetTickCount();
		tim = time.GetTime();
		
		memset(&info,0,sizeof(record_info_t));
		info.type = TYPE_VIDEO;
		info.width = prv_nWidth;
		info.height = prv_nHieght;
		info.iframe = 0;
		info.timetick = timetick;
		info.samplerate = samplerate;
		if(r_handle == NULL ){
			if(media_info->I_frame )	
				r_handle = record_open(filename, 2, &info);					
		}
		if(r_handle != NULL )
		{	
			for(int i =0; i<10; i++)
			{
				if(buff[4] ==0x67)
				{
					info.iframe = 1;
				}
			}
//加锁      
			WaitForSingleObject(hMutex,INFINITE);
			record_s += buflen;
			record_push_data(r_handle,  (char *)buff,buflen,&info);	
//解锁
			ReleaseMutex(hMutex);
			if(record_status == false )
				StopRecrod();
			
			if( record_s > 0x73333333 ){//0x73333333 = 1.8G 
				m_staiicthis->SendMessage(WM_COMMAND,IDM_STOPRECROD,0);
				m_staiicthis->SendMessage(WM_COMMAND,IDC_BTN_RECORD,0);
			}
			
		}
		
	}
		
}

/*************  RTP音频回调函数 *************/
void CRiplayerDlg::GetAudioCallBack(uint8_t *pBuf,int32_t iBufLen, Frame_Info* media_info)
{	
	samplerate = AUDIO_SAMPLE[media_info->sample_rate]*2 ;
	g_Samplerate = samplerate;
	encoder_handle->total_byte_cnt += iBufLen;
	SetAudioData(m_handle, pBuf ,iBufLen, samplerate);
		
	if(m_recording)
	{
		static int timetick=0;
		timetick= GetTickCount();
		record_info_t info;
		memset(&info,0,sizeof(record_info_t));
		info.type = TYPE_AUDIO;
		info.samplerate = samplerate;
		info.iframe = 0;
		info.timetick = timetick;
		
	
		if(r_handle != NULL )
		{			
			//加锁
		
			WaitForSingleObject(hMutex,INFINITE);
			record_s += iBufLen;
			record_push_data(r_handle,  (char *)pBuf,iBufLen,&info);
			ReleaseMutex(hMutex);
			//解锁
			//	if(record_status == false)
			//	StopRecrod();
		}
		if(record_status == false)
				StopRecrod();
		
		if( record_s > 0x73333333){
			m_staiicthis->SendMessage(WM_COMMAND,IDM_STOPRECROD,0);
			m_staiicthis->SendMessage(WM_COMMAND,IDC_BTN_RECORD,0);
			}
	}
}

/******* 播放*********/
int CRiplayerDlg::OnPlay(CString strURL) 
{

	if(play_info.flag != 0)
	{
		OnBtnStop();
		Sleep(100);
	}
	open_debug_file();
	
	char szUrl[128] = {0};
	sprintf(szUrl, "%s", strURL);
	
	if(play_info.flag == 2)
	{
		play_info.protocol = RTP_STREAM;
		play_info.flag = 0;
	}
	prv_nHieght = 0;
	prv_nWidth = 0;
	g_Lost_frame = 0;  // 丢失帧率
	g_Total_frame =0; //视频总帧数
	g_Frame_rate =0; //帧率
	g_Bit_rate =0; //位速率
	g_Samplerate=0;
	
	
	lost_frame_flag = true;
	if(NULL == encoder_handle){
		encoder_handle = open_Encoder(szUrl);
		PRINTF("open_Encoder\n");
		}
	
	if (play_info.protocol <= 0)
	{
		play_info.protocol = parse_url(szUrl, play_info.ip, &(play_info.port),&(play_info.multicast));
		if( play_info.protocol < 0){
			PRINTF("OnPlay:parse_url fail!\n");
			CMsgBoxDlg dlg;
			dlg.SetMsg(GetStringLanguage("MsgBox", "Caption3", "错误"), GetStringLanguage("MsgBox", "Tip5", "您的输入无法被打开"));
			dlg.DoModal();
			return -1;
		}
	}

	CRect rect_view;
	rect_view.top=0;
	rect_view.bottom=0;
	rect_view.left=0;
	rect_view.right=0;
	GetDlgItem(IDC_STATIC_VIEW)->ShowWindow(SW_SHOW);
	InvalidateRect(m_rtplay);//重绘矩形区域

	if (m_handle ==NULL)
	{
		m_handle= Init();//初始化视频窗口
	
	}
	
	HWND hwnd=NULL;
	hwnd = GetDlgItem(IDC_STATIC_VIEW)->m_hWnd;;//获取视频显示框句柄
	if (hwnd==NULL)
	{
		return 0;
	}
	SetShowRect(m_handle,  hwnd, rect_view);//设置视频显示窗口及位置
	
	
	if( RTP_STREAM ==  play_info.protocol )
	{
 
		sdp_info = parse_sdp(sdp_pBuf,sdp_FileLen+1);
		if(sdp_info == NULL)
		{
				play_info.protocol = -1;
		}else {
	
		if( rtp_handle == NULL)
		{
			if(0 != sdp_info->video_port){
				rtp_handle =  rtp_video_open_url(sdp_info);//连接RTP接口
				if(rtp_handle == NULL)
					play_info.protocol = -1;
					
				}
			if (rtp_handle != NULL)
			{
				rtp_set_output_func(rtp_handle,&GetVideoCallBack); //调用RTP回调函数接收数据
				rtp_set_msg_func(rtp_handle,&cb_msg_parse);
			}
		}

		if (rtp_audio_handle == NULL)
		{
			if( 0 != sdp_info->audio_port ){
				rtp_audio_handle = rtp_audio_open_url(sdp_info);
				if(rtp_audio_handle == NULL)
					play_info.protocol = -1;
				}
			if (rtp_audio_handle != NULL)
			{
				rtp_set_output_func(rtp_audio_handle,&GetAudioCallBack);
				rtp_set_msg_func(rtp_audio_handle,&cb_msg_parse);
			}				
		}
		}
	}
	else if(TS_STREAM == play_info.protocol)
	{

		if( ts_handle == NULL)
		{			
			ts_handle = TS_open_url(play_info.ip,play_info.port);
			if (ts_handle!=NULL)
			{
				ts_set_videoes_cb(ts_handle, &GetVideoCallBack);
				ts_set_ccevent_cb(ts_handle, &cb_msg_parse);
				ts_set_audioes_cb(ts_handle,&GetAudioCallBack);
			}
			else
			{
				play_info.protocol = -1;
			}
		}
	}

	else if( RTP_TS_STREAM == play_info.protocol)
	{
		if( ts_rtp_handle == NULL)
		{
			ts_rtp_handle = TS_RTP_open_url(play_info.ip,play_info.port,play_info.multicast);
			if (ts_rtp_handle!=NULL){
				ts_set_videoes_cb(ts_rtp_handle, &GetVideoCallBack);
				ts_set_audioes_cb(ts_rtp_handle,&GetAudioCallBack);			
				ts_set_ccevent_cb(ts_rtp_handle,&cb_msg_parse);
			}
			else 
			{
				play_info.protocol = -1;
			}
		}
	}else if(RTSP_STREAM == play_info.protocol){
		if(rtsp_url[0] == 0 || (0 == strncmp(szUrl,"rtsp",4))){
			strcpy(rtsp_url,szUrl);
		}
		
		if(rtsp_handle == NULL){
			rtsp_handle = rtsp_open_stream(rtsp_url);
			if(rtsp_handle){
				rtsp_set_audio_output_func( rtsp_handle,GetAudioCallBack);
				rtsp_set_video_output_func(rtsp_handle,GetVideoCallBack);						
				rtsp_set_msg_output_func(rtsp_handle,&cb_msg_parse);
			}else {
				play_info.protocol = -1;
				/* RTSP 输出正常不进入stop流程*/
				play_info.flag=0;
				prev_play_info.protocol = RTSP_STREAM;
				}
			}		
	}else {
		play_info.protocol = -1;
		
	}

	
	if (-1 == play_info.protocol)
	{
		PRINTF("OnPlay:onplay fail!\n");
		play_info.flag=0;
		CMsgBoxDlg dlg;
		dlg.SetMsg(GetStringLanguage("MsgBox", "Caption3", "错误"), GetStringLanguage("MsgBox", "Tip5", "您的输入无法被打开"));
		dlg.DoModal();
		return -1;
	}
	else
	{
		PRINTF("OnPlay:onplay success!\n");
		play_info.flag=1;
		if(encoder_handle){
			encoder_info_set_cbfunc(encoder_handle,GetEncoderCallBack);
		}
	}
	
	m_display = true;	
	m_pause = true;
	nStatus = 1; 
	::GetPrivateProfileString("Volume", "nVolume", "100", m_strVolume.GetBuffer(MAX_PATH),MAX_PATH, m_strInIName);  
	m_nVol=atoi(m_strVolume)/*GetVolume(m_handle)*/; //初始化音量
	SetVolume(m_handle,m_nVol);
	rctemp=m_nVol*(m_rtbk.Width()-12-1)/100;//滑块位移
	m_rtball.left=m_rtbk.left+rctemp;
	m_rtball.right=m_rtbk.left+rctemp+14;
	
	m_voice=false;
	SetMute(m_handle, 0);// 为0表示取消静音；
	InvalidateRect(m_rtvoice);//重绘矩形区域			
	InvalidateRect(m_rtball);
	InvalidateRect(&m_rtdisplay);
	InvalidateRect(CRect(m_rtbk.left,m_rtball.top,m_rtbk.right,m_rtball.bottom));
	InvalidateRect(m_rtvolume);
	
//	SetScale(m_handle, 0);//设置显示区域
    return 1;
	
}

void CRiplayerDlg::OnBtnPlay() 
{
	int ret=0;
	// TODO: Add your command handler code here
	if(RTP_STREAM==prev_play_info.protocol)
	{
		play_info.flag=2;
	}else if(TS_STREAM == prev_play_info.protocol 
	|| RTP_TS_STREAM == prev_play_info.protocol 
	){
		play_info.protocol = prev_play_info.protocol;
		play_info.port = prev_play_info.port;
		strncpy(play_info.ip,prev_play_info.ip,16);
		play_info.multicast = prev_play_info.multicast;
		play_info.flag=0;
	}else if( RTSP_STREAM ==  prev_play_info.protocol ){
		play_info.protocol = 0 ;
		play_info.flag=0;
	}
	if(m_CurNum)
	{
		if(RTSP_STREAM ==  prev_play_info.protocol )
			ret = OnPlay(g_strUrl);
		else 
			ret = OnPlay("4");
		if( ret != -1)
			Tipmsg(g_strUrl);
	}
}

void CRiplayerDlg::StopPlay()
{
	m_pause = false;
	m_display=false;
	m_play = false;
	nStatus = 0; 
	if( m_recording )
		OnStoprecrod();	
	
	m_display=false;
	m_nVol = 0;
	m_rtball.left = m_rtbk.left;
	m_rtball.right = m_rtbk.left +14;
	if (play_info.protocol == RTP_STREAM)
		{
			if(rtp_audio_handle != NULL){
				rtp_close_stream(&rtp_audio_handle);
				rtp_audio_handle = NULL;
				Sleep(50);
				}
			if(rtp_handle != NULL){
				rtp_close_stream(&rtp_handle);
				rtp_handle=NULL;
				Sleep(50);
				}
			PRINTF("OnBtnStop:rtp stream stop!\n");
		}
		else if(play_info.protocol == TS_STREAM)
		{
			if( ts_handle != NULL){
				TS_close_stream(&ts_handle);
				ts_handle =NULL;
				Sleep(50);
				}
			PRINTF("OnBtnStop:ts stream stop!\n");
		}else if(RTP_TS_STREAM == play_info.protocol){
			if(ts_rtp_handle != NULL){
				TS_close_stream(&ts_rtp_handle);	
				ts_rtp_handle =NULL;
				Sleep(50);
				}
			PRINTF("OnBtnStop:rtp over ts stream stop!\n");
		}else if( RTSP_STREAM == play_info.protocol){
			if( rtsp_handle != NULL){
				rtsp_close_stream(rtsp_handle);
				rtsp_handle =NULL;
				Sleep(50);
				}
			PRINTF("OnBtnStop:rtsp stream stop!\n");
		}

		if(play_info.protocol>0 ){
			prev_play_info.protocol = play_info.protocol;
			prev_play_info.port = play_info.port;
			prev_play_info.multicast = play_info.multicast;
			strncpy(prev_play_info.ip,play_info.ip,16);
			play_info.protocol=0;
		}
		if(play_info.flag == 1)
			play_info.flag=0;
		
	if( m_handle )
	{
		PRINTF("before UnInit!\n");
		UnInit(m_handle);
	}
	PRINTF("after UnInit!\n");
	if(m_handle!=NULL)
	{
		m_handle =NULL;
	}
	
	
	InvalidateRect(m_rect);
}

/**********  暂停 ***************/
void CRiplayerDlg::OnBtnPause() 
{
	// TODO: Add your command handler code here

	if (nStatus != 1)
	{
		return;
	} 
	else
	{
		//StopPlay();
		PausePlay();
	}	
		
}
void CRiplayerDlg::PausePlay()
{
	m_pause = false;
	m_display=false;
	m_play = false;
	nStatus = 0; 
	if( m_recording )
		OnStoprecrod();	
	
	m_display=false;
	m_nVol = 0;
	m_rtball.left = m_rtbk.left;
	m_rtball.right = m_rtbk.left +14;
	if (play_info.protocol == RTP_STREAM)
	{
		if(rtp_audio_handle != NULL){
			rtp_close_stream(&rtp_audio_handle);
			rtp_audio_handle = NULL;
			Sleep(50);
		}
		if(rtp_handle != NULL){
			rtp_close_stream(&rtp_handle);
			rtp_handle=NULL;
			Sleep(50);
		}
		PRINTF("OnBtnStop:rtp stream stop!\n");
	}
	else if(play_info.protocol == TS_STREAM)
	{
		if( ts_handle != NULL){
			TS_close_stream(&ts_handle);
			ts_handle =NULL;
			Sleep(50);
		}
		PRINTF("OnBtnStop:ts stream stop!\n");
	}else if(RTP_TS_STREAM == play_info.protocol){
		if(ts_rtp_handle != NULL){
			TS_close_stream(&ts_rtp_handle);	
			ts_rtp_handle =NULL;
			Sleep(50);
		}
		PRINTF("OnBtnStop:rtp over ts stream stop!\n");
	}else if( RTSP_STREAM == play_info.protocol){
		if( rtsp_handle != NULL){
			rtsp_close_stream(rtsp_handle);
			rtsp_handle =NULL;
			Sleep(50);
		}
		PRINTF("OnBtnStop:rtsp stream stop!\n");
	}
	
	if(play_info.protocol>0 ){
		prev_play_info.protocol = play_info.protocol;
		prev_play_info.port = play_info.port;
		prev_play_info.multicast = play_info.multicast;
		strncpy(prev_play_info.ip,play_info.ip,16);
		play_info.protocol=0;
	}
	if(play_info.flag == 1)
		play_info.flag=0;
	
	if( m_handle )
	{
		PRINTF("before UnInit!\n");
		UnInit(m_handle);
	}
	PRINTF("after UnInit!\n");
	if(m_handle!=NULL)
	{
		m_handle =NULL;
	}
	
	InvalidateRect(&m_rtplay);
	//InvalidateRect(m_rect);
}


void CRiplayerDlg::OnBtnStop() 
{
	// TODO: Add your command handler code here
	if (nStatus != 1)
	{
		return;
	} 
	else
	{
		StopPlay();
	}

}
void CRiplayerDlg::OnBtnRecord() 
{
	// TODO: Add your command handler code here	
	if (m_display&&(prv_nHieght>0)&&(prv_nWidth>0))
	{
		CString strPathName,str,strTemp;
		str=GetStringLanguage("Record","Static2","录制文件将保存到");
		CTime time=CTime::GetCurrentTime();
		CString strTime=time.Format("%m-%d %H.%M.%M");	//时间格式
		sprintf(filename,strTime);
		sprintf(filename,"%s.MP4",filename);
		
	
		int nWidth = 0;
		int nHeight = 0;
		int buflen = 0;
		CRecordDlg dlg;
		dlg.ReadMru();
		strTemp=dlg.m_PsPathName;
		if (GetFileAttributes(strTemp) == 0xFFFFFFFF)//判断路径是否存在
		{
			CMsgBoxDlg dlg;
			dlg.SetMsg(GetStringLanguage("MsgBox", "Caption3", "错误"), GetStringLanguage("MsgBox", "Tip12", "路径不存在"));
			dlg.DoModal();
			return;
		}
			
		strPathName.Format("%s\\%s", strTemp, filename);
		sprintf(filename, "%s", strPathName);
		if (!m_recording)
		{
			m_recording=true;	
			record_status = true;
			strTemp.Format("%s %s", str,strTemp);
			Tipmsg(strTemp);
			FILE *fp = fopen(filename, "a+");
			if(fp)
			{
				fclose(fp);
			}
			else
			{
				CMsgBoxDlg dlg;
				dlg.SetMsg(GetStringLanguage("MsgBox", "Caption2", "提示"), GetStringLanguage("MsgBox", "Tip13", "文件创建失败!"));
				dlg.DoModal();
				record_status=false;
				m_recording=false;
				return;

			}
			hMutex = CreateMutex(NULL,FALSE,NULL);// 创建一个匿名的互斥对象
		
			m_pToolMenu->GetSubMenu(6)->EnableMenuItem(IDM_SETR,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
		
		}
		
	
		InvalidateRect(&m_rtrecord);		
	}
	record_s = 0 ;// time.GetTime()+60*60;
	PRINTF("OnBtnRecord:start record!\n");
}
static void StopRecrod(void)
{

	m_recording=false;
	record_s = 0 ;
	unsigned timetick = GetTickCount();
	if(r_handle != NULL)
	{
		record_close(&r_handle,filename,timetick);
		r_handle = NULL;
		
	}
	PRINTF("StopRecrod:stop record!\n");

}

/**** 停止录制 ********/
void CRiplayerDlg::OnStoprecrod() 
{
	// TODO: Add your command handler code here
	CRecordDlg dlg;
	CString strPathName,str;
	str=GetStringLanguage("Record","Static3","录制文件已保存到:");
	dlg.ReadMru();
	strPathName=dlg.m_PsPathName;
	m_recording=false;
	record_s = 0 ;
	
	unsigned timetick = GetTickCount();
	if(r_handle != NULL)
	{
		record_close(&r_handle,filename,timetick);
		r_handle = NULL;
		strPathName.Format("%s %s", str,strPathName);
		Tipmsg(strPathName);
	}
	m_pToolMenu->GetSubMenu(6)->EnableMenuItem(IDM_SETR,MF_ENABLED|MF_BYCOMMAND);
	InvalidateRect(&m_rtrecord);
	PRINTF("OnStoprecrod:stop record!\n");

}

HBRUSH CRiplayerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if(nCtlColor == CTLCOLOR_STATIC) 
	{ 
		//IDC_STATIC为你想要设为透明的控件ID 
		if(pWnd->GetDlgCtrlID()== IDC_STATIC_VIEW)
        {
			pDC->SetBkMode(TRANSPARENT);   
			return   (HBRUSH)GetStockObject(NULL_BRUSH);
        }
		
	} 
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

/********** 全屏*************/
void CRiplayerDlg::OnFullscreen() 
{
	// TODO: Add your command handler code here

	GetClientRect(m_rcWind);    //获取全屏前对话框的客户区坐标
	GetDlgItem(IDC_STATIC_VIEW)->GetWindowRect(m_rectc);//获取全屏前播放区的客户区坐标
	if(m_display)
	{
		if(!m_bFullScreen)
		{
			m_bFullScreen = true;
			SetWindowLong(m_hWnd, GWL_STYLE,GetWindowLong(m_hWnd, GWL_STYLE) & (~(WS_CAPTION | WS_BORDER|WS_THICKFRAME|WS_SIZEBOX)));
			CWnd* pTaskWnd = CWnd::FindWindow("Shell_TrayWnd","");//得到任务栏主窗口指针   
			pTaskWnd->ShowWindow(SW_HIDE);
			SetForegroundWindow(); //总在最前
			SetWindowPos(&wndTopMost, 0, -1, m_nScreenCx + 2, m_nScreenCy + 3, SWP_NOZORDER);
			GetDlgItem(IDC_STATIC_VIEW)->MoveWindow(0, 0, m_nScreenCx , m_nScreenCy );//MoveWindow相对于屏幕移位
			InvalidateRect(&CRect(0 ,0, m_nScreenCx, m_nScreenCy));
		}
	}
}


void CRiplayerDlg::OnNormal()
{
	CRect rcTrayWnd;
	if(m_bFullScreen)
	{
		m_bFullScreen=false;
		
		ClientToScreen(m_rectc);//VIEW屏幕坐标转换成客户区坐标
		ClientToScreen(m_rcWind);

		CWnd* pTaskWnd = CWnd::FindWindow("Shell_TrayWnd","");//得到任务栏主窗口指针
		pTaskWnd->GetWindowRect(&rcTrayWnd); 
		pTaskWnd->ShowWindow(SW_SHOW);
		//	SetForegroundWindow();
		SetWindowPos(&wndNoTopMost ,(m_nScreenCx - m_rcWind.Width())/2,(m_nScreenCy - rcTrayWnd.Height()-m_rcWind.Height())/2,m_rcWind.Width(),m_rcWind.Height(),SWP_NOZORDER);
		
		GetDlgItem(IDC_STATIC_VIEW)->SetWindowPos(NULL,6,28,m_rectc.Width(),m_rectc.Height(),SWP_NOZORDER);//SetWindowPos相对于客户区移位
		InvalidateRect(m_rcWind);
	}
}

/***********  原始比例显示 ***********/
void CRiplayerDlg::OnOriginal() 
{
	// TODO: Add your command handler code here

	m_normal=false;
	m_bMax = FALSE;
	InvalidateRect(m_rtmax);
	if (m_display&&prv_nHieght&&prv_nWidth)
	{
		if ((prv_nWidth >= m_nScreenCx) && (prv_nHieght >= m_nScreenCy))
		{
			OnLarge();
		}
		else if ((prv_nWidth < m_nScreenCx) && (prv_nHieght >= m_nScreenCy))
		{
			int cy1 = GetSystemMetrics(SM_CYFULLSCREEN) + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYDLGFRAME);
			MoveWindow(0, 0, prv_nWidth, cy1);	
			InvalidateRect(CRect(0, 0, prv_nWidth, cy1));	
		}
		else if ((prv_nWidth >= m_nScreenCx) && (prv_nHieght < m_nScreenCy))
		{
		
			int cx1 = GetSystemMetrics(SM_CXFULLSCREEN) + GetSystemMetrics(SM_CXDLGFRAME); 
			MoveWindow(0, 0, cx1, prv_nHieght);	
			InvalidateRect(CRect(0, 0, cx1, prv_nHieght));
		}
		else if ((prv_nWidth <= MIN_WIDTH) && (prv_nHieght <= prv_nHieght))
		{
			OnSmall();
		}
		else
		{
			MoveWindow(0, 0, prv_nWidth, prv_nHieght);	
			InvalidateRect(CRect(0, 0, prv_nWidth, prv_nHieght));
		}
	
	}
	

}

void CRiplayerDlg::OnSmall() 
{
	// TODO: Add your command handler code here

	m_normal=false;
	m_bMax = FALSE;
	MoveWindow(0, 0, MIN_WIDTH, MIN_HEIGHT);
	InvalidateRect(m_rtmax);	
	InvalidateRect(CRect(0, 0, MIN_WIDTH, MIN_HEIGHT));
	
}

void CRiplayerDlg::OnLarge() 
{
	// TODO: Add your command handler code here

	int cx1 = GetSystemMetrics(SM_CXFULLSCREEN) + GetSystemMetrics(SM_CXDLGFRAME); 
    int cy1 = GetSystemMetrics(SM_CYFULLSCREEN) + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYDLGFRAME); //窗口最大化时不挡住任务栏
	MoveWindow(0,0,cx1,cy1);	
	InvalidateRect(CRect(0,0,cx1,cy1));
}

BOOL CRiplayerDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class

	if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN) // 屏蔽掉回车
	{
		return NULL;
	}

	if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_ESCAPE) //ESC键还原
	{
		OnNormal();
		return NULL;
	}
	if(pMsg->message==WM_RBUTTONUP) //截获WM_RBUTTONUP消息
	{
		OnRightMenu();
		return NULL;
	}



	return CDialog::PreTranslateMessage(pMsg);
}

void CRiplayerDlg::OnLicense() 
{
	// TODO: Add your command handler code here
			
				
}

void CRiplayerDlg::Tipmsg(CString str)
{
	CRect rect,rect1;
	GetWindowRect(rect);
	rect1.SetRect(rect.left+rect.Width()/2-250,rect.bottom-80,rect.left+rect.Width()/2+250,rect.bottom-40);	
	dlgTip->OnTip(str);
	dlgTip->MoveWindow(rect1,TRUE);
	dlgTip->ShowWindow(SW_SHOW);
	
}

/*******清除历史菜单*********/
void CRiplayerDlg::OnClearmenu() 
{
	// TODO: Add your command handler code here
	ClearMru();
}





BOOL CRiplayerDlg::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
	int n = m_hHistoryMenu->GetMenuItemCount() - 2;//获取历史菜单子项个数，除"清除"子菜单外
	if (0 == nCode)//判断是否是命令消息
	{
		//判断当前单击的菜单ID是否在动态菜单项的范围之内
		if (m_hHistoryMenu->GetMenuItemID(0) <= nID
			&& nID<=m_hHistoryMenu->GetMenuItemID(n))
		{
			if (pHandlerInfo==NULL )
			{	
				CString strMenuName;  //菜单项名		     	   
				m_hMruMenu.GetMenuString(nID,strMenuName,MF_STRING);//根据ID得到菜单项名
		
				if (strMenuName != GetStringLanguage("Main","Menu10","清除"))
				{
					Handler(strMenuName); //菜单处理函数
				}
			}
			return true;
		}
	}
	
	return CDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

/*********  响应历史菜单子菜单项 *************/
void CRiplayerDlg::Handler(CString strMenuName)
{
	bool bFile=false;
	int i=0,j=0;
	char temp[10];
	char name[256];
	CString strFileName;
	memset(name,0,256);
	sprintf(name,"%s",strMenuName);

	while(name[i] != '\0'){
		if( name[i] == '.')
		{
			for(j=0;name[i+1+j] != '\0' && j <4;j++)
			{
				temp[j]=name[i+j+1];
			}
			if(0 ==  strncmp(temp,"sdp",3))
			{
				bFile = true;
				break;
			}
		}
		i++;
	}
	if (bFile)
	{
		CFile file;
		
		if (file.Open(name,  CFile::modeRead))
		{
			play_info.flag = 2;
			int index = strMenuName.ReverseFind('\\');
			strFileName = strMenuName.Mid(index+1);
			sdp_FileLen = file.GetLength();
			if(sdp_FileLen >=1024)
			{
				sdp_FileLen =1024;
			}
			file.Read(sdp_pBuf,sdp_FileLen);
			sdp_pBuf[sdp_FileLen+1] = 0xa;
			file.Close();
			PRINTF("Handler:len =%d\n",sdp_FileLen );
			
		}
		else
		{	
			CMsgBoxDlg dlg;
			dlg.SetMsg(GetStringLanguage("MsgBox", "Caption3", "错误"), GetStringLanguage("MsgBox", "Tip10", "文件无法被打开"));
			dlg.DoModal();	
			return;
		}
	}
	int ret = 	OnPlay(strMenuName);
	if (ret!=-1)
	{
		if (bFile)
		{
			g_strUrl=strFileName;
		} 
		else
		{
			g_strUrl=strMenuName;
		}
		Tipmsg(g_strUrl);

	}



}


/********** 双击静态视频显示框 *********/
void CRiplayerDlg::OnStaticDblClick()
{


	if (m_display)
	{
		if (!m_bFullScreen)
		{
			OnFullscreen();	
			
		}
		else
		{		
			OnNormal();
			
		}
	}
	
}

void CRiplayerDlg::OnStaticClick()
{
    
	Display();	
}

void CRiplayerDlg::OnChinese() 
{

	g_Language=0;

	GetConfig();

}

void CRiplayerDlg::OnEnglish() 
{
	
	g_Language=1;

	GetConfig();
	
}



void CRiplayerDlg::OnRightMenu()
{


	if (m_display)
	{
		m_rKeyMenuSub->EnableMenuItem(IDC_BTN_PICTURE,MF_BYCOMMAND|MF_ENABLED);
	
		m_rKeyMenuSub->EnableMenuItem(IDC_BTN_PAUSE,MF_BYCOMMAND|MF_ENABLED);
		m_rKeyMenuSub->EnableMenuItem(IDC_BTN_STOP,MF_BYCOMMAND|MF_ENABLED);
		m_rKeyMenuSub->EnableMenuItem(IDC_BTN_PLAY,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
		if (m_recording)
		{
			m_rKeyMenuSub->EnableMenuItem(IDM_STOPRECROD,MF_BYCOMMAND|MF_ENABLED);// 录制时录制菜单无效
			m_rKeyMenuSub->EnableMenuItem(IDC_BTN_RECORD,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);//// 录制时停止录制菜单有效
		}
		else
		{
			m_rKeyMenuSub->EnableMenuItem(IDM_STOPRECROD,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
			m_rKeyMenuSub->EnableMenuItem(IDC_BTN_RECORD,MF_BYCOMMAND|MF_ENABLED);
		}
		if (m_bFullScreen)
		{
			m_rKeyMenuSub->EnableMenuItem(1,MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
		}
		else
		{
			m_rKeyMenuSub->EnableMenuItem(1,MF_BYPOSITION|MF_ENABLED);
		}

	}
	else
	{
		m_rKeyMenuSub->EnableMenuItem(IDC_BTN_PICTURE,MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
		m_rKeyMenuSub->EnableMenuItem(IDC_BTN_RECORD,MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
		m_rKeyMenuSub->EnableMenuItem(IDC_BTN_PAUSE,MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
		m_rKeyMenuSub->EnableMenuItem(IDC_BTN_STOP,MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
		m_rKeyMenuSub->EnableMenuItem(IDM_STOPRECROD,MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
		m_rKeyMenuSub->EnableMenuItem(IDC_BTN_PLAY,MF_BYCOMMAND|MF_ENABLED);
		m_rKeyMenuSub->EnableMenuItem(1,MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
	
	}
//	m_rKeyMenuSub->EnableMenuItem(9,MF_BYPOSITION|MF_DISABLED|MF_GRAYED);

	CPoint   point; 
	GetCursorPos(&point); 

	ScreenToClient(&point);
	if (m_rtdisplay.PtInRect(point))
	{
		ClientToScreen(&point);
		m_rKeyMenuSub->TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,this);
	}

}
void CRiplayerDlg::Display()
{
	CPoint   point; 
	GetCursorPos(&point); 
	ScreenToClient(&point);
	if (m_rcview.PtInRect(point))
	{
		if (!m_display)
		{
			OnOpen();
		
		}
	
	}
}

BOOL CRiplayerDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CRiplayerDlg::OnOpenfile() 
{
	// TODO: Add your command handler code here
	CFileDialog dlgFile(TRUE);
	CString strFileName;
	dlgFile.m_ofn.lpstrTitle = "打开文件";
	dlgFile.m_ofn.lpstrFilter = "Sdp File(.sdp)\0*.sdp\0\0";
	if (dlgFile.DoModal() == IDOK)
	{
		CFile file;
		if (file.Open(dlgFile.GetPathName(),  CFile::modeRead))
		{

			strFileName = dlgFile.GetPathName();
			g_strUrl = dlgFile.GetFileName();
			AddMru(strFileName);
			sdp_FileLen = file.GetLength();
			if(sdp_FileLen >=1024)
			{
				sdp_FileLen =1024;
			}
			file.Read(sdp_pBuf,sdp_FileLen);
			sdp_pBuf[sdp_FileLen+1] = 0xa;
			play_info.flag = 2;
			PRINTF("OnOpenfile:Len = %d\n",sdp_FileLen);
			file.Close();
			OnPlay("2");
			Tipmsg(g_strUrl);
		}else{
			PRINTF("OnOpenfile:open file failed!\n");
		}
	}
}

void CRiplayerDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	//窗口销毁时显示任务栏
	OnStoprecrod();
	CRect rcTrayWnd;
	CWnd* pTaskWnd = CWnd::FindWindow("Shell_TrayWnd","");//得到任务栏主窗口指针
	pTaskWnd->GetWindowRect(&rcTrayWnd); 
	pTaskWnd->ShowWindow(SW_SHOW);
}



void CRiplayerDlg::cb_msg_parse(Stream_Protocol protocol,int err)
{

//	char temp[128];
	switch(err){
	case MSG_CREATE_FAIL:
		{
			if(RTP_STREAM == protocol)
				PRINTF("cb_msg_parse:rtp thread  create fail....\n");
			else if(TS_STREAM == protocol)
				PRINTF("cb_msg_parse:TS thread  create fail....\n");
			else if(RTP_TS_STREAM == protocol)
				PRINTF("cb_msg_parse:RTP+TS thread create fail ....\n");
			else if(RTSP_STREAM == protocol)
				PRINTF("cb_msg_parse:RTSP thread create fail ....\n");
		}
		break;
	case MSG_RECV_FAIL:
		{
			PRINTF("cb_msg_parse:recv fail!\n");
			m_staiicthis->SendMessage(WM_COMMAND,WM_NEW_MSG,0);
			m_staiicthis->SendMessage(WM_COMMAND,IDC_BTN_STOP,0);
		}
		break;
	case MSG_LOST_FRAME:
		{
			g_lost_frame_cnt++;
			if( (lost_frame_flag == true && g_lost_frame_cnt >= 2 )&& (RTP_STREAM == protocol)){
					g_lost_frame_cnt -= 2;
				lost_frame_flag = false;
			}
		}
		break;
	case MSG_THREAD_EXIT:
		{			
			if(RTP_STREAM == protocol)
				PRINTF("cb_msg_parse:rtp thread  exit....\n");
			else if(TS_STREAM == protocol)
				PRINTF("cb_msg_parse:TS thread  exit....\n");
			else if(RTP_TS_STREAM == protocol)
				PRINTF("cb_msg_parse:RTP+TS thread exit ....\n");
			else if(RTSP_STREAM == protocol)
				PRINTF("cb_msg_parse:RTSP thread exit....\n");
			
			if(play_info.protocol == RTSP_STREAM)
				m_staiicthis->SendMessage(WM_COMMAND,IDC_BTN_STOP,0);
		}
		break;
	case MSG_WRITE_FAIL:
		{
			PRINTF("cb_msg_parse:write ERROR ....\n");
		}
		break;
	case MSG_MEM_ERROR:
		{
			PRINTF("cb_msg_parse:ERROR ....\n");
			
		}
		break;
	default:
		break;				
	}

		
}
static char info_cnt = 0 ;
void CRiplayerDlg::GetEncoderCallBack(Encoder_Info* info)
{
	if(info_cnt >= 30){
		PRINTF("Encoder_Info:%s,frame_rate=%d,lost_frame=%d,total_frame=%d,bit_rate=%d\n",
			info->origin,info->frame_rate,
			info->lost_frame,info->total_frame,info->bit_rate);
		info_cnt=0;
		}
	info_cnt++;
	g_Lost_frame=info->lost_frame;
	g_Total_frame=info->total_frame;
	g_Frame_rate=info->frame_rate;
	g_Bit_rate=info->bit_rate;
	//,....

}
void CRiplayerDlg::MsgBox()
{
	if( m_handle ){
		CMsgBoxDlg dlg;
		dlg.SetMsg(GetStringLanguage("MsgBox", "Caption3", "错误"), GetStringLanguage("MsgBox", "Tip11", "视频连接异常"));
		dlg.DoModal();
		}
}



void CRiplayerDlg::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);
	CRect rect,rect1;
	GetWindowRect(rect);
	rect1.SetRect(rect.left+rect.Width()/2-250,rect.bottom-80,rect.left+rect.Width()/2+250,rect.bottom-40);	
	
	::SetWindowPos(dlgTip->GetSafeHwnd(),NULL,rect1.left,rect1.top,500,40,SWP_NOSIZE);
}

