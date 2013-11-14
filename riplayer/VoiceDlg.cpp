// VoiceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Riplayer.h"
#include "VoiceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVoiceDlg dialog


CVoiceDlg::CVoiceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVoiceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVoiceDlg)
	//}}AFX_DATA_INIT
}


void CVoiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVoiceDlg)
	DDX_Control(pDX, IDC_COMBO5, m_ctrCombox5);
	DDX_Control(pDX, IDC_COMBO4, m_ctrCombox4);
	DDX_Control(pDX, IDC_COMBO3, m_ctrCombox3);
	DDX_Control(pDX, IDC_COMBO2, m_ctrCombox2);
	DDX_Control(pDX, IDC_COMBO1, m_ctrCombox1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVoiceDlg, CDialog)
	//{{AFX_MSG_MAP(CVoiceDlg)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVoiceDlg message handlers
BOOL CVoiceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	InitLanguage();
	// TODO: Add extra initialization here
	/****** �������� ***********/
	if (g_Language)
	{
		m_font.CreatePointFont(88, _T("Verdana"));
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


void CVoiceDlg::InitLanguage()
{

	if (g_Language)
	{
		m_strLanguage[0] = GetStringLanguage("Audio","Static1","Sample_Rate:");
		m_strLanguage[1] = GetStringLanguage("Audio","Static2","Code_Rate:");
		m_strLanguage[2] = GetStringLanguage("Audio","Static3","Input:");
		m_strLanguage[3] = GetStringLanguage("Audio","Static4","Audio level left: ");
		m_strLanguage[4] = GetStringLanguage("Audio","Static5","Audio level right: ");
		
		
	}
	else
	{
		m_strLanguage[0] = GetStringLanguage("Audio","Static1","������:");
		m_strLanguage[1] = GetStringLanguage("Audio","Static2","����:");
		m_strLanguage[2] = GetStringLanguage("Audio","Static3","��Ƶ����:");
		m_strLanguage[3] = GetStringLanguage("Audio","Static4","����������:");
		m_strLanguage[4] = GetStringLanguage("Audio","Static5","����������:");
	}
	SetDlgItemText(IDC_STATIC1,m_strLanguage[0]);
	SetDlgItemText(IDC_STATIC2,m_strLanguage[1]);
	SetDlgItemText(IDC_STATIC3,m_strLanguage[2]);
	SetDlgItemText(IDC_STATIC4,m_strLanguage[3]);
	SetDlgItemText(IDC_STATIC5,m_strLanguage[4]);
}

void CVoiceDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CVoiceDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnMouseMove(nFlags, point);
}

void CVoiceDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CVoiceDlg::OnEraseBkgnd(CDC* pDC) 
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
	CBitmap* pOldBmp = memdc.SelectObject(&bmp);            //��λͼѡ�뵽�ڴ���ʾ�豸��			
	DWORD style = DT_CENTER|DT_SINGLELINE|DT_VCENTER;
	
	CBrush bru(RGB(239,255,255));
	CRect cr(0,0,rect.Width(),rect.Height());
	memdc.FillRect(&cr,&bru); 	//��䱳����

	/*switch(m_numbtn)
	{
	case 0:

		break;
	case 1:	
	break;
	case 2:		m_DlgPic.Load(IDR_JPG_CLOSE1);
				if(m_DlgPic != NULL)
					m_DlgPic.Render(&memdc,m_rtclose);
	
		break;
	default:
		break;
	}*/

	pDC->BitBlt(0,0,m_rect.Width(),m_rect.Height(),&memdc,0,0,SRCCOPY);
	bmp.DeleteObject();
	memdc.DeleteDC();
	ReleaseDC(pDC);
	return true;
}

