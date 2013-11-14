#ifndef _COMMON_H
#define _COMMON_H
#include <stdio.h>

//#include <basetsd.h>
//#include <afxsock.h>
#include <winsock2.h>
//#include <stdarg.h>
//#include <iostream.h>

typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef signed __int64      INT64, *PINT64;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned __int64    UINT64, *PUINT64;
//mp4 add
//name
#define           FTYP        0x70797466
#define           FREE        0x65657266    //0x66726565;
#define           MDAT       0x7461646d    //0x6d646174;
#define           MOOV       0x766f6f6d    //0x6d6f6f76;
#define           MVHD       0x6468766d    //0x6d766864;
#define           TRAK        0x6b617274    //0x7472616b;
#define           TKHD        0x64686b74    //0x746b6864;

#define           MDHD       0x6468646d    //0x6d646864;
#define           HDLR        0x726c6468    //0x68646c72;
#define           MDIA        0x6169646d    //0x6d646961
#define           MINF        0x666e696d    //0x6d696e66;
#define           VMHD       0x64686d76    //0x766d6864;
#define           DINF        0x666e6964    //0x64696e66;
#define           DREF        0x66657264    //0x64726566;
#define           STBL        0x6c627473    //0x7374626c;
#define           STSD       0x64737473    //0x73747364;
#define           AVC1       0x31637661    //0x61766331;
#define           AVCC       0x43637661    //0x61766343;
#define           STTS       0x73747473    //0x73747473;
#define           STSS       0x73737473    //0x73747373;
#define           STSC       0x63737473    //0x73747363;
#define           STSZ       0x7a737473    //0x7374737a;
#define           STCO       0x6f637473    //0x7374636f;
#define           ISOM        0x6d6f7369    //0x69736f6d;
#define           ISO2        0x326f7369    //0x69736f32;
#define           MP41        0x3134706d    //0x6d703431;
#define           UDTA        0x61746475
#define           VIDE         0x65646976
#define           SOUN        0x6e756f73
#define           SMHD        0x64686d73
#define           URL_         0x206c7275
#define           MP4A        0x6134706d;    //0x6d703461

#define           FTYP_R        0x66747970
#define           FREE_R        0x66726565
#define           MDAT_R       0x6d646174
#define           MOOV_R      0x6d6f6f76
#define           MVHD_R       0x6d766864
#define           TRAK_R        0x7472616b
#define           TKHD_R        0x746b6864
#define           MEIA_R        0x6d646961
#define           MDHD_R       0x6d646864
#define           HDLR_R        0x68646c72
#define           MDIA_R        0x6d646961
#define           MINF_R        0x6d696e66
#define           VMHD_R       0x766d6864
#define           DINF_R        0x64696e66
#define           DREF_R        0x64726566
#define           STBL_R        0x7374626c
#define           STSD_R       0x73747364
#define           AVC1_R       0x61766331
#define           AVCC_R       0x61766343
#define           STTS_R       0x73747473
#define           STSS_R       0x73747373
#define           STSC_R       0x73747363
#define           STSZ_R       0x7374737a
#define           STCO_R       0x7374636f
#define           ISOM_R        0x69736f6d
#define           ISO2_R        0x69736f32
#define           MP41_R        0x6d703431
#define           VIDE_R         0x76696465
#define           SOUN_R        0x736F756E
#define           SMHD_R        0x736D6864
#define           URL__R         0x75726C20
#define           MP4A_R        0x6d703461


//param
#define          MAX_STREAM                                        2
#define          MAX_DIRLEN                                         256
#define          MAX_INDIR                                           128
#define          MAX_FRAMEHEAD_READ                         128
#define          MAX_SLICE_INFRAME                             5
#define          HEAD_SIZE_8BYTE                                 8
#define          OFFSET_TO_START                                0x30
#define           OFFSET_TO_START_AUDIO                   0x2c
#define          ONE_BYTE_ZERO_TAIL                           1
#define          ADTS_LEN                                             7

