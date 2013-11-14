// SetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Riplayer.h"
#include "SetDlg.h"
#include "UserDlg.h"
#include "VedioDlg.h"
#include "VoiceDlg.h"
#include "StreamDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetDlg dialog


CSetDlg::CSetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetDlg::IDD, pParent)
{
	m_close=false;
	m_ensure=false;
	m_cancal=false;
	m_app=false;
	m_dlgFlag = 0;
	m_nInRect = 0;
	m_numbtn = 0;
	m_bensure = false;
	m_bclose = false;
	m_bInclose = false;
	m_bInensure = false;
	m_bIncancel = false;
	m_bcancel = false;
	//{{AFX_DATA_INIT(CSetDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CSetDlg::~CSetDlg()
{
	for (int i = 0; i < 10; i++)
	{
		m_strLanguage[i].IsEmpty();
	}
}
void CSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetDlg)
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetDlg, CDialog)
	//{{AFX_MSG_MAP(CSetDlg)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnClickList1)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
//extern bool g_Language;
/////////////////////////////////////////////////////////////////////////////
// CSetDlg message handlers
BOOL CSetDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	GetClientRect(m_rect);
	InitLanguage();
	InitRect();
	// TODO: Add extra initialization here
	/**********绑定对话框**************/
	CRect rect;
	user=new CUserDlg();
	user->Create(IDD_LOGIN,this); 
	GetDlgItem(IDC_STATIC_RECT)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	user->MoveWindow(rect);
	user->ShowWindow(SW_SHOW);

	vedio=new CVedioDlg();
	vedio->Create(IDD_VEDIO,this);
	vedio->MoveWindow(rect);
	
	voice=new CVoiceDlg();
	voice->Create(IDD_VOICE,this);
	voice->MoveWindow(rect);

	stream=new CStreamDlg();
	stream->Create(IDD_STREAM,this);
	stream->MoveWindow(rect);


	/*******设置列表风格*****/
	DWORD dwStyle;   
	dwStyle = m_ListCtrl.GetExtendedStyle();      
	dwStyle = dwStyle|LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES ;  
	m_ListCtrl.SetExtendedStyle(dwStyle);  

    m_ListCtrl.InsertColumn(0,_T("1111"),LVCFMT_CENTER,100); 

	/**********绑定SmallIcon和LargeIcon到CListCtrl上******/
	m_LargeIcon.Create(24,24, ILC_COLORDDB | ILC_MASK, 0, 4);
    //为子项加载图标
	m_LargeIcon.Add(AfxGetApp()->LoadIcon(IDI_ICON1));
	m_LargeIcon.Add(AfxGetApp()->LoadIcon(IDI_ICON2));
	m_LargeIcon.Add(AfxGetApp()->LoadIcon(IDI_ICON3));
	m_LargeIcon.Add(AfxGetApp()->LoadIcon(IDI_ICON4));

	m_ListCtrl.SetImageList(&m_LargeIcon,LVSIL_SMALL );  

	char *str[5]={0};

	for (int i=0;i<3;i++)
	{  
	  //名称
		str[i] = m_strLanguage[i+1].GetBuffer(m_strLanguage[i+1].GetLength()); //把CString 转换成char*
		m_strLanguage[i].ReleaseBuffer();
		LV_ITEM lvitem;//LV_ITEM结构体填充
		memset ((char *) &lvitem, '\0', sizeof (LV_ITEM));//结构体初始化
		lvitem.mask = LVIF_TEXT | LVIF_IMAGE  | LVIF_STATE;
		lvitem.iItem = i;
		lvitem.iSubItem = 0;
		lvitem.stateMask = 0;
		lvitem.iImage = i;          //显示不同的图标时，可以把一个int 变量赋给这个属性值
		lvitem.pszText = str[i];    //项名标题
		m_ListCtrl.InsertItem (&lvitem);
	}
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

