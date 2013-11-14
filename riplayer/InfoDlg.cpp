// InfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Riplayer.h"
#include "InfoDlg.h"
#include "RiplayerDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
// CInfoDlg dialog

CInfoDlg::CInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInfoDlg::IDD, pParent)
{
	m_bflag = false;
	m_strVedio = _T("");
	//}}AFX_DATA_INIT
}
CInfoDlg::~CInfoDlg()
{
	for (int i =0; i < 10; i++)
	{
		m_strText[i].IsEmpty();
		m_strInfo[i].IsEmpty();
	}
}

void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInfoDlg)
//	DDX_Control(pDX, IDC_LIST1, m_List1);
	//DDX_Text(pDX, IDC_STATIC_VIEW, m_strVedio);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CInfoDlg)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
int nWidth = 0, nHeight = 0;
/////////////////////////////////////////////////////////////////////////////
// CInfoDlg message handlers
BOOL CInfoDlg::OnInitDialog() 

{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	GetClientRect(m_rect);
	InitRect();
	InitLanguage();
	CenterWindow();  //显示居中
	SetTimer(1,500,NULL);


	m_bInRect = false;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CInfoDlg::InitLanguage()
{

		m_strCaption = GetStringLanguage("Info","Caption","媒体信息");
		m_strText[7] = GetStringLanguage("Info","Static0","    媒体信息:");
		m_strText[6] = GetStringLanguage("Info","Static1","    播放地址:");
		m_strText[5] = GetStringLanguage("Info","Static3","        帧率:");
		m_strText[4] = GetStringLanguage("Info","Static4","      位速率:");
		m_strText[3] = GetStringLanguage("Info","Static5","已播放视频帧:");
		m_strText[2] = GetStringLanguage("Info","Static6","已丢失视频帧:");
		m_strText[1] = GetStringLanguage("Info","Static7","视频原始大小:");
		m_strText[0] = GetStringLanguage("Info","Static8","当前视频大小:");



	if (g_Language)
	{
		m_strVedio = "stereo";
		m_strRect = "pixel";
	}
	else
	{
		m_strVedio = "立体声";
		m_strRect = "像素";
	}

}
void CInfoDlg::InitRect()
{
	m_rtbutton.SetRect(m_rect.right-83,m_rect.bottom-30,m_rect.right-14,m_rect.bottom-8);
	m_rtclose.SetRect(m_rect.right-28,0,m_rect.right,20);
	m_rtcaption.SetRect(0,0,m_rect.Width()-32,28);
	m_rctext[0].SetRect(m_rect.left + 170, m_rect.top + /*m_rect.Height() - 87*/236,m_rect.right-5,258);
	m_rctext[1].SetRect(m_rect.left + 170, m_rect.top + 209,m_rect.right-5,m_rect.top + 231);
	m_rctext[2].SetRect(m_rect.left + 170, m_rect.top + 182,m_rect.right-5,m_rect.top + 204);
	m_rctext[3].SetRect(m_rect.left + 170, m_rect.top + 155,m_rect.right-5,m_rect.top + 177);
	m_rctext[4].SetRect(m_rect.left + 170, m_rect.top + 128,m_rect.right-5,m_rect.top + 150);
	m_rctext[5].SetRect(m_rect.left + 170, m_rect.top + 101,m_rect.right-5,m_rect.top + 123);
	m_rctext[6].SetRect(m_rect.left + 170, m_rect.top + 64,m_rect.right-5,m_rect.top + 84);
	m_rctext[7].SetRect(m_rect.left + 170, m_rect.top + 37,m_rect.right-5,m_rect.top + 59);
	m_rctext[8].SetRect(m_rect.left + 170, m_rect.top + 85,m_rect.right-5,m_rect.top + 100);

	m_rcaption[0].SetRect(m_rect.left + 5, m_rect.top + 236,m_rect.left + 160,m_rect.top + 258);
	m_rcaption[1].SetRect(m_rect.left + 5, m_rect.top + 209,m_rect.left + 160,m_rect.top + 231);
	m_rcaption[2].SetRect(m_rect.left + 5, m_rect.top + 182,m_rect.left + 160,m_rect.top + 204);
	m_rcaption[3].SetRect(m_rect.left + 5, m_rect.top + 155,m_rect.left + 160,m_rect.top + 177);
	m_rcaption[4].SetRect(m_rect.left + 5, m_rect.top + 128,m_rect.left + 160,m_rect.top + 150);
	m_rcaption[5].SetRect(m_rect.left + 5, m_rect.top + 101,m_rect.left + 160,m_rect.top + 123);
	m_rcaption[6].SetRect(m_rect.left + 5, m_rect.top + 64,m_rect.left + 160,m_rect.top + 84);
	m_rcaption[7].SetRect(m_rect.left + 5, m_rect.top + 37,m_rect.left + 160,m_rect.top + 59);
//	m_rcaption[8].SetRect(m_rect.left + 5,m_rect.bottom-294,m_rect.left + 130,m_rect.bottom-272);

}
BOOL CInfoDlg::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CPoint point;
	CRect rect;
	GetWindowRect(&rect);
	GetClientRect(&m_rect);
	CDC memdc;     //定义一个显示设备对象 
	CBitmap bmp;       //定义一个位图对象 
	memdc.CreateCompatibleDC(pDC);     //建立与屏幕显示兼容的内存显示设备 
	bmp.CreateCompatibleBitmap(pDC,rect.Width(),rect.Height());//建立一个与屏幕显示兼容的位图
	memdc.SetBkMode(TRANSPARENT);
	//	memdc.FillSolidRect(0,0,rect.Width(),rect.Height(),RGB(93,250,255)); 	//用背景色将位图清除干净
	CBitmap* pOldBmp = memdc.SelectObject(&bmp);            //将位图选入到内存显示设备中		
	
	DWORD style = DT_CENTER|DT_SINGLELINE|DT_VCENTER;
	
	
	m_DlgPic.Load(IDR_JPG_DIALOG1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rect);
	
	if (m_bflag)
	{
		m_DlgPic.Load(IDR_CLOSEX1);
		
	}
	else
	{
		m_DlgPic.Load(IDR_CLOSEX);
	}
	m_DlgPic.Render(&memdc,m_rtclose);
	CFont font;
	if (g_Language)
	{
		font.CreatePointFont(88, "Verdana");
	}
	else
	{
		font.CreatePointFont(100,/*"Verdana"*/"宋体");
	}	

	CFont   *pOldfont = (CFont *)memdc.SelectObject(&font); 
    memdc.SetTextColor(RGB(0, 0, 0)); 
	//memdc.SetBkColor(TRANSPARENT);

	memdc.DrawText(m_strCaption,CRect(8,5,180,25),DT_LEFT| DT_VCENTER | DT_SINGLELINE);
	for (int i=0; i<8 ;i++)
	{
		memdc.DrawText(m_strText[i], m_rcaption[i],DT_RIGHT  | DT_VCENTER | DT_SINGLELINE);
	}

	for (i =0; i < 9; i++)
	{
		memdc.DrawText(m_strInfo[i], m_rctext[i],DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		
	}
	pDC->BitBlt(0,0,m_rect.Width(),m_rect.Height(),&memdc,0,0,SRCCOPY);
	font.DeleteObject();
	bmp.DeleteObject();
	memdc.DeleteDC();
	ReleaseDC(pDC);
	return true;
	
}

void CInfoDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtcaption.PtInRect(point)) 
	{
		PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x, point.y));	//向系统发送HTCAPTION消息，让系统以为鼠标点在标题栏上
	}
	if(m_rtclose.PtInRect(point))
	{
		m_close=true;
		
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CInfoDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect *pRect = &m_rtclose;	
	if(pRect->PtInRect(point))
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
		InvalidateRect(pRect);
	}
	else
	{
		if (m_bInRect)//进入矩形区域内后离开，刷新
		{
			InvalidateRect(*pRect);
			m_bInRect = false;
		}

	}
	
	if(m_close)
	{
		if(!m_rtclose.PtInRect(point))
		{
			m_close=false;
		}
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CInfoDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtclose.PtInRect(point)&&m_close)
	{
		CDialog::OnCancel();
	}


	CDialog::OnLButtonUp(nFlags, point);
}

void CInfoDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here

	CRgn m_rgn;
    CRect rc;
	GetWindowRect(&rc);
	rc-=rc.TopLeft();
	m_rgn.CreateRoundRectRgn(rc.left,rc.top,rc.right,rc.bottom,5,5);
	SetWindowRgn(m_rgn,TRUE);

}


HBRUSH CInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if(nCtlColor == CTLCOLOR_STATIC) 
	{ 
		//IDC_STATIC为你想要设为透明的控件ID 
		if(pWnd->GetDlgCtrlID()== IDC_STATIC_VIEW||IDC_STATIC1)
        {
         
			pDC->SetBkMode(TRANSPARENT);   
			return   (HBRUSH)GetStockObject(NULL_BRUSH);
        }
		
	}
	return hbr;
}

void CInfoDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == 1)
	{
		InitLanguage();
		CRect rect;
		int i =0;
		int nSample = 0;
		
		CString str1, str2,str3,str4;
		AfxGetMainWnd()->GetWindowRect(&rect);
		int width, high;
		width = rect.Width();
		high = rect.Height();
		if (prv_nWidth >0&&prv_nHieght>0)
		{
		
			m_strInfo[0].Format("%d X %d", width, high);
			m_strInfo[0]=m_strInfo[0]+"  "+m_strRect;
			
			m_strInfo[1].Format("%d X %d", prv_nWidth, prv_nHieght);
			m_strInfo[1]=m_strInfo[1]+"  "+m_strRect;

			m_strInfo[2].Format("%d", g_Lost_frame);
			m_strInfo[3].Format("%d", g_Total_frame);
			m_strInfo[4].Format("%d kbps", g_Bit_rate);
			m_strInfo[5].Format("%d f/s", g_Frame_rate);
			m_strInfo[6].Format("H.264,%d X %d",prv_nWidth,prv_nHieght);
			m_strInfo[7]=g_strUrl;
			nSample = g_Samplerate/1000;
			g_Samplerate = 0;
			m_strInfo[8].Format("AAC,%s,%dKHz",m_strVedio,nSample);
			
			for(i =0;i < 9;i++)
			{
				InvalidateRect(&m_rctext[i]);
			}		
		}
		else
		{
			width = 0;
			high = 0;
			g_Total_frame = 0;
			g_Lost_frame = 0;
			m_strInfo[0].Format("%d X %d", width, high);
			m_strInfo[0]=m_strInfo[0]+"  "+m_strRect;
			
			m_strInfo[1].Format("%d X %d", prv_nWidth, prv_nHieght);
			m_strInfo[1]=m_strInfo[1]+"  "+m_strRect;
			
			m_strInfo[2].Format("%d", g_Lost_frame);
			m_strInfo[3].Format("%d", g_Total_frame);
			m_strInfo[4].Format("%d kbps", g_Bit_rate);
			m_strInfo[5].Format("%d f/s", g_Frame_rate);
			m_strInfo[6].Format("H.264,%d X %d",prv_nWidth,prv_nHieght);
			m_strInfo[7]=g_strUrl;
			nSample = g_Samplerate/1000;
			m_strInfo[8].Format("AAC,%s,%dKHz",m_strVedio,nSample);
			for(i =0;i < 9;i++)
			{
				InvalidateRect(&m_rctext[i]);
			}
		}
		InvalidateRect(&CRect(8,5,150,25));
		for (i=0;i<8;i++)
		{
			InvalidateRect(&m_rcaption[i]);
		}
	}
	CDialog::OnTimer(nIDEvent);
}
