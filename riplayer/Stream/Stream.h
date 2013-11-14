
#ifndef STREAM_H
#define  STREAM_H

#include "stdafx.h"
#include <stdio.h>
//#include   <afx.h> 
//#include <iostream.h>
#include <process.h>
#include <PROCESS.h>
#include <stdlib.h>
#include <math.h>
#include <winsock2.h>
#include <winsock.h> 
#include <set>
#include <list>
#include <Winsock.h>
#include <Ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib")
#include "../mp4/mp4_common.h"
#include "middle_control.h"
#ifndef PRINTF
#define PRINTF debug_output_file
#endif

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

#define MAX_PAT_BUF_SIZE 1024
#define MAX_PMT_BUF_SIZE 1024
#define MAX_VIDEO_DATA_BUF_SIZE (1024*1024)
#define MAX_AUDIO_DATA_BUF_SIZE (8*1024)
//hsp add by 2012/05/30
static int AUDIO_SAMPLE[] =
{
	96000,  	//0X0
		88200,  	//0X1	
		64000,	//0X2
		48000,	//0X3
		44100,	//0X4
		32000,	//0X5
		24000,	//0X6
		22050,	//0X7
		16000,	//0X8
		2000,	//0X9
		11025,	//0XA
		8000,	//0XB
};

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

typedef enum _Stream_Protocol{
	RTP_STREAM=1,
	TS_STREAM,
	RTP_TS_STREAM,
	RTSP_STREAM,
}Stream_Protocol;

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


typedef struct 
{
    /**//* byte 0 */
    unsigned char csrc_len:4;        /**//* expect 0 */
    unsigned char extension:1;        /**//* expect 1, see RTP_OP below */
    unsigned char padding:1;        /**//* expect 0 */
    unsigned char version:2;        /**//* expect 2 */
    /**//* byte 1 */
    unsigned char payload:7;        /**//* RTP_PAYLOAD_RTSP */
    unsigned char marker:1;        /**//* expect 1 */
    /**//* bytes 2, 3 */
    unsigned short seq_no;            
    /**//* bytes 4-7 */
    unsigned  long timestamp;        
    /**//* bytes 8-11 */
    unsigned long ssrc;            /**//* stream number is used here. */
} RTP_FIXED_HEADER;

typedef struct _Frame_Info
{
	char type; 
	char I_frame;  //I帧标识位
	int loss; 
	int width;
	int high;
	int sample_rate; 
	long timestamp;
	int audio_config;  //audio sdp config
	int  end ; 
}Frame_Info;

typedef struct _ENCODER_INFO{
	bool thread_ext;
	int frame_rate;       //速率
	int bit_rate;            //位速率	
	int width;      //分辨率
	int high;
	unsigned int total_frame;//总帧
	int lost_frame;//丢帧
	char origin[128];//源住处
}Encoder_Info;


typedef void (*Get_Encoder_Info_func)(Encoder_Info*);
typedef void (*Msg_func)(Stream_Protocol,int);

typedef void (* VideoEsCallBack)(unsigned char * pVideoEsBuf,int iBufLen,Frame_Info*);
typedef void (* AudioEsCallBack)(unsigned char * pAudioEsBuf,int iBufLen,Frame_Info*);
//typedef void (* CCEventCallBack)(unsigned char uiLastCC,unsigned char uiCurrCC,eEsType mediatype);

typedef struct _SDP_INFO{
int type;
char multicast;
unsigned short audio_port;
unsigned short video_port;
char IP[16];
int config;
}SDP_Info;


typedef struct _TS_Handle{
	SOCKET sock;
	char thread_ext;
	unsigned short port;
	char serv_addr[16];
	int len;
	Frame_Info frame;
	VideoEsCallBack fGetVideoEs;
	AudioEsCallBack fGetAudioEs;
	Msg_func fCCEvent;
	}TS_Handle;


typedef struct _RTP_Handle{
	char flag;
	char thread_ext;
	SOCKET sock;
	char serv_addr[16];
	unsigned short port;	
	Frame_Info frame;
	VideoEsCallBack fGetVideoEs;
	Msg_func fCCEvent;
}RTP_Handle;


typedef struct _Encoder_Handle{
	bool thread_ext;
	Encoder_Info encoder_info;
	Get_Encoder_Info_func get_encoder_info;
	uint64_t total_frame_cnt;	
	uint64_t lost_frame_cnt;
	uint64_t total_byte_cnt;
}Encoder_Handle;

typedef enum{
	MSG_CREATE_FAIL = 0,
 	MSG_RECV_FAIL ,
	MSG_WRITE_FAIL,	
	MSG_LOST_FRAME,
	MSG_MEM_ERROR,
	MSG_THREAD_EXIT,
}MSG_Stream_Info;

