#ifndef _MPEG_CONTAIN_H
#define _MPEG_CONTAIN_H
#define   ADTS_HEAD                 7
#define   MP4_TIME_PRECISION        120       //时间精度1/120毫秒

UINT32  FillFtyp(STR_MPEG* pstMpg, UINT32  uiType);
INT8 FillMdatStart(STR_MPEG* pstMpg);
INT8 FillMdatEnd(STR_MPEG* pstMpg, UINT32 uiId, UINT32 uiType, STR_MP4INFO* stSys);
UINT32  FillFree(STR_MPEG* pstMpg);
UINT32  FillMoov(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
UINT32  FillMvhd(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
UINT32  FillTrak(STR_MPEG* pstMpg, UINT32 uiId,  STR_MP4INFO* stSys);
UINT32  FillTkhd(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
UINT32  FillMdia(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
UINT32  FillMdhd(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
UINT32  FillHdlr(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
UINT32  FillMinf(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
UINT32  FillVshd(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
UINT32  FillDinf(STR_MPEG* pstMpg, UINT32 uiId);
UINT32  FillDref(STR_MPEG* pstMpg, UINT32 uiId);
UINT32  FillUrl(STR_MPEG* pstMpg, UINT32 uiId, UINT32 uiType);
UINT32  FillStbl(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
UINT32  FillStsd(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
UINT32  FillStts(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
void  Stts_EFrame(UINT32 uiId, UINT32 uiTimetick, UINT32 uiCount, STR_MP4INFO* stSys);
UINT32  FillStss(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
void  Stss_EKeyFrame(UINT32 uiId, STR_MP4INFO* stSys);
UINT32  FillStsc(STR_MPEG* pstMpg, UINT32 uiId);
UINT32  FillStsz(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
void  Stsz_EFrame(UINT32 uiSize, UINT32 uiId, STR_MP4INFO* stSys);
UINT32  FillStco(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys);
void  Stco_EFrame(UINT32 uiSize, UINT32 uiId, STR_MP4INFO* stSys);
char* SysInit(char* pacDir, UINT32 uiStreamNum);
INT8 AddStream(UINT32 uiType, UINT32 uiWidth, UINT32 uiHeight, UINT32 uiId, UINT32 uiSampleRate,UINT32 uiStartTime, char* acSys);
INT8 CreatMpgContainer(UINT32 uiType, char* acSys);//(UINT32 uiType);
INT8 FinishMpgContainer(char* pacDir, UINT32 uiEnd, char* acSys);
INT8 WtOneFrame(char* pFrame, UINT32 uiFrameLen, UINT32 uiId, UINT32 uiType, UINT32 uiKey, UINT32 uiTimetick, char* acSys);
INT8 AddToFile(FILE* pSrc, FILE* pDst);
void SetStsdParam(UINT32 uiId, STR_MP4INFO* stSys);




#endif
