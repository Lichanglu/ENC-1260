// PlayVedioDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Riplayer.h"
#include "PlayVedioDlg.h"
#include "RiplayerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlayVedioDlg dialog


CPlayVedioDlg::CPlayVedioDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlayVedioDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlayVedioDlg)
	m_numbtn = 0;
	m_nInRect = 0;
	m_button = false;
	m_cancel = false;
	m_close = false;
	m_edit1 = _T("");
	//}}AFX_DATA_INIT
}


void CPlayVedioDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlayVedioDlg)
	DDX_Control(pDX, IDC_URL, m_editUrl);
	DDX_Text(pDX, IDC_URL, m_edit1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlayVedioDlg, CDialog)
	//{{AFX_MSG_MAP(CPlayVedioDlg)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayVedioDlg message handlers
//extern bool g_Language;
void CPlayVedioDlg::OnPlay() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	//CRiplayerDlg dlg ;
	//dlg.OnBtnPlay(m_edit1);
	OnOK();
//	InvalidateRect(m_rect);
}

BOOL CPlayVedioDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	GetClientRect(&m_rect);
	InitLanguage();
	if (g_Language)
	{
		m_font.CreatePointFont(88, "Verdana");
	} 
	else
	{
		m_font.CreatePointFont(90,"宋体");
	}
	SetFont(&m_font); 
	CWnd   *pw   =   GetWindow(GW_CHILD); 
	while(pw   !=   NULL) 
	{ 
		pw-> SetFont(&m_font); 
		pw   =   pw-> GetWindow(GW_HWNDNEXT); 
	}; 
	
	SetDlgItemText(IDC_STATIC1,m_strLanguage[1]);
	SetDlgItemText(IDC_STATIC2,m_strLanguage[2]);
	InitRect();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPlayVedioDlg::InitRect()
{
	m_rtbutton.SetRect(m_rect.right-179,m_rect.bottom-27,m_rect.right-110,m_rect.bottom-5);//播放
	m_rtbutton1.SetRect(m_rect.right-90,m_rect.bottom-27,m_rect.right-21,m_rect.bottom-5);//取消
	m_rtclose.SetRect(m_rect.Width()-28,0,m_rect.Width(),20);
	m_rtText.SetRect(10,3,150,28);
	m_rtcaption.SetRect(0,0,m_rect.Width()-32,20);
}
void CPlayVedioDlg::InitLanguage()
{
	
	m_strLanguage[0] = GetStringLanguage("Open Vedio","Caption","确定");;
	m_strLanguage[1] = GetStringLanguage("Open Vedio","Static1","请输入网络URL:");
	m_strLanguage[2] = GetStringLanguage("Open Vedio","Static2","网络协议");
	m_strLanguage[3] = GetStringLanguage("Open Vedio","Button1","播放");
	m_strLanguage[4] = GetStringLanguage("Open Vedio","Button2","取消");


}
void CPlayVedioDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtcaption.PtInRect(point)) 
	{
		PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x, point.y));	//向系统发送HTCAPTION消息，让系统以为鼠标点在标题栏上
	}
	if(m_rtbutton.PtInRect(point))
	{
		m_button=true;
	}
	if(m_rtclose.PtInRect(point))
	{
		m_close=true;
	}
	if(m_rtbutton1.PtInRect(point))
	{
		m_cancel=true;
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CPlayVedioDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect[5];
	rect[0]=m_rtbutton;
	rect[1]=m_rtclose;
	rect[2]=m_rtbutton1;
	for(int i=0; i<3; i++)
	{
		if(rect[i].PtInRect(point))      
		{  
			m_numbtn=i+1;
			m_nInRect=i+1;
			break;
		}
		else
		{   
			m_numbtn=0;
			
		}
		
	}

	if (m_numbtn)//进入矩形区域，刷新
	{
		switch (m_nInRect)
		{
		case 1:
			{
				InvalidateRect(&rect[0]);
			}
			break;
		case 2:
			{
				InvalidateRect(&rect[1]);
			}
			break;
		case 3:
			{
				InvalidateRect(&rect[2]);
			}
			break;
		default:
			break;
		}
	}
	else
	{
		if (m_nInRect)//进入矩形区域内后离开，刷新
		{
			switch (m_nInRect)
			{
			case 1:
				{
					InvalidateRect(&rect[0]);
				}
				break;
			case 2:
				{
					InvalidateRect(&rect[1]);
				}
				break;
			case 3:
				{
					InvalidateRect(&rect[2]);
				}
				break;
			default:
				break;
			}
			m_nInRect = 0;
			
		}
		
	}

	if(m_button)
	{
		if(!m_rtbutton.PtInRect(point))
		{
			m_button=false;
		}
	}
	
	if(m_close)
	{
		if(!m_rtclose.PtInRect(point))
		{
			m_close=false;
		}
	}

	if(m_cancel)
	{
		if(!m_rtbutton1.PtInRect(point))
		{
			m_cancel=false;
		}
	}
	CDialog::OnMouseMove(nFlags, point);
}

void CPlayVedioDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtclose.PtInRect(point)&&m_close)
	{
		CDialog::OnCancel();
	}
	if(m_rtbutton1.PtInRect(point)&&m_cancel)
	{
		CDialog::OnCancel();
	}
	if(m_rtbutton.PtInRect(point)&&m_button)
	{
		OnPlay();

	}
	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CPlayVedioDlg::OnEraseBkgnd(CDC* pDC) 
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
//	memdc.FillSolidRect(0,0,rect.Width(),rect.Height(),RGB(93,250,255)); 	//用背景色将位图清除干净
	CBitmap* pOldBmp = memdc.SelectObject(&bmp);            //将位图选入到内存显示设备中		
	
	DWORD style = DT_CENTER|DT_SINGLELINE|DT_VCENTER;


	m_DlgPic.Load(IDR_JPG_DIALOG1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rect);

/*	m_DlgPic.Load(IDR_JPG_BTN1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rtbutton);

	m_DlgPic.Load(IDR_JPG_BTN1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rtbutton1);

	m_DlgPic.Load(IDR_CLOSEX);
	if(m_DlgPic != NULL)
	{
		m_DlgPic.Render(&memdc,m_rtclose);
	}*/
	switch(m_numbtn)
	{
	case 0:
		{
			m_DlgPic.Load(IDR_JPG_BTN1);
			if(m_DlgPic != NULL)
				m_DlgPic.Render(&memdc,m_rtbutton);
			
			m_DlgPic.Load(IDR_JPG_BTN1);
			if(m_DlgPic != NULL)
				m_DlgPic.Render(&memdc,m_rtbutton1);
			
			m_DlgPic.Load(IDR_CLOSEX);
			if(m_DlgPic != NULL)
			{
				m_DlgPic.Render(&memdc,m_rtclose);
			}
		}
		break;
	case 1:		
		{
			m_DlgPic.Load(IDR_JPG_BTN1_1);
			if(m_DlgPic != NULL)
			{
				m_DlgPic.Render(&memdc,m_rtbutton);
			}
		}
		break;
	case 2:		m_DlgPic.Load(IDR_CLOSEX1);
				if(m_DlgPic != NULL)
					m_DlgPic.Render(&memdc,m_rtclose);
		break;
	case 3:		m_DlgPic.Load(IDR_JPG_BTN1_1);
				if(m_DlgPic != NULL)
					m_DlgPic.Render(&memdc,m_rtbutton1);

		break;
	default:
		break;
	}

	CFont   *pOldfont = (CFont *)memdc.SelectObject(&m_font); 
    //memdc.SetTextColor(RGB(0, 0, 0)); 
	//memdc.SetBkColor(RGB(233,235,245));//
	memdc.DrawText(m_strLanguage[3],m_rtbutton,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[4],m_rtbutton1,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[0],m_rtText,DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pDC->BitBlt(0,0,m_rect.Width(),m_rect.Height(),&memdc,0,0,SRCCOPY);
	bmp.DeleteObject();
	memdc.DeleteDC();
	ReleaseDC(pDC);
	return true;
}

void CPlayVedioDlg::OnSize(UINT nType, int cx, int cy) 
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

HBRUSH CPlayVedioDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if(nCtlColor == CTLCOLOR_STATIC) 
	{ 
		//IDC_STATIC为你想要设为透明的控件ID 
		if(pWnd->GetDlgCtrlID()== IDC_STATIC1||IDC_STATIC2)
        {
            // pDC->SetTextColor(RGB(0,0,250));      
			//pDC->SetBkColor(RGB(251, 247, 200));
			pDC->SetBkMode(TRANSPARENT);   
			return   (HBRUSH)GetStockObject(NULL_BRUSH);
        }
		
	}
	// TODO: Return a different brush if the default is not desired
	return hbr;
}
