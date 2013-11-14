// PictureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Riplayer.h"
#include "PictureDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPictureDlg dialog


CPictureDlg::CPictureDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPictureDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPictureDlg)
	m_strPath = _T("");
	m_IniFileName = _T("SavePathName.ini");	//设置ini文件名
	m_PsPathName= _T("");
	m_CurNum=0;
	m_nInRect = 0;
	m_numbtn = 0;
	//}}AFX_DATA_INIT
}


void CPictureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPictureDlg)
	DDX_Control(pDX, IDC_EDIT1, m_ctredit1);
	DDX_Text(pDX, IDC_EDIT1, m_strPath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPictureDlg, CDialog)
	//{{AFX_MSG_MAP(CPictureDlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPictureDlg message handlers
//extern bool g_Language;
BOOL CPictureDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetClientRect(m_rect);
	InitLanguage();
	InitRect();
	ReadMru();
	GetDlgItem(IDC_EDIT1)->EnableWindow(false); //让编辑框失去焦点
	m_strPath=m_PsPathName;

	SetDlgItemText(IDC_STATIC1,m_strLanguage[1]);
	UpdateData(FALSE);
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
void CPictureDlg::InitRect()
{
	m_rtbutton.SetRect(m_rect.right-105,m_rect.top+90,m_rect.right-36,m_rect.top+112);
	m_rtclose.SetRect(m_rect.Width()-28,0,m_rect.Width(),20);
	m_rtcaption.SetRect(0,0,m_rect.Width()-30,20);
	m_rtcancel.SetRect(m_rect.right-74,m_rect.bottom-30,m_rect.right-5,m_rect.bottom-7);
	m_rtensure.SetRect(m_rect.right-157,m_rect.bottom-30,m_rect.right-84,m_rect.bottom-7);
}

void CPictureDlg::InitLanguage()
{
	if (g_Language)
	{
		m_strLanguage[0] = GetStringLanguage("Picture","Caption","Save File Pictures");
		m_strLanguage[1] = GetStringLanguage("Picture","Static1","The Pictures Will Save this Folder:");
		m_strLanguage[2] = GetStringLanguage("Picture","Button1","Change");
		m_strLanguage[3] = GetStringLanguage("Picture","Button2","OK");
		m_strLanguage[4] = GetStringLanguage("Picture","Button3","Connect");
		
	}
	else
	{
		m_strLanguage[0] = GetStringLanguage("Picture","Caption","保存拍照文件");
		m_strLanguage[1] = GetStringLanguage("Picture","Static1","把拍照图片保存到此文件夹中:");
		m_strLanguage[2] = GetStringLanguage("Picture","Button1","更改目录");
		m_strLanguage[3] = GetStringLanguage("Picture","Button2","确定");
		m_strLanguage[4] = GetStringLanguage("Picture","Button3","取消");
	}
	m_strLanguage[5] = GetStringLanguage("Picture","Static3","请选择图片保存目录");
}

void CPictureDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	char szDir[MAX_PATH];
	BROWSEINFO bi;
	ITEMIDLIST *pidl;
	//填充BROWSEINFO结构体
	bi.hwndOwner = this->m_hWnd; // 指定父窗口，在对话框显示期间，父窗口将被禁用 
	bi.pidlRoot = NULL; // 如果指定NULL，就以"桌面"为根 
	bi.pszDisplayName = szDir;
	bi.lpszTitle = m_strLanguage[5]; // 这一行将显示在对话框的顶端 
	bi.ulFlags = BIF_STATUSTEXT  | BIF_RETURNONLYFSDIRS  ;//在对话框中添加新建文件夹按钮
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;
	//显示对话框
	pidl = SHBrowseForFolder(&bi);
	if(pidl == NULL)
		return;
	if(!SHGetPathFromIDList(pidl, szDir)) 
		return;
	else
	{
		m_strPath = szDir;
	}
	UpdateData(FALSE);
	m_numbtn = 0;
	InvalidateRect(&m_rtclose);
	InvalidateRect(&m_rtensure);
	InvalidateRect(&m_rtbutton);
	InvalidateRect(&m_rtcancel);
}

void CPictureDlg::EnSure() 
{
	// TODO: Add your control notification handler code here
	m_PsPathName=m_strPath;
    WriteMru();
	CDialog::OnOK();
}
//读取ini文件中的记录
void CPictureDlg::ReadMru()
{

	m_PathFileName = strAppDir + m_IniFileName;
	::GetPrivateProfileString("Picture", "PathName", "D:\\My documents", m_PsPathName.GetBuffer(MAX_PATH),MAX_PATH, m_PathFileName);   //读取历史记录名
}

