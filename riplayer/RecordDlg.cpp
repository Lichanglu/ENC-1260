// RecordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Riplayer.h"
#include "RecordDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecordDlg dialog


CRecordDlg::CRecordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRecordDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRecordDlg)
	m_edit2 = _T("");
	//}}AFX_DATA_INIT
	m_IniFileName = _T("SavePathName.ini");	//设置ini文件名
	m_PsPathName= _T("");
	m_CurNum=0;
	m_nInRect=0;
	m_numbtn=0;
}


void CRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecordDlg)
	DDX_Control(pDX, IDC_EDIT2, m_ctredit);
	DDX_Text(pDX, IDC_EDIT2, m_edit2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRecordDlg, CDialog)
	//{{AFX_MSG_MAP(CRecordDlg)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CRecordDlg message handlers
BOOL CRecordDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetClientRect(m_rect);
	InitLanguage();
	InitRect();
	m_PathFileName = strAppDir + m_IniFileName;
	ReadMru();
	GetDlgItem(IDC_EDIT2)->EnableWindow(false);
	m_edit2=m_PsPathName;

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

void CRecordDlg::InitLanguage()
{

	if (g_Language)
	{
		m_strLanguage[0] = GetStringLanguage("Record","Caption","Save Recorded");
		m_strLanguage[1] = GetStringLanguage("Record","Static1","Record File save to this Folder:");
		m_strLanguage[2] = GetStringLanguage("Record","Button1","Change");
		m_strLanguage[3] = GetStringLanguage("Record","Button2","OK");
		m_strLanguage[4] = GetStringLanguage("Record","Button3","Connect");
		
	}
	else
	{
		m_strLanguage[0] = GetStringLanguage("Record","Caption","保存拍照文件");
		m_strLanguage[1] = GetStringLanguage("Record","Static1","把录制文件保存到此文件夹中:");
		m_strLanguage[2] = GetStringLanguage("Record","Button1","更改目录");
		m_strLanguage[3] = GetStringLanguage("Record","Button2","确定");
		m_strLanguage[4] = GetStringLanguage("Record","Button3","取消");
	}
	m_strLanguage[5] = GetStringLanguage("Record","Static4","请选择录制文件保存目录");
	SetDlgItemText(IDC_STATIC1,m_strLanguage[1]);
}

void CRecordDlg::InitRect()
{
	m_rtbutton.SetRect(m_rect.right-95,m_rect.top+90,m_rect.right-26,m_rect.top+112);
	m_rtclose.SetRect(m_rect.Width()-28,0,m_rect.Width(),20);
	m_rtcaption.SetRect(0,0,m_rect.Width()-30,20);
	m_rtcancel.SetRect(m_rect.right-74,m_rect.bottom-30,m_rect.right-5,m_rect.bottom-7);
	m_rtensure.SetRect(m_rect.right-157,m_rect.bottom-30,m_rect.right-84,m_rect.bottom-7);
}


void CRecordDlg::ReadMru()
{
	
	m_PathFileName = strAppDir + m_IniFileName;
	::GetPrivateProfileString("Recorded", "PathName", "D:\\My documents", m_PsPathName.GetBuffer(MAX_PATH),MAX_PATH, m_PathFileName);   //读取历史记录名
}


void CRecordDlg::WriteMru()
{

	::WritePrivateProfileString("Recorded", "PathName", m_PsPathName, m_PathFileName); //写路径名
	
}

void CRecordDlg::OnBrowse() 
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
		m_edit2 = szDir;
	}
	UpdateData(FALSE);
	m_numbtn = 0;
	InvalidateRect(&m_rtclose);
	InvalidateRect(&m_rtensure);
	InvalidateRect(&m_rtbutton);
	InvalidateRect(&m_rtcancel);

}

void CRecordDlg::EnSure() 
{
	// TODO: Add your control notification handler code here
	m_PsPathName=m_edit2;
	m_CurNum=1;
    WriteMru();
	CDialog::OnOK();
}

void CRecordDlg::OnLButtonDown(UINT nFlags, CPoint point) 
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

void CRecordDlg::OnMouseMove(UINT nFlags, CPoint point) 
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
			
			for (i=0;i<4;i++)
			{
				InvalidateRect(&rect[i]);
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

void CRecordDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtclose.PtInRect(point)&&m_close)
	{
		CDialog::OnCancel();
	}

	if(m_rtbutton.PtInRect(point)&&m_button)
	{
		OnBrowse(); 
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

void CRecordDlg::OnSize(UINT nType, int cx, int cy) 
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

BOOL CRecordDlg::OnEraseBkgnd(CDC* pDC) 
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
	memdc.DrawText(m_strLanguage[0],CRect(12,5,150,30),DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[2],m_rtbutton,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[3],m_rtensure,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[4],m_rtcancel,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pDC->BitBlt(0,0,m_rect.Width(),m_rect.Height(),&memdc,0,0,SRCCOPY);
	bmp.DeleteObject();
	memdc.DeleteDC();
	ReleaseDC(pDC);

	return true;
}

HBRUSH CRecordDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
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

