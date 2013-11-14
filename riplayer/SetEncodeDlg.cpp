// SetEncodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Riplayer.h"
#include "SetEncodeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetEncodeDlg dialog


CSetEncodeDlg::CSetEncodeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetEncodeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetEncodeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSetEncodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetEncodeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetEncodeDlg, CDialog)
	//{{AFX_MSG_MAP(CSetEncodeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetEncodeDlg message handlers