//file
#define          DIR_MPEG                    ".mp4"
#define          DIR_MDAT                    ".mp4"
#define          DIR_STSS                     ".stss.tmp"
#define          DIR_STSC                     ".stsc.tmp"
#define          DIR_STSZ                     ".stsz.tmp"
#define          DIR_STCO                     ".stco.tmp"
#define          DIR_STTS                     ".stts.tmp"


#define          CTIME                      0x80b0257c             //0x789abcde 
#define          FRAME_PERIOD         60  //300                       // 2400 
#define          FRAME_RATE             25
#define          RATE_NORMAL           0x10000                  //0x10000
#define          VOLUME_MAX            0x100                     // 0x100
#define          TK_NO_RELATION        0                        //default
#define          LAN_DEFAULT             0xc455  // 1// 
#define          V_HAND                    "VideoHandler"
#define          A_HAND                    "SoundHandler"
#define          FLAG_1                     1
#define          FLAG_0                     0

//MVHD
#define          SCALE_ONE_MSEC      0x3e8             // 1000
#define          SCALE_ONE_MSEC_44K1      22050 //0xac44   //44100 //
#define          SCALE_ONE_MSEC_48K      24000 //48000

#define          SCALE_44100_FRAMERATE    43
#define          MVHD_NEXT                2
//MDHD
#define          MDHD_FRAME_RATE     25

//VSHD
#define          VMHD_GRAPMODE      0
#define          VMHD_OPCOLOR        0
#define          VMHD_BALANCE         0
//DREF
#define          DREF_COUNT             1
//URL
#define          URL_NONE                  1
#define          URL_STRING                           "nothing"
//STSD
#define          STSD_INDEX               1                              //video  always  1
#define          STSD_CODNAME         0x31637661                //avc1
#define          STSD_HORIZ               0x00480000               //72 dpi
#define          STSD_VERT                0x00480000               //72 dpi
#define          STSD_FRAMECNT         1
#define          STSD_DEPTH              0x18                         //image in color without alpha
#define          STSD_PREDEF             0xffff
#define          STSD_EXTLEN_V             30
#define          STSD_DESCR30               30
#define          STSD_DESCR31               31
#define          STSD_EXTLEN_A             0x27

#define          STSD_SAMPLE_SIZE     16
#define          STSD_CHANNEL             2

//STTS
#define          STTS_CNT                    1
#define          STTS_AUDIO_SAMPLE       0x400

//STSC
#define          STSC_CNT                    1
#define          STSC_FST_CHK              1
#define          STSC_SAMPLE_PCHK      1
#define          STSC_SAMPLE_INDEX    1
//STSZ
#define          STSZ_SAMPLE_SIZE       0      


enum _enuPix
{
	PIX640X480 = 0,
	PIX704X576,
	PIX720X480,
	PIX800X600,
	PIX1024X576,
	PIX1024X768,
	PIX1280X720,
	PIX1280X960,
	PIX1366X768,
	PIX1280X1024,
	PIX1400X1050,
};

enum _enuLayer
{
	LAYER0 = 0,
	LAYER1 = 1,
	LAYER2 = 2,
};

enum _enuTkhdFlg
{
	TRAK_ENABLE = 1,
	TRAK_IN_MOVIE = 2,
	TRACK_IN_PREV = 4,
	TRACK_FM_VALUE = 0xf,
};

enum _enuAvtype
{
    VIDEO = 0,
    AUDIO,
    AVMIX,
};

enum _enuVer
{
	VER_0 = 0,
	VER_1 = 1,
};

typedef struct _strParVideo
{
	UINT32    uiNum;
	UINT8     aucPara[0x30];
}STR_PARV;

typedef struct _strMpgParam
{
	UINT8       aucStsdVedio[0x30];
	UINT8       aucStsdAudio[0x30];
	STR_PARV  st640480;
	STR_PARV  st800600;
	STR_PARV  st704576;
	STR_PARV  st720480;
	STR_PARV  st720576;
	STR_PARV  st1024576;
	STR_PARV  st1024768;
	STR_PARV  st1280720;
	STR_PARV  st1280960;
	STR_PARV  st1366768;
	STR_PARV  st12801024;
	STR_PARV  st14001050;
	STR_PARV  st19201088;
}STR_MPGPARAM;

