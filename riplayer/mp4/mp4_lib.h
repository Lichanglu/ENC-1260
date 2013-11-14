#ifndef _MP4_LIB
#define _MP4_LIB

#define MP4_CHANNEL_TEST 2
/********************************************

********************************************/




#define MP4_VIDEO     0
#define MP4_AUDIO     1
#define MP4_MAXLEN    0xfe000000

//typedef unsigned char  INT8;
//typedef unsigned int   UINT32;

extern char* SysInit(char* pacDir, UINT32 uiStreamNum);
extern INT8 AddStream(UINT32 uiType, UINT32 uiWidth, UINT32 uiHeight, UINT32 uiId, UINT32 uiSampleRate,UINT32 uiStartTime, char* acSys);
extern INT8 CreatMpgContainer(UINT32 uiType, char* acSys);
extern INT8 FinishMpgContainer(char* pacDir, UINT32 uiEnd, char* acSys);
extern INT8 WtOneFrame(char* pFrame, UINT32 uiFrameLen, UINT32 uiId, UINT32 uiType, UINT32 uiKey, UINT32 uiTimetick, char* acSys);
extern INT8 AddToFile(FILE* pSrc, FILE* pDst);
//extern void SetStsdParam(UINT32 uiId, STR_MP4INFO* stSys);







#endif
