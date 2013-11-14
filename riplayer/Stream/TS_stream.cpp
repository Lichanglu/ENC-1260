#include "../StdAfx.h"
#include "stream.h"
#include "mpegts.h"
#include "../RMedia.h"
#include "../RiplayerDlg.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <PROCESS.h>
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
#include <process.h>
#pragma comment(lib,"ws2_32.lib") 

extern int32_t mpegts_parse(MpegTSContext * pMpegTSCtx,TS_Handle *ts_handle,uint8_t * pBuf);	
static void TS_stream_pthread(TS_Handle* ts_handle);
static  void  TS_RTP_thread(void* handle);
//创建TS接收线程


#define BUF_SIZE   1500

/*static int AUDIO_SAMPLE[] =
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
};*/


typedef struct audio_aac_adts_fixed_header_t {
	int syncword;
	unsigned char id;
	unsigned char layer;
	unsigned char profile;
	unsigned char sampling_frequency_index;
	unsigned char channel_configuration;
} audio_aac_adts_fixed_header_s;


extern int32_t mpegts_init(MpegTSContext * pMpegTSCtx);
extern int32_t mpegts_destroy(MpegTSContext * pMpegTSCtx);

/*parse audio aac adts header length*/
 unsigned int rtsp_stream_audio_filter_sdp_info(unsigned char *pdata,int *samplerate)
{
	if(pdata == NULL) {
		return -1;
	}

	int syncword = 0;
	unsigned char id;
	unsigned char layer;
	unsigned char profile;
	unsigned char sampling_frequency_index;
	unsigned char channel_configuration;
	audio_aac_adts_fixed_header_s audio_adts_header;
	unsigned int config = 0;

	syncword = (pdata[0] << 4) | ((pdata[1] & 0xf0) >> 4) ;

	if(syncword != 0x0fff) {
		PRINTF("rtsp_stream_audio_filter_sdp_info:the syncword =0x%x\n", syncword);
		return -1;
	}

	id = (pdata[1] & 0x08) >> 3;
	layer = (pdata[1] & 0x06) >> 1;
	profile = (pdata[2] & 0xc0) >> 6;
	sampling_frequency_index = (pdata[2] & 0x3c) >> 2;
	channel_configuration = ((pdata[2] & 0x01) >> 2) | ((pdata[3] & 0xc0) >> 6);
	
	memset(&audio_adts_header, 0, sizeof(audio_adts_header));
	audio_adts_header.id = id;
	audio_adts_header.layer = layer;
	audio_adts_header.profile = profile;
	audio_adts_header.sampling_frequency_index = sampling_frequency_index;
	audio_adts_header.channel_configuration = channel_configuration;

	config = 0x1000 | (((audio_adts_header.sampling_frequency_index & 0x0f) >> 1) << 8)
	         | ((audio_adts_header.sampling_frequency_index & 0x01) << 7)
	         | ((audio_adts_header.channel_configuration) << 3) ;
	
	*samplerate	=	audio_adts_header.sampling_frequency_index;
	return config;
}




int ts_set_videoes_cb(TS_Handle* ts_handle,void* fCallBack)
{
	if(fCallBack)
	{
		ts_handle->fGetVideoEs = (VideoEsCallBack)fCallBack;
	}
	return 0;
}
int ts_set_audioes_cb(TS_Handle * ts_handle,void* fCallBack)
{
	if( fCallBack )
	{
		ts_handle->fGetAudioEs = (AudioEsCallBack)fCallBack;
	}
	return 0;
}
int ts_set_ccevent_cb(TS_Handle * ts_handle,void* fCallBack)
{
	if(fCallBack)
	{
		ts_handle->fCCEvent = (Msg_func)fCallBack;
	}
	return 0;
}