typedef struct _strftyp
{
	UINT32    uiSize;
	UINT32    uiType;
	UINT32    auiString[6];
}STR_FTYP;

typedef struct _strStreamInfo
{
	UINT32    uiType;
	UINT32    uiStarttime;
	UINT32    uiEndtime;
	double    uiLasttime;
	UINT32    uiWidth;
	UINT32    uiHeight;
	UINT32    uiFrameCnt;
	UINT32    uiKeyFrameCnt;
	UINT16    usSampleRate;
	//UINT16    usRealSample;
	UINT16    usFrameRate;
	//UINT16    usResv;
	UINT32    uiOffset;                            //64 bit
	double     dGain;
	FILE*      pStss;
	UINT32    uiStssPos;
	FILE*      pStsc;
	UINT32    uiStscPos;
	FILE*      pStsz;
	UINT32    uiStszPos;
	FILE*      pStco;
	UINT32    uiStcoPos;
	FILE*      pStts;
	UINT32    uiSttsPos;
}STR_STREAMINFO;

typedef struct _strStsz
{
    UINT32    uiSize;
    UINT32    uiType;
    UINT32    uiVer;
    UINT32    uiSampleSize;
    UINT32    uiSampleCnt;
    //UINT32    uiEntryCnt;
}STR_STSZ;


typedef struct _strStscChunk
{
	UINT32    uiFstChunk;
	UINT32    uiSamplePerChunk;
	UINT32    uiSampleDescIndex;
}STR_STSCCHK;


typedef struct _strStsc
{
	UINT32    uiSize;
	UINT32    uiType;
	UINT32    uiVer;
	UINT32    uiEntryCnt;
	STR_STSCCHK    stChunk;
}STR_STSC;


typedef struct _strStco
{
	UINT32    uiSize;
	UINT32    uiType;
	UINT32    uiVer;
	UINT32    uiEntryCnt;
}STR_STCO;


typedef struct _strStss
{
	UINT32    uiSize;
	UINT32    uiType;
	UINT32    uiVer;
	UINT32    uiEntryCnt;
}STR_STSS;


typedef struct _strmvhd
{
	UINT32    uiSize;
	UINT32    uiType;
	UINT8      ucVer;
	UINT8      aucFlag[3];
	UINT32     uiCreatTime;
	UINT32     uiModTime;
	UINT32     uiTimeScale;
	UINT32     uiDura;
	UINT32     uiRate;
	UINT16     usVolume;
	UINT8      aucResv[10];
	UINT32     auiMatrix[9];
	UINT32     auiPredef[6];
	UINT32     uiNextTrack;
}STR_MVHD;

typedef struct _strTkhd
{
	UINT32    uiSize;
	UINT32    uiType;
	UINT8     ucVer;
	UINT8     aucFlag[3];
	UINT32    uiCreatTime;
	UINT32    uiModTime;
	UINT32    uiTrack;
	UINT32    uiResv;
	UINT32    uiDura;
	UINT8      aucResv[8];
	UINT16    usLay;
	UINT16    usAlte;
	UINT16    usVolume;
	UINT16    usResv;
	UINT32    auiMatrix[9];
	UINT32    uiWidth;
	UINT32    uiHeight;
}STR_TKHD;

typedef struct _strMdhd
{
	UINT32    uiSize;
	UINT32    uiType;
	UINT8     ucVer;
	UINT8     aucFlag[3];
	//UINT32    uiVer1Fill1;
	UINT32    uiCreatTime;
	//UINT32    uiVer1Fill2;
	UINT32    uiModTime;
	UINT32    uiTimeScale;
    	//UINT32    uiVer1Fill3;
	UINT32    uiDura;
	UINT16    usLang;
	UINT16    usPreDef;
}STR_MDHD;

