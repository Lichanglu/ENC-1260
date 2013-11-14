// SaveAsJpg.h : main header file for the SAVEASJPG DLL
//

#if !defined(AFX_SAVEASJPG_H__BB69E25F_5138_4CBB_BBF3_82C0C2917F29__INCLUDED_)
#define AFX_SAVEASJPG_H__BB69E25F_5138_4CBB_BBF3_82C0C2917F29__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif


//#include "PictureOperate.h"

#define DLLEXPORT_API  extern "C"__declspec(dllexport) 

DLLEXPORT_API void  SaveBmpImage( BYTE *pFrameRGB, int width, int height,int bpp,char* filePath);    
DLLEXPORT_API void SaveJpgImage(BYTE *pFrameRGB, int width, int height,int bpp,char* filePath);    
#endif // !defined(AFX_SAVEASJPG_H__BB69E25F_5138_4CBB_BBF3_82C0C2917F29__INCLUDED_)