int TS_close_stream(TS_Handle  **handle)
{
	if( *handle != NULL){
		(*handle)->thread_ext=1;
		(*handle)->fGetVideoEs = NULL;
		(*handle)->fGetAudioEs = NULL;
		(*handle)->fCCEvent = NULL;
	}
	return 0;
}



TS_Handle* TS_open_url(char *addr,unsigned short port)
{
	TS_Handle*   ts_handle = NULL;
	int addr_len;
	struct sockaddr_in srv;
	struct ip_mreq mreq;
	
	ts_handle =(TS_Handle*)malloc(sizeof(TS_Handle)) ;	
	if( NULL ==ts_handle )
		return NULL;

	ts_handle->fGetAudioEs = NULL;
	ts_handle->fGetVideoEs = NULL;
	ts_handle->fCCEvent = NULL;
	ts_handle->thread_ext = 0;

	strncpy(ts_handle->serv_addr,addr,16);
	ts_handle->port = port;
	
	WSADATA wsaData;
     if (WSAStartup(MAKEWORD(2,1),&wsaData)) //调用Windows Sockets DLL
     { 
          printf("Winsock无法初始化!\n");
          WSACleanup();
          return NULL ;
     }

	addr_len = sizeof(struct sockaddr_in);

	memset(&srv, 0,sizeof(srv));
	srv.sin_family = AF_INET;
	srv.sin_port = htons( ts_handle->port);
	srv.sin_addr.s_addr = INADDR_ANY; 

	if ((ts_handle->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return NULL;
	}
	
	
	char val = 1; 
	//SO_REUSEADDR选项就是可以实现端口重绑定的	
	if(setsockopt(ts_handle->sock,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val))<0) 
	{ 
		PRINTF("TS_open_url:error!setsockopt failed!/n"); 
		return NULL; 
	} 

	
	if (bind(ts_handle->sock, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
		PRINTF("TS_open_url:bind fail\n");
		return NULL;
	}

	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	mreq.imr_multiaddr.s_addr = inet_addr(ts_handle->serv_addr);

	if (setsockopt(ts_handle->sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const char *)&mreq,sizeof(mreq))< 0)
	{
		PRINTF("TS_open_url:setsockopt memership\n");
		return NULL;
	}
	
	int ret = 0 ;
	int buf_size=1024*1024*10;
	int optlen=sizeof(buf_size);
	ret = setsockopt(ts_handle->sock,SOL_SOCKET,SO_RCVBUF,(char *)&buf_size,sizeof(int));//SOL_S0CKET
	if( ret < 0 )
	{
		PRINTF("TS_open_url:setsockopt: ret = %d \n",ret );
		ret = getsockopt(ts_handle->sock,SOL_SOCKET,SO_RCVBUF,(char*)&buf_size,(int*)&optlen);
		if( ret < 0 )
			PRINTF("TS_open_url:getsockopt \n");
		return NULL;
	}

	struct	 timeval   timeout={10000,0};
	setsockopt( ts_handle->sock, SOL_SOCKET, SO_RCVTIMEO, ( char * )&timeout, sizeof( timeout ) ); 
	if( ret < 0 ){
		PRINTF("TS_open_url:set timeout fail!\n");
		return NULL;
	}
	
	_beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))TS_stream_pthread,ts_handle, 0, 0);

	return ts_handle;
}


int TS_RTP_close_stream(TS_Handle  **handle)
{
	if(*handle != NULL ){
		(*handle)->thread_ext=1;
		(*handle)->fGetVideoEs = NULL;
		(*handle)->fGetAudioEs = NULL;
		(*handle)->fCCEvent = NULL;
	}
	return 0;
}

