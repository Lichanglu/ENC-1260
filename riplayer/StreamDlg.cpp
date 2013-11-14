// StreamDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Riplayer.h"
#include "StreamDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStreamDlg dialog


CStreamDlg::CStreamDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStreamDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStreamDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CStreamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStreamDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStreamDlg, CDialog)
	//{{AFX_MSG_MAP(CStreamDlg)
//	ON_BN_CLICKED(IDC_TSTART, OnTsart)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnButton7)
	ON_BN_CLICKED(IDC_BUTTON8, OnButton8)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStreamDlg message handlers
BOOL CStreamDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	InitLanguage();
	GetClientRect(m_rect);
	InitRect();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CStreamDlg::InitLanguage()
{
	if (g_Language)
	{
		m_strLanguage[0] = GetStringLanguage("Stream Code","Static1","Start");
		m_strLanguage[1] = GetStringLanguage("Stream Code","Static2","Pause");
		m_strLanguage[2] = GetStringLanguage("Stream Code","Static3","Stop");
		m_strLanguage[3] = GetStringLanguage("Stream Code","Static4","Resume");
		m_strLanguage[4] = GetStringLanguage("Stream Code","button1","Save");

		
	}
	else
	{
		m_strLanguage[0] = GetStringLanguage("Stream Code","Static1","开始");
		m_strLanguage[1] = GetStringLanguage("Stream Code","Static2","暂停:");
		m_strLanguage[2] = GetStringLanguage("Stream Code","Static3","停止");
		m_strLanguage[3] = GetStringLanguage("Stream Code","Static4","恢复");
		m_strLanguage[4] = GetStringLanguage("Stream Code","button1","保存");
	
	}
}

void CStreamDlg::InitRect()
{
	m_rtbutton.SetRect(m_rect.right-169,m_rect.top+130,m_rect.right-100,m_rect.top+152);
	m_rtclose.SetRect(m_rect.left+72,m_rect.top+35,m_rect.left+141,m_rect.top+57);
	m_rtbutton1.SetRect(m_rect.left+72,m_rect.top+65,m_rect.left+141,m_rect.top+87);
	m_rtbutton2.SetRect(m_rect.left+72,m_rect.top+95,m_rect.left+141,m_rect.top+117);
	m_rtbutton3.SetRect(m_rect.left+151,m_rect.top+65,m_rect.left+220,m_rect.top+87);
	m_rtbutton4.SetRect(m_rect.left+151,m_rect.top+35,m_rect.left+220,m_rect.top+57);
	m_rtbutton5.SetRect(m_rect.left+151,m_rect.top+95,m_rect.left+220,m_rect.top+117);
}

void CStreamDlg::OnTsart() 
{
	// TODO: Add your control notification handler code here
	CString str;
/*	GetDlgItemText(IDC_TSTART,str);
	if(str=="开始")
		SetDlgItemText(IDC_TSTART,"停止");
	else
		SetDlgItemText(IDC_TSTART,"开始");*/
}

void CStreamDlg::OnButton4() 
{
	// TODO: Add your control notification handler code here
	CString str;
	GetDlgItemText(IDC_BUTTON4,str);
	if(str=="开始")
		SetDlgItemText(IDC_BUTTON4,"停止");
	else
		SetDlgItemText(IDC_BUTTON4,"开始");
}

void CStreamDlg::OnButton5() 
{
	// TODO: Add your control notification handler code here
	CString str;
	GetDlgItemText(IDC_BUTTON5,str);
	if(str=="开始")
		SetDlgItemText(IDC_BUTTON5,"停止");
	else
		SetDlgItemText(IDC_BUTTON5,"开始");
}

void CStreamDlg::OnButton6() 
{
	// TODO: Add your control notification handler code here
	CString str;
	GetDlgItemText(IDC_BUTTON6,str);
	if(str=="暂停")
		SetDlgItemText(IDC_BUTTON6,"恢复");
	else
		SetDlgItemText(IDC_BUTTON6,"暂停");
}

void CStreamDlg::OnButton7() 
{
	// TODO: Add your control notification handler code here
	CString str;
	GetDlgItemText(IDC_BUTTON7,str);
	if(str=="暂停")
		SetDlgItemText(IDC_BUTTON7,"恢复");
	else
		SetDlgItemText(IDC_BUTTON7,"暂停");
}

void CStreamDlg::OnButton8() 
{
	// TODO: Add your control notification handler code here
	CString str;
	GetDlgItemText(IDC_BUTTON8,str);
	if(str=="暂停")
		SetDlgItemText(IDC_BUTTON8,"恢复");
	else
		SetDlgItemText(IDC_BUTTON8,"暂停");
}

void CStreamDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtbutton.PtInRect(point))
	{
		m_button=true;
	//	InvalidateRect(m_rtbutton);
	}
	if(m_rtclose.PtInRect(point))
	{
		m_close=true;
		//InvalidateRect(m_rtclose);
	}
	if(m_rtbutton1.PtInRect(point))
	{
		m_button1=true;
		//InvalidateRect(m_rtclose);
	}
	if(m_rtbutton2.PtInRect(point))
	{
		m_button1=true;
		//InvalidateRect(m_rtclose);
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CStreamDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect[10];
	rect[0]=m_rtbutton;
	rect[1]=m_rtclose;
	rect[2]=m_rtbutton1;
	rect[3]=m_rtbutton2;
	rect[4]=m_rtbutton3;
	rect[5]=m_rtbutton4;
	rect[6]=m_rtbutton5;
	for(int i=0; i<7; i++)
	{
		if(rect[i].PtInRect(point))      
		{  
			m_numbtn=i+1;
			for(int i=0; i<7; i++)
			{
						
				InvalidateRect(rect[i]);//换图
		
			}
			break;

		}
		else
		{   
			m_numbtn=0;
			InvalidateRect(rect[i]);//换图
		}
		
	}


	if(m_button)
	{
		if(!m_rtbutton.PtInRect(point))
		{
			m_button=false;
		}
	}
	if(m_button1)
	{
		if(!m_rtbutton1.PtInRect(point))
		{
			m_button1=false;
		}
	}
	if(m_button2)
	{
		if(!m_rtbutton1.PtInRect(point))
		{
			m_button1=false;
		}
	}
	if(m_button3)
	{
		if(!m_rtbutton3.PtInRect(point))
		{
			m_button3=false;
		}
	}
	if(m_button4)
	{
		if(!m_rtbutton4.PtInRect(point))
		{
			m_button4=false;
		}
	}
	if(m_button5)
	{
		if(!m_rtbutton5.PtInRect(point))
		{
			m_button5=false;
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

void CStreamDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtclose.PtInRect(point)&&m_close)
	{
		MessageBox("开始");
	}

	if(m_rtbutton.PtInRect(point)&&m_button)
	{
		MessageBox("保存");
	}
	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CStreamDlg::OnEraseBkgnd(CDC* pDC) 
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
	CBrush bru(RGB(239,255,255));
	CRect cr(0,0,rect.Width(),rect.Height());
	memdc.FillRect(&cr,&bru); 	//填充背景色

/*	m_DlgPic.Load(IDR_JPG_DIALOG1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rect);*/

	m_DlgPic.Load(IDR_JPG_BTN1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rtbutton);

	m_DlgPic.Load(IDR_JPG_BTN1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rtbutton1);

	m_DlgPic.Load(IDR_JPG_BTN1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rtbutton2);

	m_DlgPic.Load(IDR_JPG_BTN1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rtclose);
	m_DlgPic.Load(IDR_JPG_BTN1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rtbutton3);
	
	m_DlgPic.Load(IDR_JPG_BTN1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rtbutton4);
	
	m_DlgPic.Load(IDR_JPG_BTN1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rtbutton5);
	switch(m_numbtn)
	{
		case 0:

			break;
		case 1:		m_DlgPic.Load(IDR_JPG_BTN1_1);
					if(m_DlgPic != NULL)
							m_DlgPic.Render(&memdc,m_rtbutton);
		break;
		case 2:		m_DlgPic.Load(IDR_JPG_BTN1_1);
					if(m_DlgPic != NULL)
						m_DlgPic.Render(&memdc,m_rtclose);
			break;
		case 3:		m_DlgPic.Load(IDR_JPG_BTN1_1);
					if(m_DlgPic != NULL)
						m_DlgPic.Render(&memdc,m_rtbutton1);
			break;
		case 4:		m_DlgPic.Load(IDR_JPG_BTN1_1);
					if(m_DlgPic != NULL)
						m_DlgPic.Render(&memdc,m_rtbutton2);
			break;
		case 5:		m_DlgPic.Load(IDR_JPG_BTN1_1);
						m_DlgPic.Render(&memdc,m_rtbutton3);
			break;
		case 6:		m_DlgPic.Load(IDR_JPG_BTN1_1);
						m_DlgPic.Render(&memdc,m_rtbutton4);
			break;
		case 7:		m_DlgPic.Load(IDR_JPG_BTN1_1);
						m_DlgPic.Render(&memdc,m_rtbutton5);
			break;
		default:
			break;
	}

	CFont font;
	font.CreateFont(14, 0, 0, 0 ,40,
            FALSE, FALSE, 0,
            ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH|FF_SWISS,
            "Verdana");
	CFont   *pOldfont = (CFont *)memdc.SelectObject(&font); 
    memdc.SetTextColor(RGB(0, 0, 0)); 
	//memdc.SetBkColor(TRANSPARENT);*/
	memdc.DrawText(m_strLanguage[4],m_rtbutton,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[0],m_rtbutton1,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[0],m_rtbutton2,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[0],m_rtclose,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[1],m_rtbutton3,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[1],m_rtbutton4,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[1],m_rtbutton5,DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	pDC->BitBlt(0,0,m_rect.Width(),m_rect.Height(),&memdc,0,0,SRCCOPY);
	bmp.DeleteObject();
	memdc.DeleteDC();
	ReleaseDC(pDC);
	return true;
}


HBRUSH CStreamDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if(nCtlColor == CTLCOLOR_STATIC) 
	{ 
		//IDC_STATIC为你想要设为透明的控件ID 
		if(pWnd->GetDlgCtrlID()== IDC_STATIC1||IDC_STATIC2||IDC_STATIC3)
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

void CStreamDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	delete this;
	CDialog::PostNcDestroy();
}
