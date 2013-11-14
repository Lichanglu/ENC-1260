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
	m_IniFileName = _T("SavePathName.ini");	//����ini�ļ���
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
	GetDlgItem(IDC_EDIT1)->EnableWindow(false); //�ñ༭��ʧȥ����
	m_strPath=m_PsPathName;

	SetDlgItemText(IDC_STATIC1,m_strLanguage[1]);
	UpdateData(FALSE);
	/****** �������� ***********/
	if (g_Language)
	{
		m_font.CreatePointFont(90, "Verdana");
	} 
	else
	{
		m_font.CreatePointFont(100,"����");
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
		m_strLanguage[0] = GetStringLanguage("Picture","Caption","���������ļ�");
		m_strLanguage[1] = GetStringLanguage("Picture","Static1","������ͼƬ���浽���ļ�����:");
		m_strLanguage[2] = GetStringLanguage("Picture","Button1","����Ŀ¼");
		m_strLanguage[3] = GetStringLanguage("Picture","Button2","ȷ��");
		m_strLanguage[4] = GetStringLanguage("Picture","Button3","ȡ��");
	}
	m_strLanguage[5] = GetStringLanguage("Picture","Static3","��ѡ��ͼƬ����Ŀ¼");
}

void CPictureDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	char szDir[MAX_PATH];
	BROWSEINFO bi;
	ITEMIDLIST *pidl;
	//���BROWSEINFO�ṹ��
	bi.hwndOwner = this->m_hWnd; // ָ�������ڣ��ڶԻ�����ʾ�ڼ䣬�����ڽ������� 
	bi.pidlRoot = NULL; // ���ָ��NULL������"����"Ϊ�� 
	bi.pszDisplayName = szDir;
	bi.lpszTitle = m_strLanguage[5]; // ��һ�н���ʾ�ڶԻ���Ķ��� 
	bi.ulFlags = BIF_STATUSTEXT  | BIF_RETURNONLYFSDIRS  ;//�ڶԻ���������½��ļ��а�ť
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;
	//��ʾ�Ի���
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
//��ȡini�ļ��еļ�¼
void CPictureDlg::ReadMru()
{

	m_PathFileName = strAppDir + m_IniFileName;
	::GetPrivateProfileString("Picture", "PathName", "D:\\My documents", m_PsPathName.GetBuffer(MAX_PATH),MAX_PATH, m_PathFileName);   //��ȡ��ʷ��¼��
}

//��ʷ��¼д��ini�ļ�
void CPictureDlg::WriteMru()
{
	::WritePrivateProfileString("Picture", "PathName", m_PsPathName, m_PathFileName); //д·����
	
}


void CPictureDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtcaption.PtInRect(point)) 
	{
		PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x, point.y));	//��ϵͳ����HTCAPTION��Ϣ����ϵͳ��Ϊ�����ڱ�������
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
	if (m_numbtn)//�����������ˢ��
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
		if (m_nInRect)//������������ں��뿪��ˢ��
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
	CDC memdc;     //����һ����ʾ�豸���� 
	CBitmap bmp;       //����һ��λͼ���� 
	memdc.CreateCompatibleDC(pDC);     //��������Ļ��ʾ���ݵ��ڴ���ʾ�豸 
	bmp.CreateCompatibleBitmap(pDC,rect.Width(),rect.Height());//����һ������Ļ��ʾ���ݵ�λͼ
	memdc.SetBkMode(TRANSPARENT);
//	memdc.FillSolidRect(0,0,rect.Width(),rect.Height(),RGB(93,250,255)); 	//�ñ���ɫ��λͼ����ɾ�
	CBitmap* pOldBmp = memdc.SelectObject(&bmp);            //��λͼѡ�뵽�ڴ���ʾ�豸��		
	
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
		//IDC_STATICΪ����Ҫ��Ϊ͸���Ŀؼ�ID 
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
	/********* ���öԻ���߽�ΪԲ�� *******/
	CRgn m_rgn;
    CRect rc;
	GetWindowRect(&rc);
	rc-=rc.TopLeft();
	m_rgn.CreateRoundRectRgn(rc.left,rc.top,rc.right,rc.bottom,5,5);
	SetWindowRgn(m_rgn,TRUE);
}
