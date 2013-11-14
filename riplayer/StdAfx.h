// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A7B61E32_9892_4E57_97BF_1AE08B422FAD__INCLUDED_)
#define AFX_STDAFX_H__A7B61E32_9892_4E57_97BF_1AE08B422FAD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include "Stream\record.h"

#pragma comment(lib,"RMedia.lib  ") //���ſ��ƿ�
#pragma comment(lib,"SaveAsJpg.lib ") //���տ�
#pragma comment(lib,"winmm.lib")// �������ƿ�
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

void GetAppDir();//��ȡ�ļ�·��
CString GetStringLanguage(CString strAppName,CString strKeyName,CString strDefault); //��ȡĬ������ 

extern CString strAppDir; //�ļ�·��
extern CString strLanguageFile;//�ļ���
extern int g_Language;//��������
extern  bool m_recording;  //�Ƿ�¼��
extern record_handle  *r_handle; //¼�ƽӿھ��
extern char filename[64];
extern CString g_strUrl; //����URL��ַ
extern UINT g_Lost_frame;  // ��ʧ֡��
extern UINT g_Total_frame; //��Ƶ��֡��
extern UINT g_Frame_rate; //֡��
extern UINT g_Bit_rate; //λ����
extern UINT g_Samplerate;
extern CString g_strIpAddress;
extern CString g_strUserName;
extern CString g_strPassword;
extern int  prv_nWidth;
extern int prv_nHieght;

extern int client_socket;  //TCP_socket;

#endif // !defined(AFX_STDAFX_H__A7B61E32_9892_4E57_97BF_1AE08B422FAD__INCLUDED_)