typedef struct _strHdlr
{
	UINT32    uiSize;
	UINT32    uiType;
	UINT8     ucVer;
	UINT8     aucFlag[3];
	UINT32    uiPreDef;
	UINT32    uiHdType;
	UINT8     aucResv[12];
	char       acName[12];    //以\0结尾的字符串
}STR_HDLR;


typedef struct _strVmhd
{
	UINT32      uiSize;
	UINT32      uiType;
	UINT8        ucVer;
	UINT8        aucFlag[3];
	UINT16       uiGrapMode;
	UINT16      ausOpcolor[3];
}STR_VMHD;

typedef struct _strSmhd
{
	UINT32       uiSize;
	UINT32       uiType;
	UINT8         ucVer;
	UINT8         aucFlag[3];
	UINT16       usBalan;
	UINT16       usResv;
}STR_SMHD;

typedef struct _strSttsSample
{
	UINT32    uiSampleCnt;
	UINT32    uiSampleDelta;
}STR_STTS_SAMPLE;

typedef struct _stUrl
{
	UINT32    uiSize;
	UINT32    uiType;
	char       acUrl[0x20];
}STR_URL;

#define          STSD_EXTLEN                       0x30
typedef struct _strStsdAudio
{
	UINT32      uiSize;
	UINT32      uiType;
	INT8         acResv[6];
	UINT16      usIndex;
	UINT32      auiResv[2];
	UINT16      usChannel;
	UINT16      usSampleSize;
	UINT16     usPredef;
	UINT16     usResv;
	UINT32     uiSampleRate;
 	UINT32     uiExtLen;      // ISO上没有这一段，应该是根据PREDEF值生成的扩展 
	char         acExt[STSD_EXTLEN];    // 同上   
}STR_STSDAUDIO;

typedef struct _strStts
{
	UINT32    uiSize;
	UINT32    uiType;
	UINT32    uiVer;
	UINT32    uiEntryCnt;
	//STR_STTS_SAMPLE    stSample[4];
}STR_STTS;


typedef struct _stDref
{
	UINT32    uiSize;
	UINT32    uiType;
	UINT8     ucVer;
	UINT8     aucFlag[3];
	UINT32    uiCnt;
	STR_URL  stUrl;
}STR_DREF;

typedef struct _strDinf
{
	UINT32        uiSize;
	UINT32        uiType;
	STR_DREF    stDref;
}STR_DINF;


typedef union _unMfhd
{
    STR_VMHD    stVmhd;
    STR_SMHD    stSmhd;
}UN_MFHD;


typedef struct _strStsdVideo
{
	UINT32      uiSize;
	UINT32      uiType;
	INT8         acResv[6];
	UINT16      usIndex;
	UINT16      usPredef;
	UINT16      usResv;
	UINT32      auiPredef[3];
	UINT16      usWidth;
	UINT16      usHeight;
	UINT32      uiHoriz;
	UINT32      uiVert;
	UINT32      uiResv;
	UINT16      usFrameCnt;
	INT8         acName[32];
	UINT16      usDepth;
	UINT16        sPredef;
	UINT32      uiExtLen;      // ISO上没有这一段，应该是根据PREDEF值生成的扩展 
	INT8          acExt[STSD_EXTLEN];    // 同上   
}STR_STSDVIDEO;


typedef struct _strStsdV
{
	UINT32      uiSize;
	UINT32      uiType;
	INT8         acResv[6];
	UINT16      usIndex;
	STR_STSDVIDEO stVideo;
}STR_STSDV;

typedef struct _strStsdA
{
	UINT32      uiSize;
	UINT32      uiType;
	INT8         acResv[6];
	UINT16      usIndex;
	STR_STSDAUDIO  stAudio;
}STR_STSDA;

typedef union _enuStsd
{
    STR_STSDV    stStsdv;
    STR_STSDA    stStsdA;
}UN_STSD;

