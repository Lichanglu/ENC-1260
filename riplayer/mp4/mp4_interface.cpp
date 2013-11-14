/*根据New RecService_7.1.0.12 中asf.cpp生成函数接口，计划封装如下：

BOOL CAsf::CreateAsfFile(char *pchName)    
根据pchName(路径和文件名)生成文件

char* CAsf::GetCurFileName() 
返回生成的文件名和路径

BOOL CAsf::Start() 
开始录制文件

void CAsf::Close() 
录制完成，关闭文件

int CAsf::GetFrameCount() 
返回当前文件帧数？无对应索引号，应该是第几路？

BOOL CAsf::WriteStreamSample(int nStreamNum, int nLen, BYTE *pData, QWORD qwTime, BOOL bKey) 
nStreamNum，流号，由对应索引决定？音视频类型也有该值确定？
nLen，帧长度
pData，数据地址
qwTime，时间
bKey，关键帧

BOOL CAsf::AddStream(int nBitRate,WM_MEDIA_TYPE * pType) 
nBitRate，无效，任意值
pType，"reach_audio" 或"reach_video" 字符串
根据AddStream次数得到nStreamNum*/

#include "mp4_common.h"
#include "mp4_contain.h"
#include "mp4_release.h"
#include "mp4_interface.h"
CMp4::CMp4()
{
	memset(m_strFile, 0, 128);
	m_nStreamIndex = 0;
	m_nChannelIndex = 0;
	m_bKeyFrame = 0;
	m_dwStartTime = 0;
	m_nFrameCount = 0;
	m_nState = 1;
}

CMp4::~CMp4()
{
	memset(m_strFile, 0, 128);
	m_nState = 0;

}

BOOL CMp4::CreateMp4File(char *pchName, DWORD uiStreamNum)    
{
	m_stMp4Info = SysInit(pchName, uiStreamNum);
	if(m_stMp4Info == NULL)
	{
		m_nState = 0;
		return FALSE;
	}
	
	CreatMpgContainer(0, m_stMp4Info);
	
	strcpy(m_strFile, pchName);
	
	return TRUE;
}

char* CMp4::GetCurFileName() 
{
	return m_strFile;
}

BOOL CMp4::Start() 
{
	if(m_nState)
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CMp4::AddOneStream(DWORD uiType, DWORD uiWidth, DWORD uiHeight, DWORD uiId, DWORD uiSamplerate)
{
	BOOL  bRet = 0;
	DWORD uiStartTime = timeGetTime();

	bRet = AddStream(uiType, uiWidth, uiHeight, uiId, uiSamplerate, uiStartTime, m_stMp4Info);
	return bRet;
}


int CMp4::WriteStreamSample(int uiId, int nLen, BYTE *pData, DWORD qwTime, BOOL bKey) 
{
	int  bRet = 0;
	bRet = WtOneFrame((char*)pData, uiId, nLen, 0, bKey, qwTime, m_stMp4Info);
	return bRet;
}

#if 0
//========================read part=========================================
//return null:err
BOOL CMp4::LoadStart(char* acFile)
{
	m_stMp4Release = MP4R_LoadStart(acFile);
	if(!m_stMp4Release)
	{
		return FALSE;
	}
	return TRUE;
}

void CMp4::LoadFinish()
{
	MP4R_LoadFinish(m_stMp4Release);
}

//uitime, micro second
void CMp4::LoadSeek(DWORD uiTime)
{
	MP4R_Seek(m_stMp4Release, uiTime);
}

//pacBuf:frame buff
//type: 0:video 1:audio
BOOL  CMp4::ReadOneFrame(char* pacBuf, UINT32* puiLen, UINT32* puiTimestamp, UINT32* puiType)
{
	UINT32 uiRet = 0;
	uiRet = MP4R_ReadOneFrame(pacBuf, (STR_MP4_RELEASE_INFO*) m_stMp4Release, puiLen, puiTimestamp, puiType);
	if(uiRet)
	{
		return TRUE;
	}
	return FALSE;

}

#endif

UINT32 CMp4::Mp4ReadInterface(UINT8 ucCommand, UINT32 uiTime, char* acFile, CALLBACK_MP4READ CallbackMp4read) 
{
	UINT32 uiDura = 0;

	uiDura = MP4R_FuncInterface( ucCommand,  uiTime,  acFile,  &m_stMp4ParamPass, CallbackMp4read);

	if(ucCommand == MP4_COMMAND_START)
	{
		return uiDura;
	}
	return 0;
}

