#ifndef __MPEGTS_H_
#define __MPEGTS_H_
#if 0
#include "stream.h"

#define TS_PACKET_SIZE 188

#define STREAM_TYPE_VIDEO_MPEG1     0x01
#define STREAM_TYPE_VIDEO_MPEG2     0x02
#define STREAM_TYPE_AUDIO_MPEG1     0x03
#define STREAM_TYPE_AUDIO_MPEG2     0x04
#define STREAM_TYPE_PRIVATE_SECTION 0x05
#define STREAM_TYPE_PRIVATE_DATA    0x06
#define STREAM_TYPE_AUDIO_AAC       0x0f
#define STREAM_TYPE_AUDIO_AAC_LATM  0x11
#define STREAM_TYPE_VIDEO_MPEG4     0x10
#define STREAM_TYPE_VIDEO_H264      0x1b
#define STREAM_TYPE_VIDEO_VC1       0xea
#define STREAM_TYPE_VIDEO_DIRAC     0xd1

#define STREAM_TYPE_AUDIO_AC3       0x81
#define STREAM_TYPE_AUDIO_DTS       0x8a

#define  Peek_Short(ptr) (*(ptr) << 8 | *((ptr) + 1))


#define MAX_PAT_BUF_SIZE 1024
#define MAX_PMT_BUF_SIZE 1024
#define MAX_VIDEO_DATA_BUF_SIZE (1024*1024)
#define MAX_AUDIO_DATA_BUF_SIZE (8*1024)

#ifdef WIN32
typedef char int8_t;
typedef unsigned short int16_t;
typedef int int32_t;
typedef __int64 int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;
#endif


typedef enum ePidType
{
	PID_INVALID,
	PID_PAT,
	PID_PMT,
	PID_VIDEO,
	PID_AUDIO,
	PID_SUBTITLE
}ePidType;

typedef enum eEsType
{
	ES_VIDEO,
	ES_AUDIO,
	ES_SUBTITLE
}eEsType;

typedef struct {
    uint32_t sample_rate;
    uint8_t  chan_config;

} AACADTSHeaderInfo;

typedef struct MpegTSContext 
{
	int32_t  bGetPAT;
	int32_t  bGetPMT; 
	int32_t  iPMTPid;
	int32_t  iVideoPid;
	int32_t  iAudioPid;
	int32_t  iVideoStreamType;
	int32_t  iAudioStreamType;
	uint64_t uiVideoPts;
	uint64_t uiVideoDts;
	uint64_t uiAudioPts;
	int8_t  iLastVideoCC;
	int8_t  iLastAudioCC;
	int32_t iPatBufSize;
	uint8_t auPatBuf[MAX_PAT_BUF_SIZE];
	int32_t iPmtBufSize;
	uint8_t auPmtBuf[MAX_PMT_BUF_SIZE];
	uint16_t uiVideoPesLen;
	uint8_t * pVideoDataBuf;
	int32_t iVideoDataBufSize;
	uint16_t uiAudioPesLen;
	uint8_t * pAudioDataBuf;
	int32_t iAudioDataBufSize;
//	VideoEsCallBack fGetVideoEs;
//	AudioEsCallBack fGetAudioEs;
//	CCEventCallBack fCCEvent;
}MpegTSContext;



#ifdef __cplusplus
extern "C" {
#endif

extern int32_t mpegts_init(MpegTSContext * pMpegTSCtx);
extern int32_t mpegts_destroy(MpegTSContext * pMpegTSCtx);
extern int32_t mpegts_register_videoes_cb(MpegTSContext * pMpegTSCtx,VideoEsCallBack fCallBack);
extern int32_t mpegts_register_audioes_cb(MpegTSContext * pMpegTSCtx,AudioEsCallBack fCallBack);
extern int32_t mpegts_register_ccevent_cb(MpegTSContext * pMpegTSCtx,CCEventCallBack fCallBack);

#ifdef __cplusplus
}
#endif

#endif
#endif