void CSetDlg::InitLanguage()
{

	m_strLanguage[0] = GetStringLanguage("Encoded","Caption","编码器设置");
	m_strLanguage[1] = GetStringLanguage("Encoded","List1","用户管理:");
	m_strLanguage[2] = GetStringLanguage("Encoded","List2","视频管理");
	m_strLanguage[3] = GetStringLanguage("Encoded","List3","音频管理");
	m_strLanguage[4] = GetStringLanguage("Encoded","List4","码流管理");
	m_strLanguage[5] = GetStringLanguage("Encoded","Button1","确定");
	m_strLanguage[6] = GetStringLanguage("Encoded","Button2","取消");
	m_strLanguage[7] = GetStringLanguage("Encoded","Button3","应用");

}

void CSetDlg::InitRect()
{
	m_rtcancal.SetRect(m_rect.right-77,m_rect.bottom-32,m_rect.right-2,m_rect.bottom-4);
	m_rtensure.SetRect(m_rect.right-160,m_rect.bottom-32,m_rect.right-85,m_rect.bottom-4);
	m_rtclose.SetRect(m_rect.Width()-28,0,m_rect.Width(),20);
	m_rtcaption.SetRect(0,0,m_rect.Width()-32,35);
}
void CSetDlg::OnUser() 
{
	m_dlgFlag = 0;
	if(vedio!=NULL)
		vedio->ShowWindow(SW_HIDE);
	if(voice!=NULL)
		voice->ShowWindow(SW_HIDE);
	if(user==NULL)
	{
		CRect rect;
		user=new CUserDlg();
		user->Create(IDD_LOGIN,this);
		GetDlgItem(IDC_STATIC_RECT)->GetWindowRect(&rect);
		ScreenToClient(&rect);
		user->MoveWindow(rect);
		user->ShowWindow(SW_SHOW);
	}
	else
	{
		user->ShowWindow(SW_SHOW);
	}
    user->SetDlgItemText(IDC_IPADDRESS, g_strIpAddress);
	user->SetDlgItemText(IDC_EDIT1, g_strUserName);
	user->SetDlgItemText(IDC_EDIT5, g_strPassword);
	
}



void CSetDlg::OnVedio() 
{
	
	int ret = -1;
	CString strTemp;
	OutputVideoInfo inVideoInfo;
	int nFrameRate=0, nResolution=0, nBitrate=0;
	if(user!=NULL)
		user->ShowWindow(SW_HIDE);
	if(voice!=NULL)
		voice->ShowWindow(SW_HIDE);
	if(vedio==NULL)
	{
		CRect rect;
		vedio=new CVedioDlg();
		vedio->Create(IDD_VEDIO,this);
		GetDlgItem(IDC_STATIC_RECT)->GetWindowRect(&rect);
		ScreenToClient(&rect);
		vedio->MoveWindow(rect);
		vedio->ShowWindow(SW_SHOW);
	}
	else
	{
	
		vedio->ShowWindow(SW_SHOW);
	}
	m_dlgFlag = 1;
	ret = sdkGetVideoparam(client_socket,&inVideoInfo);
	if(ret != 0)
	{
		TRACE("ERROR,the error id =0x%x\n",ret);
	}
	nFrameRate = inVideoInfo.nFrameRate;
	nResolution = inVideoInfo.resolution;
	nBitrate = inVideoInfo.sBitrate;
	switch(inVideoInfo.resolution)
	{
	case 0:
		{
			strTemp = "Auto";
		}
		break;
	case 1:
		{
			strTemp = "1920X1080";
		}
		break;
	case 2:
		{
			strTemp = "1280X720";
		}
		break;
	case 3:
		{
			strTemp = "1400X1050";
		}
		break;
	case 4:
		{
			strTemp = "1366X768";
		}
		break;
	case 5:
		{
			strTemp = "1280X1024";
		}
		break;
	case 6:
		{
			strTemp = "1280X768";
		}
		break;
	case 7:
		{
			strTemp = "1024X768";
		}
		break;
	case 8:
		{
			strTemp = "720X480";
		}
		break;
	case 9:
		{
			strTemp = "352X288";
		}
		break;

	default:
		break;
	}
	CString str;
	str.Format("%d",nBitrate);
	vedio->SetDlgItemText(IDC_EDIT1,str);
	str.Format("%d",nFrameRate);
	vedio->SetDlgItemText(IDC_EDIT3,str);

	TRACE("resoultion= %d\n",nResolution);

	vedio->SettBox(strTemp);


}