extern int read_gaps_in_frame(unsigned char *p,  int *width,  int *hight, unsigned int len);
extern INT32 parse_url(char* url,char *addr,unsigned short *port,char*);
extern SDP_Info *parse_sdp(const char* buff,int buff_len);

extern int is_loss_frame(unsigned short prev_seq_no,unsigned short seq_no,uint16_t tim);
extern unsigned int rtsp_stream_audio_filter_sdp_info(unsigned char *pdata,int *samplerate); //hsp add

extern RTP_Handle* rtp_video_open_url(SDP_Info* sdp_info);
extern RTP_Handle* rtp_audio_open_url(SDP_Info* sdp_info);
extern int rtp_set_output_func(RTP_Handle *handle,void* func);
extern int rtp_set_msg_func(RTP_Handle *handle,void *func);
extern int rtp_close_stream(RTP_Handle  **handle);

extern TS_Handle* TS_open_url(char *addr,UINT16 port);
extern TS_Handle* TS_RTP_open_url(char *addr,UINT16 port,char multicast);

extern int ts_set_videoes_cb(TS_Handle * ,void*  );
extern int ts_set_audioes_cb(TS_Handle * ,void*  );
extern int ts_set_ccevent_cb(TS_Handle * ,void* );
extern int TS_close_stream(TS_Handle  **handle);

extern int sdkTcpConnect(char *ip);
extern int sdkCloseTCPConnet (SOCKET Sockfd);
/*音频参数*/
extern int sdkSetAudioParam(SOCKET fd,AudioParam *inaudio,AudioParam *outaudio);
extern int sdkGetAudioParam(SOCKET fd,AudioParam *audio);
//视频参数
extern int sdkSetVideoparam(SOCKET fd,OutputVideoInfo *inval,OutputVideoInfo *outval);
extern int sdkGetVideoparam(SOCKET fd,OutputVideoInfo *outval);

extern int sdkSetLogin(SOCKET fd,char *username, char *password);

extern bool open_debug_file();
extern int  debug_output_file(char* format,...);
extern void close_debug_file();

extern Encoder_Handle* open_Encoder(const char* src);
extern void encoder_info_set_cbfunc(Encoder_Handle* handle ,void* function);
extern void get_encoder_resolution(Encoder_Info* handle,int width,int high);
extern void close_Encoder_Info(Encoder_Handle* info);
 
/*************************** RTSP *******************************/

//#define BUF_SIZE 4096
#define HEADER_SIZE 1024
#define MAX_FIELDS 256	
	
#define RTSP_STATUS_SET_PARAMETER  10
#define RTSP_STATUS_OK            200

#define DEFAULT_HEARTBIT_TIME 120   //120s
	
typedef   	struct rtsp_s {
	
	  SOCKET		s;
	
	  char		   *host;
	  int			port;
	  char		   *path;
	  char		   *mrl;
	  char		   *user_agent;
	
	  char		   *server;
	  unsigned int	server_state;
	   unsigned int	server_caps;
	
	  unsigned int	cseq;
	  char		   *session;
	  
	  unsigned int		 sdp_len;
	
	  char		  *answers[MAX_FIELDS];   /* data of last message */
	  char		  *scheduled[MAX_FIELDS]; /* will be sent with next message */
}rtsp_t;

typedef int (*PF_connect)( void *p_userdata, char *p_server, int i_port );
typedef int (*PF_disconnect)( void *p_userdata );
typedef int (*PF_read)( void *p_userdata, uint8_t *p_buffer, int i_buffer );
typedef int (*PF_read_line)( void *p_userdata, uint8_t *p_buffer, int i_buffer );
typedef int (*PF_write)( void *p_userdata, uint8_t *p_buffer, int i_buffer );
	
typedef struct
{
		void *p_userdata;
		PF_connect pf_connect;
		PF_disconnect pf_disconnect;
		PF_read pf_read;
		PF_read_line pf_read_line;
		PF_write pf_write;
		rtsp_t *p_private;	
} rtsp_client_t;
	


typedef struct RTSP_HANDLE_T{
	rtsp_client_t *client;
	char rtsp_url[256];
	char sdp_info[2048];
	int status;  //1表示关闭 0表示开启
	RTP_Handle *video_handle;
	RTP_Handle *audio_handle;
	VideoEsCallBack fGetVideoEs;
	AudioEsCallBack fGetAudioEs;
	Msg_func fCCEvent;
}RTSP_HANDLE;


extern RTSP_HANDLE* rtsp_open_stream(char *url);
extern void rtsp_set_video_output_func(RTSP_HANDLE *handle,void *func);
extern void rtsp_set_audio_output_func(RTSP_HANDLE *handle,void *func);
extern void rtsp_set_msg_output_func(RTSP_HANDLE *handle,void *func);
extern void rtsp_close_stream(RTSP_HANDLE *handle);


#endif
