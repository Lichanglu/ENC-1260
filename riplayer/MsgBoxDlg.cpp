// MsgBoxDlg.cpp : implementation file
//

#include "stdafx.h"
#include "riplayer.h"
#include "MsgBoxDlg.h"
#include "RiplayerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMsgBoxDlg dialog


CMsgBoxDlg::CMsgBoxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMsgBoxDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMsgBoxDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMsgBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMsgBoxDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMsgBoxDlg, CDialog)
	//{{AFX_MSG_MAP(CMsgBoxDlg)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsgBoxDlg message handlers

void CMsgBoxDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CMsgBoxDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}
BOOL CMsgBoxDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_bInRect = false;
	m_numbtn = 0;
	GetClientRect(m_rect);
	
	InitRect();
	InitLanguage();
	/****** 字体设置 ***********/
	if (g_Language)
	{
		m_font.CreatePointFont(100, "Verdana");
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
void CMsgBoxDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtclose.PtInRect(point)&&m_close)
	{
		CDialog::OnCancel();
	}
	
	if(m_rtbutton.PtInRect(point)&&m_button)
	{
		OnOK() ;
	}

	CDialog::OnLButtonUp(nFlags, point);
}

void CMsgBoxDlg::OnLButtonDown(UINT nFlags, CPoint point) 
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
	if(m_rtcancel.PtInRect(point))
	{
		m_bCancel=true;
	}
	if(m_rtclose.PtInRect(point))
	{
		m_close=true;
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CMsgBoxDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect[2];
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
	if(m_bCancel)
	{
		if(!m_rtcancel.PtInRect(point))
		{
			m_bCancel=false;
		}
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CMsgBoxDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	CRgn m_rgn;
    CRect rc;
	GetWindowRect(&rc);
	rc-=rc.TopLeft();
	m_rgn.CreateRoundRectRgn(rc.left,rc.top,rc.right,rc.bottom,3,3);
	SetWindowRgn(m_rgn,TRUE);
	// TODO: Add your message handler code here
	
}


void CMsgBoxDlg::InitRect()
{
	m_rtbutton.SetRect(m_rect.Width()/2 - 34, m_rect.bottom - 30, m_rect.Width()/2 + 35, m_rect.bottom - 8);
	m_rtmsg.SetRect(m_rect.left+30,m_rtbutton.bottom+30,m_rect.right-30,m_rtbutton.bottom+10);
	m_rtclose.SetRect(m_rect.Width()-28,0,m_rect.Width(),20);
	m_rtcaption.SetRect(0,0,m_rect.Width()-32,20);
}
void CMsgBoxDlg::InitLanguage()
{
	if (g_Language)
	{
		m_strText[1] = GetStringLanguage("MsgBox","Button1","OK");
		m_strText[2] = GetStringLanguage("MsgBox","Button2","Cancel");
	}
	else
	{
		m_strText[1] = GetStringLanguage("MsgBox","Button1","确定");
		m_strText[2] = GetStringLanguage("MsgBox","Button2","取消");
	}
}

BOOL CMsgBoxDlg::OnEraseBkgnd(CDC* pDC) 
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
	
	
	m_DlgPic.Load(IDR_ABOUT);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rect);
	
	switch(m_numbtn)
	{
	case 0:
		{
			m_DlgPic.Load(IDR_JPG_BTN1);
			if(m_DlgPic != NULL)
				m_DlgPic.Render(&memdc,m_rtbutton);			
			m_DlgPic.Load(IDR_CLOSEX);
			if(m_DlgPic != NULL)
        		m_DlgPic.Render(&memdc,m_rtclose);
		}	
		break;
	case 1:
		{
			m_DlgPic.Load(IDR_JPG_BTN1_1);
			if(m_DlgPic != NULL)
				m_DlgPic.Render(&memdc,m_rtbutton);
		}
		break;
	case 2:	
		{
			m_DlgPic.Load(IDR_CLOSEX1);
			if(m_DlgPic != NULL)
				m_DlgPic.Render(&memdc,m_rtclose);
		}
		
		break;

	default:
		break;
	}
	
	
	CFont   *pOldfont = (CFont *)memdc.SelectObject(&m_font); 
    memdc.SetTextColor(RGB(0, 0, 0)); 

	CSize sz;
	sz = memdc.GetTextExtent(m_StrMsg);
	if(sz.cx > rect.Width() - 20)
	{
		CString str1 = m_StrMsg.Left(m_StrMsg.GetLength()/2);
		CString str2 = m_StrMsg.Right(m_StrMsg.GetLength() - m_StrMsg.GetLength()/2);
		memdc.DrawText(str1,m_rtmsg,DT_CENTER|DT_VCENTER);
		memdc.DrawText(str2,CRect(16,70,rect.Width() - 6,rect.Height() - 24),DT_CENTER|DT_VCENTER);
	}
	else
		memdc.DrawText(m_StrMsg,CRect(16,60,rect.Width() - 6,rect.Height() - 24),DT_CENTER|DT_VCENTER);
	memdc.DrawText(m_strText[0],CRect(5,5,57,25),DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strText[1],m_rtbutton,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pDC->BitBlt(0,0,m_rect.Width(),m_rect.Height(),&memdc,0,0,SRCCOPY);
	bmp.DeleteObject();
	memdc.DeleteDC();
	ReleaseDC(pDC);
	return true;	

}
void CMsgBoxDlg::SetMsg(CString caption, CString str)
{
	m_strText[0] = caption;
	m_StrMsg = str;
}