// TipDlg.cpp : implementation file
//

#include "stdafx.h"
#include "riplayer.h"
#include "TipDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define _countof(arr)	(sizeof(arr)/sizeof(arr[0]))

int m_nNewTop;
int m_nWidth;
int m_nMaxFontCy;
/////////////////////////////////////////////////////////////////////////////
// CTipDlg dialog


CTipDlg::CTipDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTipDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTipDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_layerColor = RGB(0,0,0);

}


void CTipDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTipDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTipDlg, CDialog)
	//{{AFX_MSG_MAP(CTipDlg)
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
CRect main_rect;
/////////////////////////////////////////////////////////////////////////////
// CTipDlg message handlers
BOOL CTipDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	CRect   Rect; 
	GetWindowRect(&Rect); 	
	SetWindowPos(&wndTopMost, 0, 0, Rect.Width(), Rect.Height(), SWP_NOMOVE);
	SetTimer(1,5500,NULL);
//	OnTip(m_tip);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CTipDlg::OnTip(CString str)
{

	m_tip = str;

}
void CTipDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == 1)
	{
		OnTip(m_tip);
		OnCancel();
	}
	
	CDialog::OnTimer(nIDEvent);
}

void CTipDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType,cx,cy);
}



int CTipDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if(CDialog::OnCreate(lpCreateStruct)==-1)
		return -1;
	SetWindowLong(this->GetSafeHwnd(),GWL_EXSTYLE,GetWindowLong(this->GetSafeHwnd(),GWL_EXSTYLE)^0x80000);
	HINSTANCE hInst=LoadLibrary("User32.dll");
	if(hInst)
	{
		typedef BOOL (WINAPI* MYFUNC)(HWND,COLORREF,BYTE,DWORD);
		MYFUNC fun=NULL;
		fun=(MYFUNC)GetProcAddress(hInst,"SetLayeredWindowAttributes"); //从DLL中导出"SetLayeredWindowAttributes"函数
		if(fun)
			fun(this->GetSafeHwnd(),m_layerColor,0,0x1 );//m_layerColor 为掩码颜色，0为透明度，0全透明，255为不透明;0x1时掩码颜色处变为透明，透明度值无效
		FreeLibrary(hInst);
	}

	return 0;
}

void CTipDlg::OnPaint() 
{
	//CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting
		
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);
		
		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		
		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		CRect rect;
		GetClientRect(&rect);
		dc.FillSolidRect(rect,m_layerColor);
		
		//设置字体
		CFont font;
		font.CreateFont(
			26,                        // nHeight
			0,                         // nWidth
			0,                         // nEscapement
			0,                         // nOrientation
			FW_BOLD,					// nWeight
			FALSE,                     // bItalic
			FALSE,                     // bUnderline
			0,                         // cStrikeOut
			GB2312_CHARSET,              // nCharSet
			OUT_DEFAULT_PRECIS,        // nOutPrecision
			CLIP_DEFAULT_PRECIS,       // nClipPrecision
			DEFAULT_QUALITY,           // nQuality
			DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
			"宋体");                 // lpszFacename
		
		CFont* def_font = dc.SelectObject(&font);
		dc.SetTextColor(RGB(
			abs(255-GetRValue(m_layerColor)),
			abs(255-GetGValue(m_layerColor)),
			abs(255-GetBValue(m_layerColor))
			));
		
		//得到字体尺寸
		CSize sz = dc.GetTextExtent(m_tip); 
		dc.TextOut(rect.left+(rect.Width()-sz.cx)/2, rect.top+(rect.Height()-sz.cy)/2, m_tip);
		
		dc.SelectObject(def_font);
		
		CDialog::OnPaint();
	}
	
}

BOOL CTipDlg::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	return /*CDialog::OnEraseBkgnd(pDC)*/true;
}

void CTipDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	delete   this; 
	CDialog::PostNcDestroy();
}
