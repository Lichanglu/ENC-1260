// stdafx.cpp : source file that includes just the standard includes
//	Riplayer.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

int g_Language =0;  //����ѡ��
bool m_recording=0;
record_handle *r_handle =NULL;
char filename[64]={0};
CString strAppDir;
CString strLanguageFile;
CString g_strUrl;
int prv_nWidth=0;  //��ǰ�ֱ��ʵĿ�
int prv_nHieght =0 ; //��ǰ�ֱ��ʵĸ�
CString g_strIpAddress;
CString g_strUserName;
CString g_strPassword;
int client_socket = -1;
UINT g_Lost_frame = 0;  // ��ʧ֡��
UINT g_Total_frame=0; //��Ƶ��֡��
UINT g_Frame_rate=0; //֡��
UINT g_Bit_rate=0; //λ����
UINT g_Samplerate=0;

void GetAppDir() //Ѱ���ļ�·��
{
	char buf[MAX_PATH];
	memset(buf,0,MAX_PATH);
	GetModuleFileName(NULL,buf,MAX_PATH);
	strAppDir = buf;
	int index = strAppDir.ReverseFind('\\');
	if (index > 0)
		strAppDir = strAppDir.Left(index + 1);

}

CString GetStringLanguage(CString strAppName,CString strKeyName,CString strDefault) //��ȡĬ����������
{
	char buf[100];
	memset(buf,0,100);
	GetPrivateProfileString(strAppName,strKeyName,strDefault,buf,100,strLanguageFile);
	return buf;
}


