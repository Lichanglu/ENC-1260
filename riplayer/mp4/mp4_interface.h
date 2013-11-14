#ifndef _MP4_INTERFACE_H
#define _MP4_INTERFACE_H

#include "mp4_release.h"
class  __declspec(dllexport) CMp4
{
public:
	CMp4();
	virtual ~CMp4();
	BOOL CreateMp4File(char *pchName, DWORD uiStreamNum);
	//BOOL CreateBroadCast(DWORD dwPort,UINT nMaxClient);
	//BOOL CreatePushMediaServer(char *pchPushURL);
	BOOL Start();
	void Close();
	//DWORD GetBroadCastClient();
	BOOL AddOneStream(DWORD uiType, DWORD uiWidth, DWORD uiHeight, DWORD uiId, DWORD uiSamplerate);//(int nBitRate,WM_MEDIA_TYPE * pType);
	int GetStreamIndex();
	void SetStreamIndex(int nStreamIndex);
	int GetChannelIndex();
	void SetChannelIndex(int nChannelIndex);

	//int GetClipIndex();
	//void SetClipIndex(int nClipIndex);

	int WriteStreamSample(int nStreamNum, int nLen, BYTE *pData, DWORD qwTime, BOOL bKey);

	//void SetUseClock(BOOL bUseClock);
	int GetFrameCount();
	char* GetCurFileName();

	//¶Áº¯Êý
	//BOOL LoadStart(char* acFile);
	//void LoadFinish();
	//void LoadSeek(DWORD uiTime);
	//BOOL ReadOneFrame(char* pacBuf, UINT32* puiLen, UINT32* puiTimestamp, UINT32* puiType);
	unsigned int Mp4ReadInterface(unsigned char ucCommand, unsigned int uiTime, char* acFile, CALLBACK_MP4READ CallbackMp4read);
	unsigned int m_stMp4ParamPass;


private:
	char m_strFile[128];
	int m_nStreamIndex;
	int m_nState;
	int m_nChannelIndex;
	BOOL m_bKeyFrame;
	DWORD m_dwStartTime;
	int m_nFrameCount;
	char* m_stMp4Info;
	char* m_stMp4Release;

};

#endif