TS_Handle* TS_RTP_open_url(char *addr,UINT16 port,char multicast)
{
	TS_Handle*   ts_handle =NULL;
	int addr_len;
	struct sockaddr_in srv;
	struct ip_mreq mreq;
	
	ts_handle =(TS_Handle*)malloc(sizeof(TS_Handle)) ;	
	if( NULL ==ts_handle )
		return NULL;

	ts_handle->fGetAudioEs = NULL;
	ts_handle->fGetVideoEs = NULL;
	ts_handle->fCCEvent = NULL;
	ts_handle->thread_ext = 0;

	strncpy(ts_handle->serv_addr,addr,16);
	ts_handle->port = port;
	
	WSADATA wsaData;
     if (WSAStartup(MAKEWORD(2,1),&wsaData)) //调用Windows Sockets DLL
     { 
          printf("Winsock无法初始化!\n");
          WSACleanup();
          return NULL ;
     }

	addr_len = sizeof(struct sockaddr_in);

	memset(&srv, 0,sizeof(srv));
	srv.sin_family = AF_INET;
	srv.sin_port = htons( ts_handle->port);
	srv.sin_addr.s_addr = INADDR_ANY; 

	if ((ts_handle->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return NULL;
	}	

	char val = 1; 
	//SO_REUSEADDR选项就是可以实现端口重绑定的	
	if(setsockopt(ts_handle->sock,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val))<0) 
	{ 
	 PRINTF("TS_RTP_open_url:error!setsockopt failed!/n"); 
	 return NULL; 
	} 

	if (bind(ts_handle->sock, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
		perror("bind main");
		return NULL;
	}
	
	if(1 == multicast){
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		mreq.imr_multiaddr.s_addr = inet_addr(ts_handle->serv_addr);

		if (setsockopt(ts_handle->sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const char *)&mreq,sizeof(mreq))< 0) {
			PRINTF("TS_RTP_open_url: setsockopt fail\n");
			return NULL;
		}
	}

	int ret = 0 ;
	int buf_size=1024*1024*10;
	int optlen=sizeof(buf_size);
	ret = setsockopt(ts_handle->sock,SOL_SOCKET,SO_RCVBUF,(char *)&buf_size,sizeof(int));//SOL_S0CKET
	if( ret < 0 )
	{
		PRINTF("setsockopt: ret = %d \n",ret );
		ret = getsockopt(ts_handle->sock,SOL_SOCKET,SO_RCVBUF,(char*)&buf_size,(int*)&optlen);
		if( ret < 0 )
			PRINTF("getsockopt \n");
	}
	
	struct	 timeval   timeout={10000,0};
	setsockopt( ts_handle->sock, SOL_SOCKET, SO_RCVTIMEO, ( char * )&timeout, sizeof( timeout ) ); 
	if( ret < 0 ){
		PRINTF("TS_RTP_open_url:set timeout fail!\n");
		return NULL;
	}
	
	_beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))TS_RTP_thread,ts_handle, 0, 0);

	return ts_handle;
}

static void TS_stream_pthread(TS_Handle* ts_handle)
{
	unsigned char buf[BUF_SIZE];
	int r;
	CTime time =CTime::GetCurrentTime();
	MpegTSContext tsctx;
	struct sockaddr_in cli;
	int addr_len = sizeof(	struct sockaddr_in);
	
	mpegts_init(&tsctx);

	while (1)
	{
		if(1 == ts_handle->thread_ext){
			if(ts_handle->fCCEvent)
				ts_handle->fCCEvent(TS_STREAM,MSG_THREAD_EXIT);
			break;
		}

		//memset(&tsctx,0,sizeof(tsctx));
		memset(buf,0,sizeof(buf));
		r = recvfrom(ts_handle->sock,(char*)buf, sizeof(buf), 0, (struct sockaddr *)&cli, &addr_len);
		if (r< 0) {			
			time =CTime::GetCurrentTime();
			PRINTF("TS_stream_pthread:recv timeout! \t h:m:s = %d:%d:%d\n",
				time.GetHour(),time.GetMinute(),time.GetSecond());
			if(ts_handle->fCCEvent)
				ts_handle->fCCEvent(TS_STREAM,MSG_RECV_FAIL);
			continue;
		} else {
			ts_handle->len = r;
			r = mpegts_parse(&tsctx,ts_handle,buf);
			if(r == -1 ){
				if(ts_handle->fCCEvent)
					ts_handle->fCCEvent(TS_STREAM,MSG_LOST_FRAME	);
			}else if( r == -2){// NO Media stream
				Sleep(3000);
				if( ts_handle->fCCEvent)
					;//	ts_handle->fCCEvent(TS_STREAM,MSG_RECV_FAIL);
			}
		}
 	}	
	mpegts_destroy(&tsctx);
	closesocket(ts_handle->sock);
	(ts_handle)->fGetVideoEs = NULL;
	(ts_handle)->fGetAudioEs = NULL;
	(ts_handle)->fCCEvent = NULL;
	free(ts_handle);
	ts_handle= NULL;	
	PRINTF("TS_Recv_thread:free");
	return ;
}