int convert(UCHAR temp)
{
	switch(temp)
	{
	case 0:
		{
			temp = 0;
		}
		break;
	case 3:
		{
			temp = 1;
		}
		break;
	case 6:
		{
			temp = 2;
		}
		break;
	case 9:
		{
			temp = 3;
		}
		break;
	case 12:
		{
			temp = 4;
		}
		break;
	case 15:
		{
			temp = 5;
		}
		break;
	case 18:
		{
			temp = 6;
		}
		break;
	case 21:
		{
			temp = 7;
		}
		break;
	case 24:
		{
			temp = 8;
		}
		break;
	case 27:
		{
			temp = 9;
		}
		break;
	case 30:
		{
			temp = 10;
		}
		break;
	default :
		break;
	}
	return temp;
}

int transform(UCHAR temp)
{
	switch(temp)
	{
	case 0:
		{
			temp = 0;
		}
		break;
	case 1:
		{
			temp = 3;
		}
		break;
	case 2:
		{
			temp = 6;
		}
		break;
	case 3:
		{
			temp = 9;
		}
		break;
	case 4:
		{
			temp = 12;
		}
		break;
	case 5:
		{
			temp = 15;
		}
		break;
	case 6:
		{
			temp = 18;
		}
		break;
	case 7:
		{
			temp = 21;
		}
		break;
	case 8:
		{
			temp = 24;
		}
		break;
	case 9:
		{
			temp = 27;
		}
		break;
	case 10:
		{
			temp = 30;
		}
		break;
	default :
		break;
	}
	return temp;
}

void CSetDlg::OnVoice() //音频
{
	int ret = -1;
	AudioParam inputAudio;
	CString strTemp, strBuf,strInput,strlVolume,strrVolume;
	int sampleRate, bitRate,nInput;
	UCHAR lVolume,rVolume;
	if(user!=NULL)
		user->ShowWindow(SW_HIDE);
	if(vedio!=NULL)
		vedio->ShowWindow(SW_HIDE);
	/*if(stream!=NULL)
		stream->ShowWindow(SW_HIDE);*/
	if(voice==NULL)
	{
		CRect rect;
		voice=new CVoiceDlg();
		voice->Create(IDD_VOICE,this);
		GetDlgItem(IDC_STATIC_RECT)->GetWindowRect(&rect);
		ScreenToClient(&rect);
		voice->MoveWindow(rect);
		voice->ShowWindow(SW_SHOW);
	}
	else
	{
		voice->ShowWindow(SW_SHOW);
	}
	m_dlgFlag = 2;
	ret = sdkGetAudioParam(client_socket,&inputAudio);
	if(ret != 0)
	{
		TRACE("ERROR,the error id =0x%x\n",ret);
		
	}
	
	TRACE("bitrate:%d,sampleRate:%d,samplebit:%d\n",inputAudio.LVolume,inputAudio.RVolume,inputAudio.SampleBit);
	sampleRate = inputAudio.SampleRate;
	switch(sampleRate)
	{
/*	case 2:
		{
			strTemp = "44.1KHz";
		}
		break;*/
	case 3:
		{
			strTemp = "48KHz";
		}
		break;
	default:
		break;
	}

	bitRate = inputAudio.BitRate;
	switch(bitRate)
	{
	case 64000:
		{
			strBuf = "64kbps";
		}
		break;
	case 96000:
		{
			strBuf = "96kbps";
		}
		break;
	case 128000:
		{
			strBuf = "128kbps";
		}
		break;
	default:
		break;
	}
	nInput = inputAudio.InputMode;
	switch(nInput)
	{
	case 0:
		 {
			  strInput="Line in";
		 }
		  break;
	case 1:
		{
			strInput = "Mic";
		}
		break;
	default:
		break;
	}
	lVolume = inputAudio.LVolume;
	lVolume = convert(lVolume);
	rVolume = inputAudio.RVolume;
	rVolume = convert(rVolume);
	strlVolume.Format("%d", lVolume);
	strrVolume.Format("%d", rVolume);

	voice->SettBox(strTemp);	
	voice->setcombox(strBuf);
	voice->SetView(strInput);
	voice->setLVolume(strlVolume);
	voice->setRVolume(strrVolume);

}