typedef struct _strStbl
{
	UINT32        uiSize;
	UINT32        uiType;
	UN_STSD     stStsd;
	STR_STTS    stStts;
	STR_STSS    stStss;
	STR_STSC    stStsc;
	STR_STSZ    stStsz;
	STR_STCO    stStco;
}STR_STBL;

typedef struct _strMinf
{
	UINT32         uiSize;
	UINT32         uiType;
	UN_MFHD      unMfhd;
	STR_DINF      stDinf;
	STR_STBL      stStbl;

}STR_MINF;

typedef struct _strMdia
{
	UINT32        uiSize;
	UINT32        uiType;
	STR_MDHD    stMdhd;
	STR_HDLR     stHdlr;
	STR_MINF     stMinf;
}STR_MDIA;


typedef struct _strTrack
{
	UINT32        uiSize;
	UINT32        uiType;
	STR_TKHD    stTkhd;
	STR_MDIA    stMdia;
}STR_TRAK;

typedef struct _strMoov
{
	UINT32    uiSize;
	UINT32    uiType;
	STR_MVHD    stMvhd;
	STR_TRAK     stTrak[MAX_STREAM];
}STR_MOOV;


typedef struct _strMdat
{
	UINT32    uiSize;
	UINT32    uiType;
	INT8*     pcDat;
}STR_MDAT;

typedef struct _strFree
{
	UINT32    uiSize;
	UINT32    uiType;
}STR_FREE;

typedef struct _strMpeg
{
    STR_FTYP    stFtyp;
    STR_FREE    stFree;
    STR_MDAT   stMdat;
    STR_MOOV   stMoov;
}STR_MPEG;


typedef struct _stTime
{
	UINT32      uiYear;
	UINT32      uiMonth;
	UINT32      uiDay;
	UINT32      uiHour;
	UINT32      uiMinute;
	UINT32      uiSecond;
	char          acLog[30];
}STR_TIME;

typedef struct _strMp4Info
{
	UINT32                  uiStreamNum;
	UINT32                  uiFstFrame;
	UINT32                  uiSliceflag;
	UINT32                  uiLastNum;
	char                      acFileName[128];
	STR_TIME              stTime;
    	FILE*                    pMpg;
	char*                    pucDatbuf;
	STR_STREAMINFO    astStream[MAX_STREAM];
	STR_MPGPARAM      stMpgParam;
	STR_MPEG              astMpg;
}STR_MP4INFO;

typedef struct   _strAdtsHead
{
	unsigned   syncword  : 12 ;
	unsigned   ID: 1 ;
	unsigned   layer: 2 ;
	unsigned   protection_absent: 1 ;
	unsigned   profile: 2 ;
	unsigned    sampling_frequency_index: 4 ;
	unsigned   private_bit: 1 ;
	unsigned   channel_configuration: 3 ;
	unsigned   original: 1 ;
	unsigned   home: 1 ;
	unsigned   copyright_identification_bit: 1 ;
	unsigned   copyright_identification_start: 1 ;
	unsigned   frame_length: 13 ;
	unsigned   adts_buffer_fullness: 11 ;
	unsigned   number_of_raw_data_blocks_in_frame: 2 ;
}STR_ADTS_HEAD; 

void CreatLogFile();
//void _PrintInfo_ERROR(char* pChar, ...);
UINT32 Swap32(UINT32 uiNum);
UINT16 Swap16(UINT16 usNum);
UINT64 Swap64(UINT64 ulNum);
INT8 CreatTmpFile(char* pacDir, UINT32 uiType, UINT32 uiId, STR_MP4INFO* stSys);
INT8 DelTmpFile(char* pacDir, UINT32 uiType, UINT32 uiId, STR_MP4INFO* stSys);
INT8 WtFile(INT8* pacSrc, UINT32 uiLen, FILE* pFile);
INT8  ConvertSliceLen(char* pSlice, UINT32 uiSliceLen);
UINT32 CalcSliceLen(char* pFrame, UINT32 uiFrameLen, UINT32* pauiSliceLen);
UINT16 NumberToAscii(UINT32 uiValue);


#endif
