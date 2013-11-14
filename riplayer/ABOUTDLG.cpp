// ABOUTDLG.cpp : implementation file
//

#include "stdafx.h"
#include "Riplayer.h"
#include "ABOUTDLG.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CABOUTDLG dialog


CABOUTDLG::CABOUTDLG(CWnd* pParent /*=NULL*/)
	: CDialog(CABOUTDLG::IDD, pParent)
{
	//{{AFX_DATA_INIT(CABOUTDLG)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CABOUTDLG::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CABOUTDLG)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CABOUTDLG, CDialog)
	//{{AFX_MSG_MAP(CABOUTDLG)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CABOUTDLG message handlers

BOOL CABOUTDLG::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CABOUTDLG::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonDown(nFlags, point);
}
