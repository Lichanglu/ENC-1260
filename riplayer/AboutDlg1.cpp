// AboutDlg1.cpp : implementation file
//

#include "stdafx.h"
#include "Riplayer.h"
#include "AboutDlg1.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// AboutDlg dialog


AboutDlg::AboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AboutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(AboutDlg)
	m_close=false;
	m_button=false;
	m_numbtn=0;
	m_bInRect=false;
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void AboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AboutDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AboutDlg, CDialog)
	//{{AFX_MSG_MAP(AboutDlg)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AboutDlg message handlers

BOOL AboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetClientRect(m_rect);

	InitRect();
	/****** 字体设置 ***********/
	if (g_Language)
	{
		m_font.CreatePointFont(90, "Verdana");
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


void AboutDlg::OnLButtonDown(UINT nFlags, CPoint point) 
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
	CDialog::OnLButtonDown(nFlags, point);
}

void AboutDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect[5];
	rect[0]=m_rtbutton;
	rect[1]=m_rtclose;
	for(int i=0; i<2; i++)
	{
		if(rect[i].PtInRect(point))      
		{  
			m_numbtn=i+1;
			m_bInRect=true;
			break;
			
		}
		else
		{   
			m_numbtn=0;
		}
		
	}
	if (m_numbtn)//进入矩形区域，刷新
	{
		switch (m_numbtn)
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
	
		default:
			break;
		}
	}
	else
	{
		if (m_bInRect)//进入矩形区域内后离开，刷新
		{
			for (i=0;i<2;i++)
			{
				InvalidateRect(&rect[i]);
			}
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
	CDialog::OnMouseMove(nFlags, point);
}

void AboutDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtclose.PtInRect(point)&&m_close)
	{
		OnCancel();
	}
	
	if(m_rtbutton.PtInRect(point)&&m_button)
	{
	
		OnOK();

	}
	CDialog::OnLButtonUp(nFlags, point);
}


BOOL AboutDlg::OnEraseBkgnd(CDC* pDC) 
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
	CBitmap* pOldBmp = memdc.SelectObject(&bmp);            //将位图选入到内存显示设备中		
	DWORD style = DT_CENTER|DT_SINGLELINE|DT_VCENTER;
	m_DlgPic.Load(IDR_ABOUT);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rect);

	m_DlgPic.Load(IDR_JPG_BTN1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rtbutton);

	m_DlgPic.Load(IDR_CLOSEX);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rtclose);
	switch(m_numbtn)
	{
	case 0:

		break;
	case 1:		m_DlgPic.Load(IDR_JPG_BTN1_1);
				if(m_DlgPic != NULL)
						m_DlgPic.Render(&memdc,m_rtbutton);
	break;
	case 2:		m_DlgPic.Load(IDR_CLOSEX1);
				if(m_DlgPic != NULL)
					m_DlgPic.Render(&memdc,m_rtclose);
	
		break;
	default:
		break;
	}

	CFont   *pOldfont = (CFont *)memdc.SelectObject(&m_font); 
    memdc.SetTextColor(RGB(0, 0, 0)); 
	//memdc.SetBkColor(TRANSPARENT);
	CString str,str1;
	if (g_Language)
	{
		str = GetStringLanguage("About","Caption","About RiPlayer");
		str1 = GetStringLanguage("About","Button1","Ok");
	}
	else
	{
		str = GetStringLanguage("About","Caption","关于RiPlayer");
		str1 = GetStringLanguage("About","Button1","确定");
	}
	memdc.DrawText(str,CRect(10,2,180,28),DT_LEFT| DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(str1,m_rtbutton,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pDC->BitBlt(0,0,m_rect.Width(),m_rect.Height(),&memdc,0,0,SRCCOPY);
	bmp.DeleteObject();
	memdc.DeleteDC();
	ReleaseDC(pDC);
	return true;//true
}

void AboutDlg::OnSize(UINT nType, int cx, int cy) 
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

HBRUSH AboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if(nCtlColor == CTLCOLOR_STATIC) 
	{ 
	
		if(pWnd->GetDlgCtrlID()== IDC_STATIC1)
        {
			pDC->SetBkMode(TRANSPARENT);   
			return   (HBRUSH)GetStockObject(NULL_BRUSH);
        }
		
	}
	// TODO: Return a different brush if the default is not desired
	return hbr;
}
void AboutDlg::InitRect()
{
	m_rtbutton.SetRect(m_rect.Width()/2-35,m_rect.bottom-28,m_rect.Width()/2+35,m_rect.bottom-6);//ok
	m_rtclose.SetRect(m_rect.right-28,m_rect.top+0,m_rect.right-0,m_rect.top+20);//close
	m_rtcaption.SetRect(0,7,m_rect.Width()-31,27);//caption
}