static  void  TS_RTP_thread(void* handle)
 {
	struct sockaddr_in addr;
	MpegTSContext tsctx;
	unsigned char buf[BUF_SIZE];
	TS_Handle* ts_handle = (TS_Handle*)handle;
	RTP_FIXED_HEADER* rtp_hdr=NULL;
	RTP_FIXED_HEADER rtp_prev_hdr;
	bool flag = true;
	CTime time =CTime::GetCurrentTime();
	
	int addrlen = sizeof(struct sockaddr_in);
	int ret = 0;

	mpegts_init(&tsctx);
	while(1){
		if(1 == ts_handle->thread_ext){
			if(ts_handle->fCCEvent)
				ts_handle->fCCEvent(TS_STREAM,MSG_THREAD_EXIT);
			break;
			}
		
		ret = recvfrom(ts_handle->sock,(char*)buf, BUF_SIZE, 0,(struct sockaddr *)&addr, &addrlen);
		if( ret < 0 )	
		{
			time =CTime::GetCurrentTime();
			PRINTF("TS_RTP_thread:recv timeout! \t h:m:s = %d:%d:%d\n",
				time.GetHour(),time.GetMinute(),time.GetSecond());
			if(ts_handle->fCCEvent)
				ts_handle->fCCEvent(RTP_TS_STREAM,MSG_RECV_FAIL);
			continue;
		}
		
		//检测seq_no的连续
		rtp_hdr=(RTP_FIXED_HEADER*)buf;
		if(flag){
			rtp_prev_hdr = *rtp_hdr;
			flag=false;
			}

		if(  -1 == is_loss_frame( ntohs(rtp_prev_hdr.seq_no),ntohs(rtp_hdr->seq_no),ntohs(rtp_hdr->timestamp)) )
		{
			PRINTF("TS_RTP_thread:lost frame\n");
			ts_handle->fCCEvent(RTP_TS_STREAM,MSG_LOST_FRAME);
		}
		
		rtp_prev_hdr = *rtp_hdr;
		rtp_hdr=NULL;
		
		ts_handle->len = ret-12;
		ret = mpegts_parse(&tsctx,ts_handle,(unsigned char *)(buf+12));
		
		if(ret == -1 ){
			if(ts_handle->fCCEvent)
				ts_handle->fCCEvent(RTP_TS_STREAM,MSG_LOST_FRAME	);
		}else if( ret == -2){// NO Media stream
			Sleep(3000);
			if(ts_handle->fCCEvent)
				;//ts_handle->fCCEvent(RTP_TS_STREAM,MSG_RECV_FAIL	);
		}
		
 	}	
	mpegts_destroy(&tsctx);
	closesocket(ts_handle->sock);
	(ts_handle)->fGetVideoEs = NULL;
	(ts_handle)->fGetAudioEs = NULL;
	(ts_handle)->fCCEvent = NULL;
	free(ts_handle);
	ts_handle= NULL;
	PRINTF("TS_RTP_thread:free");

	return ;
}