//历史记录写入ini文件
void CPictureDlg::WriteMru()
{
	::WritePrivateProfileString("Picture", "PathName", m_PsPathName, m_PathFileName); //写路径名
	
}


void CPictureDlg::OnLButtonDown(UINT nFlags, CPoint point) 
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
	
	if(m_rtensure.PtInRect(point))
	{
		m_ensure=true;
	}
	if(m_rtcancel.PtInRect(point))
	{
		m_cancel=true;
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CPictureDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect[5];
	rect[0] = m_rtbutton;
	rect[1] = m_rtclose;
	rect[2] = m_rtcancel;
	rect[3] = m_rtensure;
	for(int i=0; i<4; i++)
	{
		if(rect[i].PtInRect(point))      
		{  
			m_numbtn = i+1;
			m_nInRect = i +1;
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
		case 3:
			{
				InvalidateRect(&rect[2]);
			}
			break;
		case 4:
			{
				InvalidateRect(&rect[3]);
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
		case 4:
			{
				InvalidateRect(&rect[3]);
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
		if(!m_rtcancel.PtInRect(point))
		{
			m_cancel=false;
		}
	}
	
	if(m_ensure)
	{
		if(!m_rtensure.PtInRect(point))
		{
			m_ensure=false;
		}
	}
	CDialog::OnMouseMove(nFlags, point);
}

void CPictureDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtclose.PtInRect(point)&&m_close)
	{
		CDialog::OnCancel();
	}

	if(m_rtbutton.PtInRect(point)&&m_button)
	{
		OnButton1();
	}
	if(m_rtcancel.PtInRect(point)&&m_cancel)
	{
		CDialog::OnCancel();
	}

	if(m_rtensure.PtInRect(point)&&m_ensure)
	{
		EnSure();
	}
	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CPictureDlg::OnEraseBkgnd(CDC* pDC) 
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
			m_DlgPic.Load(IDR_JPG_BTN1);
			if(m_DlgPic != NULL)
				m_DlgPic.Render(&memdc,m_rtcancel);	
			m_DlgPic.Load(IDR_JPG_BTN1);
			if(m_DlgPic != NULL)
				m_DlgPic.Render(&memdc,m_rtensure);
		}
		break;
	case 1:	
		{	
			m_DlgPic.Load(IDR_JPG_BTN1_1);
			m_DlgPic.Render(&memdc,m_rtbutton);
		}
		break;
	case 2:	
		{
			m_DlgPic.Load(IDR_CLOSEX1);
			m_DlgPic.Render(&memdc,m_rtclose);
		}
		break;
	case 3:	
		{	
			m_DlgPic.Load(IDR_JPG_BTN1_1);
			m_DlgPic.Render(&memdc,m_rtcancel);
		}
		break;
	case 4:	
		{
			m_DlgPic.Load(IDR_JPG_BTN1_1);
			m_DlgPic.Render(&memdc,m_rtensure);
		}
		break;
	default:
		break;
	}

	CFont   *pOldfont = (CFont *)memdc.SelectObject(&m_font); 
    memdc.SetTextColor(RGB(0, 0, 0)); 
	//memdc.SetBkColor(TRANSPARENT);
	memdc.DrawText(m_strLanguage[0],CRect(10,5,150,30),DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[2],m_rtbutton,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[3],m_rtensure,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[4],m_rtcancel,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pDC->BitBlt(0,0,m_rect.Width(),m_rect.Height(),&memdc,0,0,SRCCOPY);
	bmp.DeleteObject();
	memdc.DeleteDC();
	ReleaseDC(pDC);
	return true;
}


HBRUSH CPictureDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if(nCtlColor == CTLCOLOR_STATIC) 
	{ 
		//IDC_STATIC为你想要设为透明的控件ID 
		if(pWnd->GetDlgCtrlID()== IDC_STATIC1)
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

void CPictureDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	/********* 设置对话框边角为圆角 *******/
	CRgn m_rgn;
    CRect rc;
	GetWindowRect(&rc);
	rc-=rc.TopLeft();
	m_rgn.CreateRoundRectRgn(rc.left,rc.top,rc.right,rc.bottom,5,5);
	SetWindowRgn(m_rgn,TRUE);
}
