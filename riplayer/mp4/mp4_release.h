#ifndef _MP4_RELEASE_H
#define _MP4_RELEASE_H

#define  MP4_READ_LEN_PER_TIME_BYTE         96000//12000
#define  MP4_READ_LEN_PER_TIME_UINT32    24000//3000

typedef struct stMp4CurFrameInfo
{
	UINT32  uiLen;
	UINT32  uiPos;
	UINT32  uiTime;
	UINT32  uiType;
	UINT32  uiCycle;
	UINT32  uiCycleGain;
    	UINT32  uiTimeStamp;
}STR_MP4_CUR_FRAME_INFO;

typedef struct stMp4InfoPerRead
{
	UINT32  uiTotal;                             //总个数
	UINT32  uiNum;                              //已用个数
	UINT32  uiStartPosInFile;               //开始地址，用于SEEK
	UINT32  uiNumPerTime;                  //每次读到的总长度中已用个数
	UINT32  uiTotalPerTime;                //每次读到的总长度
	UINT32  uiCurPosInFile;                 //当前地址，用于SEEK
	UINT32  uiLeftPerBlock;                 //STTS需要，多少个以后刷新TIMESTAMP的cycle
	UINT32  aucDat[MP4_READ_LEN_PER_TIME_UINT32];
}STR_MP4_INFO_PER_READ;

typedef struct stMp4StreamInfo
{
	UINT32  uiDura;
	UINT32  uiFrameNum;
	UINT32  uiKeyNum;
}STR_MP4_STREAM_INFO;

// stream0:video    stream1:audio
typedef struct stMp4ReleaseInfo
{
	UINT32  uiHeight;
	UINT32  uiWidth;
	UINT32  uiSamplerate;
	FILE*    pFile;
	UINT32  uiFilelen;
	UINT16  usStreamNum;
	UINT16  usStreamType;
	UINT32  uiDura;
	UINT32  uiCurFilePos;
	UINT32  uiTimeScale;
	UINT32  uiFramePeriod;
	UINT32  uiLoadStartTime;
	INT32  iCorrectSeektime;
	UINT8    ucCommandState;
	UINT8    aucResv[3];
	UINT32  uiSetSeektime;
	UINT32  uiPausetimeStart;
	UINT32  uiPausetimeTotal;
	UINT32  uiFrameCnt;
	STR_MP4_STREAM_INFO      stStreamInfo[2];
	STR_MP4_INFO_PER_READ    stStts[2];
	STR_MP4_INFO_PER_READ    stStss[2];
	STR_MP4_INFO_PER_READ    stStsz[2];
	STR_MP4_INFO_PER_READ    stStco[2];
	STR_MP4_CUR_FRAME_INFO  stCurrent;
	STR_MP4_CUR_FRAME_INFO  stVideo;
	STR_MP4_CUR_FRAME_INFO  stAudio;
}STR_MP4_RELEASE_INFO;

typedef int (__stdcall *CALLBACK_MP4READ)(const unsigned int, const unsigned int, const  unsigned int, const unsigned int, const char*,
                  const unsigned int, const unsigned int, const unsigned int);

typedef struct stMp4Parampass
{
    STR_MP4_RELEASE_INFO*  pstMp4info;
    char*                             pBuff;
    CALLBACK_MP4READ          pCallback;
}STR_MP4_PARAM_PASS;

enum _enuMp4Command
{
	MP4_COMMAND_STOP = 0,
	MP4_COMMAND_START = 1,
	MP4_COMMAND_GOON = 2,
	MP4_COMMAND_PAUSE = 3,
	MP4_COMMAND_SEEK = 4,
};


UINT32  MP4R_VideoHeadRecover(char* pacBuf, STR_MP4_RELEASE_INFO* pstMp4RInfo);
UINT32  MP4R_AudioHeadRecover(char* pacBuf, STR_MP4_RELEASE_INFO* pstMp4RInfo);
UINT32  MP4R_CalcTimestamp(STR_MP4_RELEASE_INFO* pstMp4RInfo);
UINT32  MP4R_FreshCurStream(STR_MP4_RELEASE_INFO* pstMp4RInfo);
UINT32  MP4R_FreshCurPos(STR_MP4_RELEASE_INFO* pstMp4RInfo);
UINT32  MP4R_FreshCurLen(STR_MP4_RELEASE_INFO* pstMp4RInfo);
UINT32  MP4R_FreshStreamStruct(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiTime);
//UINT32  MP4R_ReadOneFrame(char* pacBuf, STR_MP4_RELEASE_INFO* pstMp4RInfo,
//	UINT32* puiLen, UINT32* puiTimestamp, UINT32* puiType);
DWORD  WINAPI  MP4R_ReadOneFrame(LPVOID  lparam);
UINT32  MP4R_LoadHeadInfo(FILE* pFile, UINT32* puiType);
UINT32  MP4R_LoadTkhd(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiCacheLen);
void  MP4R_LoadMvhd(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiCacheLen);
void  MP4R_LoadMdhd(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiCacheLen);
void MP4R_LoadStts(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiType, UINT32 uiLoadLen);
void MP4R_LoadStss(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiType, UINT32  uiLoadLen, UINT32 uiOffset);
void MP4R_LoadStsz(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiType, UINT32 uiLoadLen, UINT32 uiOffset);
void MP4R_LoadStco(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiType, UINT32 uiLoadLen, UINT32 uiOffset);
UINT32  MP4R_LoadCertainInfo(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiStreamType);
char* MP4R_LoadStart(char* acFile);
void MP4R_LoadFinish(char* acMp4RInfo);
void MP4R_Seek(char* acMp4RInfo, UINT32 uiTime);
UINT32 MP4R_IndentIFrame(STR_MP4_RELEASE_INFO* pstMp4RInfo);
UINT32 MP4R_FuncInterface(UINT8 ucCommand, UINT32 uiTime, char* acFile, UINT32* Mp4Info, CALLBACK_MP4READ CallbackMp4read);
#endif