HBRUSH CVoiceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if(nCtlColor == CTLCOLOR_STATIC) 
	{ 
		//IDC_STATICΪ����Ҫ��Ϊ͸���Ŀؼ�ID 
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



void CVoiceDlg::AddString(CString str)
{
	CString strBox[2];
	if (str=="")
	{
		return;
	}
	int iCount = ((CComboBox*)GetDlgItem(IDC_COMBO1))->GetCount();
	if (iCount < 1)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO1))->AddString(str);
		iCount ++;
		return;
	}
	else if (iCount >=2)
	{
		return;
	}
	TRACE("iCount == %d\n", iCount);
	for (int i =0;  i< iCount; i++)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO1))->GetLBText(i,strBox[i]);
		if (str == strBox[i])
		{
			break;
			
		}
		
	}
	if (i<iCount)
	{
		return;
	}
	else
	{
		((CComboBox*)GetDlgItem(IDC_COMBO1))->AddString(str);		
	}
	
	
}

void CVoiceDlg::AddCombox(CString str)
{
	CString strBox[3];
	if (str=="")
	{
		return;
	}
	int iCount = ((CComboBox*)GetDlgItem(IDC_COMBO2))->GetCount();
	if (iCount < 1)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO2))->AddString(str);
		iCount ++;
		return;
	}
	else if (iCount >=3)
	{
		return;
	}
	TRACE("iCount == %d\n", iCount);
	for (int i =0;  i< iCount; i++)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO2))->GetLBText(i,strBox[i]);
		if (str == strBox[i])
		{
			break;
			
		}
		
	}
	if (i<iCount)
	{
		return;
	}
	else
	{
		((CComboBox*)GetDlgItem(IDC_COMBO2))->AddString(str);		
	}
	
	
}

void CVoiceDlg::SettBox(CString str)
{
/*	CString strbox[10];
	int iCount = ((CComboBox*)GetDlgItem(IDC_COMBO1))->GetCount();
	for (int i = 0; i < iCount; i++)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO1))->GetLBText(i,strbox[i]);
		if (str == strbox[i])
		{
			((CComboBox*)GetDlgItem(IDC_COMBO1))->SetCurSel(i);//���õ�n������Ϊ��ʾ������
			
		}
	}*/
	CString strTemp;
	((CComboBox*)GetDlgItem(IDC_COMBO1))->GetLBText(0,strTemp);
	if (str == strTemp)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO1))->SetCurSel(0);//���õ�n������Ϊ��ʾ������
		
	}

	
}
void CVoiceDlg::setcombox(CString str)
{
	CString strbox[10];
	int iCount = ((CComboBox*)GetDlgItem(IDC_COMBO2))->GetCount();
	for (int i = 0; i < iCount; i++)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO2))->GetLBText(i,strbox[i]);
		if (str == strbox[i])
		{
			((CComboBox*)GetDlgItem(IDC_COMBO2))->SetCurSel(i);//���õ�n������Ϊ��ʾ������
			
		}
	}
}
void CVoiceDlg::SetView(CString str)
{
	CString strbox[10];
	int iCount = ((CComboBox*)GetDlgItem(IDC_COMBO3))->GetCount();
	for (int i = 0; i < iCount; i++)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO3))->GetLBText(i,strbox[i]);
		if (str == strbox[i])
		{
			((CComboBox*)GetDlgItem(IDC_COMBO3))->SetCurSel(i);//���õ�n������Ϊ��ʾ������
			
		}
	}


}
void CVoiceDlg::setLVolume(CString str)
{
	CString strbox[16];
	int iCount = ((CComboBox*)GetDlgItem(IDC_COMBO4))->GetCount();
	for (int i = 0; i < iCount; i++)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO4))->GetLBText(i,strbox[i]);
		if (str == strbox[i])
		{
			((CComboBox*)GetDlgItem(IDC_COMBO4))->SetCurSel(i);//���õ�n������Ϊ��ʾ������
			
		}
	}
}
void CVoiceDlg::setRVolume(CString str)
{
	CString strbox[16];
	int iCount = ((CComboBox*)GetDlgItem(IDC_COMBO5))->GetCount();
	for (int i = 0; i < iCount; i++)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO5))->GetLBText(i,strbox[i]);
		if (str == strbox[i])
		{
			((CComboBox*)GetDlgItem(IDC_COMBO5))->SetCurSel(i);//���õ�n������Ϊ��ʾ������
			
		}
	}
}
BOOL CVoiceDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN||VK_ESCAPE) // ���ε��س�
	{
		return NULL;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CVoiceDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	delete this;
	CDialog::PostNcDestroy();
}
