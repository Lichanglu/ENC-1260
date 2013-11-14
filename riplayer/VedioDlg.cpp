// VedioDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Riplayer.h"
#include "VedioDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVedioDlg dialog


CVedioDlg::CVedioDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVedioDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVedioDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CVedioDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVedioDlg)
	DDX_Control(pDX, IDC_COMBO1, m_ctrCombox1);
	DDX_Control(pDX, IDC_EDIT3, m_ctrEdit3);
	DDX_Control(pDX, IDC_EDIT1, m_ctrEdit1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVedioDlg, CDialog)
	//{{AFX_MSG_MAP(CVedioDlg)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVedioDlg message handlers
BOOL CVedioDlg::OnInitDialog() 
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


void CVedioDlg::InitLanguage()
{

	if (g_Language)
	{
		m_strLanguage[0] = GetStringLanguage("Vedio","Static1","Code Rate:");
		m_strLanguage[1] = GetStringLanguage("Vedio","Static2","Frame Rate:");
		m_strLanguage[2] = GetStringLanguage("Vedio","Static3","Output_Resolution:");
	
		
	}
	else
	{
		m_strLanguage[0] = GetStringLanguage("Vedio","Static1","����:");
		m_strLanguage[1] = GetStringLanguage("Vedio","Static2","֡��:");
		m_strLanguage[2] = GetStringLanguage("Vedio","Static3","����ֱ���:");
	}
	SetDlgItemText(IDC_STATIC1,m_strLanguage[0]);
	SetDlgItemText(IDC_STATIC2,m_strLanguage[1]);
	SetDlgItemText(IDC_STATIC3,m_strLanguage[2]);
}


BOOL CVedioDlg::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	GetWindowRect(&rect);
	GetClientRect(&m_rect);
	CDC memdc;     //����һ����ʾ�豸���� 
	CBitmap bmp;       //����һ��λͼ���� 
	m_rtclose.SetRect(m_rect.left+72,m_rect.top+35,m_rect.left+141,m_rect.top+57);
	memdc.CreateCompatibleDC(pDC);     //��������Ļ��ʾ���ݵ��ڴ���ʾ�豸 
	bmp.CreateCompatibleBitmap(pDC,rect.Width(),rect.Height());//����һ������Ļ��ʾ���ݵ�λͼ
	memdc.SetBkMode(TRANSPARENT);
	CBitmap* pOldBmp = memdc.SelectObject(&bmp);            //��λͼѡ�뵽�ڴ���ʾ�豸��		
	
	DWORD style = DT_CENTER|DT_SINGLELINE|DT_VCENTER;
	CBrush bru(RGB(239,255,255));
	CRect cr(0,0,rect.Width(),rect.Height());
	memdc.FillRect(&cr,&bru); 	//��䱳����


	pDC->BitBlt(0,0,m_rect.Width(),m_rect.Height(),&memdc,0,0,SRCCOPY);
	bmp.DeleteObject();
	memdc.DeleteDC();
	ReleaseDC(pDC);
	return true;
}

HBRUSH CVedioDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
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
/*void CVedioDlg::AddString(CString str)
{
	int iCount=((CComboBox*)GetDlgItem(IDC_COMBO1))->GetCount();
	����
 
}*/
void CVedioDlg::AddString(CString str)
{
	CString strBox[10];
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
	else if (iCount >=10)
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
void CVedioDlg::SettBox(CString str)
{
	CString strbox[16];
	int iCount = ((CComboBox*)GetDlgItem(IDC_COMBO1))->GetCount();
	for (int i = 0; i < iCount; i++)
	{
		((CComboBox*)GetDlgItem(IDC_COMBO1))->GetLBText(i,strbox[i]);
		if (str == strbox[i])
		{
			((CComboBox*)GetDlgItem(IDC_COMBO1))->SetCurSel(i);//���õ�n������Ϊ��ʾ������
			
		}
	}

}

BOOL CVedioDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN||VK_ESCAPE) // ���ε��س�
	{
		return NULL;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CVedioDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	delete this;
	CDialog::PostNcDestroy();
}