void CSetDlg::OnStream() 
{

	if(user!=NULL)
		user->ShowWindow(SW_HIDE);
	if(voice!=NULL)
		voice->ShowWindow(SW_HIDE);
	if(vedio!=NULL)
		vedio->ShowWindow(SW_HIDE);
	if(stream==NULL)
	{
		CRect rect;
		stream=new CStreamDlg();
		stream->Create(IDD_STREAM,this);
		GetDlgItem(IDC_STATIC_RECT)->GetWindowRect(&rect);
		ScreenToClient(&rect);
		stream->MoveWindow(rect);
		stream->ShowWindow(SW_SHOW);
	}
	else
	{
		stream->ShowWindow(SW_SHOW);
	}
	//GetDlgItem(IDC_STREAM)->EnableWindow(FALSE);
}



void CSetDlg::OnClickList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	int iItem;
	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	if (pos == NULL)
    {
		;
	}
	else
    {
        while (pos)
        {
            iItem = m_ListCtrl.GetNextSelectedItem(pos);//选中第几行，获得行号
			switch (iItem)
			{
				case 0: 
					{
						CSetDlg::OnUser();
				
					}
					break;
				case 1: 
					{
						if (user->m_bConnect)
						{
							CSetDlg::OnVedio();
						}
					}
					break;
				case 2: 
					{
						if (user->m_bConnect)
						{
							CSetDlg::OnVoice();
						}
					}
					break;
              /*  case 3: 
					CSetDlg::OnStream();
					break;*/
				default:
					break;

			}
        }
    }
	*pResult = 0;
}


void CSetDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtcaption.PtInRect(point)) 
	{
		PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x, point.y));	//向系统发送HTCAPTION消息，让系统以为鼠标点在标题栏上
	}
	if(m_rtclose.PtInRect(point))
	{
		m_close=true;
	}
	
	if(m_rtensure.PtInRect(point))
	{	
		m_ensure=true;
		
	}
	if(m_rtcancal.PtInRect(point))
	{
		
		m_cancal=true;
		
	}
	if(m_rtapp.PtInRect(point))
	{
		
		m_app=true;
		
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CSetDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect[4];
	rect[0]=m_rtensure;
	rect[1]=m_rtclose;
	rect[2]=m_rtcancal;
	if(m_rtclose.PtInRect(point))
	{
		m_bclose= true;
		m_bInclose = true;
		
	}
	else
	{
		m_bclose = false;
		
	}
	if (m_bclose)//进入矩形区域，刷新
	{
		InvalidateRect(&m_rtclose);
	}
	else
	{
		if (m_bInclose)//进入矩形区域内后离开，刷新
		{
			InvalidateRect(&m_rtclose);
			m_bInclose = false;
		}
		
	}
	if(m_ensure)
	{
		if(!m_rtensure.PtInRect(point))
		{
			m_ensure=false;
		}
	}
	if(m_rtensure.PtInRect(point))
	{
		m_bensure = true;
		m_bInensure = true;
		
	}
	else
	{
		m_bensure = false;
		
	}
	if (m_bensure)//进入矩形区域，刷新
	{
		InvalidateRect(&m_rtensure);
	}
	else
	{
		if (m_bInensure)//进入矩形区域内后离开，刷新
		{
			InvalidateRect(&m_rtensure);
			m_bInensure = false;
		}
		
	}
	if(m_rtcancal.PtInRect(point))
	{
		m_bcancel= true;
		m_bIncancel = true;
		
	}
	else
	{
		m_bcancel = false;
		
	}
	if (m_bcancel)//进入矩形区域，刷新
	{
		InvalidateRect(&m_rtcancal);
	}
	else
	{
		if (m_bIncancel)//进入矩形区域内后离开，刷新
		{
			InvalidateRect(&m_rtcancal);
			m_bIncancel = false;
		}
		
	}
	if(m_ensure)
	{
		if(!m_rtensure.PtInRect(point))
		{
			m_ensure=false;
		}
	}
	
	if(m_close)
	{
		if(!m_rtclose.PtInRect(point))
		{
			m_close=false;
		}
	}
	if(m_cancal)
	{
		if(!m_rtcancal.PtInRect(point))
		{
			m_cancal=false;
		}
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CSetDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_rtclose.PtInRect(point)&&m_close)
	{
		sdkCloseTCPConnet(client_socket);
		CDialog::OnCancel();
	}
	if(m_rtcancal.PtInRect(point)&&m_cancal)
	{
		sdkCloseTCPConnet(client_socket);
		CDialog::OnCancel();
	}

	if(m_rtensure.PtInRect(point)&&m_ensure)
	{
		OnEnsure();
		
	}

	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CSetDlg::OnEraseBkgnd(CDC* pDC) 
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


	m_DlgPic.Load(IDR_JPG_DIALOG1);
	if(m_DlgPic != NULL)
		m_DlgPic.Render(&memdc,m_rect);

	if (m_bensure) //确定
	{
		m_DlgPic.Load(IDR_JPG_BTN2_2);
	} 
	else
	{
		m_DlgPic.Load(IDR_JPG_BTN2);
			
	}
	m_DlgPic.Render(&memdc,m_rtensure);
	if (m_bcancel)  //取消
	{
		m_DlgPic.Load(IDR_JPG_BTN2_2);
	} 
	else
	{
		m_DlgPic.Load(IDR_JPG_BTN2);
		
	}
	m_DlgPic.Render(&memdc,m_rtcancal);
	if (m_bclose)
	{
		m_DlgPic.Load(IDR_CLOSEX1);
	} 
	else
	{
		m_DlgPic.Load(IDR_CLOSEX);
		
	}
	m_DlgPic.Render(&memdc,m_rtclose);



	CFont   *pOldfont = (CFont *)memdc.SelectObject(&m_font); 
  
	memdc.DrawText(m_strLanguage[0],CRect(m_rect.left+8,5,m_rect.left+150,30),DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[5],m_rtensure,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memdc.DrawText(m_strLanguage[6],m_rtcancal,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pDC->BitBlt(0,0,m_rect.Width(),m_rect.Height(),&memdc,0,0,SRCCOPY);
	bmp.DeleteObject();
	memdc.DeleteDC();
	ReleaseDC(pDC);
	return true;

}

void CSetDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	CRgn m_rgn;
    CRect rc;
	GetWindowRect(&rc);
	rc-=rc.TopLeft();
	m_rgn.CreateRoundRectRgn(rc.left,rc.top,rc.right,rc.bottom,5,5);
	SetWindowRgn(m_rgn,TRUE);
}
void CSetDlg::OnEnsure()//确定
{
	if (user->m_bConnect)
	{
		CString strbuff;
		CString strTemp[10], strBit[3];
		CString strSampleRate, strBitRate,strInput, strlVolume, strrVolume;
		int ret = -1;
		int i = 0, nBitRate = 0, nFrameRate = 0, AudioInput = 0;
		int resolution = 0;
		UCHAR lVolume, rVolume;
		OutputVideoInfo inVideoInfo,outVideoInfo;
		
		AudioParam      inAudioInfo,outAudioInfo;
		int AudioSampleRate =0;
		int AudioBitRate = 0;
		strTemp[0] = "Auto";
		strTemp[1] = "1920X1080";
		strTemp[2] = "1280X720";
		strTemp[3] = "1400X1050";
		strTemp[4] = "1366X768";
		strTemp[5] = "1280X1024";
		strTemp[6] = "1280X768";
		strTemp[7] = "1024X768";
		strTemp[8] = "720X480";
		strTemp[9] = "352X288";
		
		ret = sdkGetVideoparam(client_socket,&inVideoInfo);
		if(ret != 0)
		{
			TRACE("ERROR,the error id =0x%x\n",ret);
		}
		
		
		ret = sdkGetAudioParam(client_socket,&inAudioInfo);
		if(ret != 0)
		{
			TRACE("ERROR,the error id =0x%x\n",ret);
			
		}
		if (m_dlgFlag == 1) //设置视频参数
		{
			vedio->GetDlgItemText(IDC_EDIT1,strbuff); //码率
			nFrameRate = atoi(strbuff);
			if (nFrameRate < 128)
			{
				CMsgBoxDlg dlg;
				dlg.SetMsg(GetStringLanguage("MsgBox", "Caption1", "警告"), GetStringLanguage("MsgBox", "Tip6", "码率最小值为128"));
				dlg.DoModal();
				return ;
			}
			else if (nFrameRate > 20000)
			{
				CMsgBoxDlg dlg;
				dlg.SetMsg(GetStringLanguage("MsgBox", "Caption1", "警告"), GetStringLanguage("MsgBox", "Tip7", "码率最大值为20000"));
				dlg.DoModal();
				return ;
			}
			inVideoInfo.sBitrate = nFrameRate;
			
			vedio->GetDlgItemText(IDC_EDIT3,strbuff); //帧率
			nBitRate = atoi(strbuff);
			if (nBitRate  < 1  )
			{
				CMsgBoxDlg dlg;
				dlg.SetMsg(GetStringLanguage("MsgBox", "Caption1", "警告"), GetStringLanguage("MsgBox", "Tip7", "码率最小值为1"));
				dlg.DoModal();
				return;
			}
			else if (nBitRate >60)
			{
				CMsgBoxDlg dlg;
				dlg.SetMsg(GetStringLanguage("MsgBox", "Caption1", "警告"), GetStringLanguage("MsgBox", "Tip8", "帧率最大值为60"));
				dlg.DoModal();
				return ;
			}
			
			inVideoInfo.nFrameRate = nBitRate;		
			vedio->GetDlgItem(IDC_COMBO1)->GetWindowText(strbuff);	
			for (i = 0; i < 10; i++)
			{
				if (strbuff == strTemp[i])
				{
					resolution = i;
					break;
				}
			}
			
			inVideoInfo.resolution = resolution;
			ret = sdkSetVideoparam(client_socket, &inVideoInfo, &outVideoInfo);
			if (ret < 0)
			{
				TRACE("ret = %d\n",ret);
			}
			else
			{
				CMsgBoxDlg dlg;
				dlg.SetMsg(GetStringLanguage("MsgBox", "Caption2", "提示"), GetStringLanguage("MsgBox", "Tip4", "操作成功"));
				dlg.DoModal();
			}
		}
		else if (m_dlgFlag == 2) //设置音频参数
		{
			voice->GetDlgItem(IDC_COMBO1)->GetWindowText(strSampleRate);
			/*if (strSampleRate == "44.1KHz")
			{
			AudioSampleRate = 2;
		}*/
			if (strSampleRate == "48KHz")
			{
				AudioSampleRate = 3;
			}
			voice->GetDlgItem(IDC_COMBO2)->GetWindowText(strBitRate);
			if (strBitRate == "64kbps")
			{
				AudioBitRate = 64000;
			}
			else if (strBitRate == "96kbps")
			{
				AudioBitRate = 96000;
			}
			else if (strBitRate == "128kbps")
			{
				AudioBitRate = 128000;
			}
			voice->GetDlgItem(IDC_COMBO3)->GetWindowText(strInput);
			if (strInput == "Line in")
			{
				AudioInput = 0;
			}
			else if (strInput == "Mic")
			{
				AudioInput = 1;
			}
			voice->GetDlgItem(IDC_COMBO4)->GetWindowText(strlVolume);
			lVolume = atoi(strlVolume);
			lVolume = transform(lVolume);
			voice->GetDlgItem(IDC_COMBO5)->GetWindowText(strrVolume);
			rVolume = atoi(strrVolume);
			rVolume = transform(rVolume);
			
			inAudioInfo.SampleRate = AudioSampleRate;
			inAudioInfo.BitRate = AudioBitRate;
			inAudioInfo.InputMode = AudioInput;
			inAudioInfo.LVolume = lVolume;
			inAudioInfo.RVolume = rVolume;
			ret = sdkSetAudioParam(client_socket, &inAudioInfo, &outAudioInfo);
			if (ret < 0)
			{
				TRACE("ret = %d\n",ret);
			}
			else
			{
				CMsgBoxDlg dlg;
				dlg.SetMsg(GetStringLanguage("MsgBox", "Caption2", "提示"), GetStringLanguage("MsgBox", "Tip4", "操作成功"));
				dlg.DoModal();
			}
		}
	}

}

BOOL CSetDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN) // 屏蔽掉回车
	{
	//	user->OnConnect();
		return NULL;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

