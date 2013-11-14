#include "stdafx.h"
#include "stream.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <PROCESS.h>

#include <process.h>
#include "mpegts.h"
#pragma comment(lib,"ws2_32.lib") 
//#pragma comment(lib,"mp4_lib.lib") 


#ifndef		PRINT_DEBUG
#define     PRINT_DEBUG   0
#endif

typedef enum{
 IDR_FRAME= 2,
 SPS_FRAME   , 
 PPS_FRAME   , 
 P_FRAME     ,
}Frame_Type;


#define BUF_SIZE   1500	
#define FRAME_SIZE   1920*1080*3/2
#define AUDIO_FRAME_SIZE 2048*2

typedef struct 
{
    //byte 0
	unsigned char TYPE:5;
    unsigned char NRI:2;
	unsigned char F:1;             
} NALU_HEADER; /**//* 1 BYTES */

typedef struct 
{
    //byte 0
    unsigned char TYPE:5;
	unsigned char NRI:2; 
	unsigned char F:1;          
} FU_INDICATOR; /**//* 1 BYTES */

typedef struct 
{
    //byte 0	
	unsigned char TYPE:5;
	unsigned char R:1;
	unsigned char E:1;
	unsigned char S:1;
} FU_HEADER; /**//* 1 BYTES */

typedef struct 
{
    /**//* byte 0 */
    unsigned char byte1;
    /**//* byte 1 */
    unsigned char byte2;
    /**//* bytes 2, 3 */
    unsigned short seq_no;            
    /**//* bytes 4-7 */
    unsigned  long timestamp;        
    /**//* bytes 8-11 */
    unsigned long ssrc;            /**/
} RTP_FIXED_HEADER_AUDIO;

#define MEM_ERROR -11
typedef struct RTP_RECV_INFO_T{

	unsigned int 	p_type;        //payload type
	unsigned int    market;
	 int seq_no;     //last seq_no	
	unsigned long timestamp;
}RTP_RECV_INFO;

typedef enum MARKET_STATUS{
	MARKET_INVAILD = 3,
	MARKET_BEGIN  = 4,
	MARKET_MIDDLE  = 5,
	MARKET_END = 6,
	MARKET_MAX
};

static int RTPPacket_to_H264Data(		unsigned char* buf,int len,
		RTP_FIXED_HEADER *rtp_prev_hdr,	RTP_Handle *rtp_handle,unsigned char* frame_buf,unsigned int *frame_len);
static void* rtp_audio_thread_recv(RTP_Handle *rtp_handle);
extern int read_gaps_in_frame(unsigned char *p, int *width, int *hight, unsigned int len);

static void rtp_stream_thread(RTP_Handle *rtp_handle)
{
	struct sockaddr_in  addr;//serv_addr;
	int addrlen = sizeof(struct sockaddr_in);
	int ret = 0;
	RTP_FIXED_HEADER rtp_header;	
	unsigned char buf[BUF_SIZE];
	
	Frame_Info media_info = {0,0,0,0,0,0,0,0,0};
	struct timeval tm = {0,100};
	unsigned int frame_len=0 ; 
	unsigned char* frame_buf = (unsigned char*)malloc(FRAME_SIZE);
	CTime time =CTime::GetCurrentTime(); //时间
	int err_packet_cnt = 0;
	
	if( frame_buf == NULL){
		if(frame_buf != NULL){
			PRINTF("rtp_stream_thread:free");
			free(frame_buf);
			frame_buf=NULL;
		}
		if(rtp_handle->fCCEvent)
			rtp_handle->fCCEvent(RTP_STREAM,MSG_CREATE_FAIL);
		frame_len=0;
		rtp_handle->fGetVideoEs = NULL;	
		rtp_handle->fCCEvent = NULL;
		closesocket((rtp_handle)->sock);
		free(rtp_handle);
		(rtp_handle)= NULL;
		return;	
	}	
	memset(frame_buf,0,FRAME_SIZE);

	
	while(1)
	{
		if(1 ==  rtp_handle->thread_ext ){
			PRINTF("rtp_stream_thread:Thread Exit!\n");
			break;
		}
		memset(buf,0,BUF_SIZE);
		
		ret = recvfrom(rtp_handle->sock ,(char*)buf, BUF_SIZE, 0,(struct sockaddr *)&addr, &addrlen);

		if( ret < 0 )
		{
			time =CTime::GetCurrentTime();
			PRINTF("rtp_stream_thread:recv timeout! \t H:m:s = %d:%d:%d\n",
				time.GetHour(),time.GetMinute(),time.GetSecond());
			if(rtp_handle->fCCEvent )
				rtp_handle->fCCEvent( RTP_STREAM,MSG_RECV_FAIL);
			continue;
		}
		
		ret = RTPPacket_to_H264Data(buf,ret,&rtp_header,rtp_handle,frame_buf,&frame_len) ; 
		if ( ret == 0 ){
		//	err_packet_cnt = 0;
			if(rtp_handle->frame.I_frame > 0 ){
				if( rtp_handle->fGetVideoEs != NULL ){
					
					rtp_handle->frame.type=0;
					if(frame_buf[4] == 0x67){
						media_info.I_frame = 1;
						read_gaps_in_frame( (frame_buf)+4,&(media_info.width),
							 &(media_info.high),(frame_len)-4);
						}else
							media_info.I_frame = 0;							
										
					rtp_handle->fGetVideoEs(frame_buf,frame_len,&media_info);
					frame_len = 0 ;
					memset(frame_buf,0,FRAME_SIZE);
					continue;
				}
				rtp_handle->frame.I_frame=1;
			}
			else if(rtp_handle->frame.I_frame == 0 ){
				if(frame_buf[4] == 0x67){
						read_gaps_in_frame( (frame_buf)+4,&(media_info.width),
							 &(media_info.high),(frame_len)-4);
						rtp_handle->frame.I_frame=1;
						continue;	
				}
				frame_len = 0 ;
				memset(frame_buf,0,FRAME_SIZE);
			}
		}
		else if( -1 == ret ){	
			(rtp_handle->frame.loss)++ ;
			
			if(rtp_handle->fCCEvent != NULL){
				rtp_handle->fCCEvent( RTP_STREAM,MSG_LOST_FRAME);
				char strTepm[100]={0};
				sprintf(strTepm, "**** loss = %d ****\n",rtp_handle->frame.loss);
				OutputDebugString(strTepm);
			}
			PRINTF("rtp_stream_thread:**** loss = %d ****\n",rtp_handle->frame.loss);

			frame_len = 0 ;
			memset(frame_buf,0,FRAME_SIZE);
			continue;			
		} else if( -2 == ret )
		{
			err_packet_cnt++;
			if( err_packet_cnt > 200){
				if(rtp_handle->fCCEvent != NULL)
				rtp_handle->fCCEvent( RTP_STREAM,MSG_RECV_FAIL);
			}
		}
	}

	
	if(frame_buf != NULL){
		PRINTF("rtp_stream_thread:free!\n");
		free(frame_buf);
		frame_buf=NULL;
	}
	frame_len=0;
	rtp_handle->fGetVideoEs = NULL;	
	rtp_handle->fCCEvent = NULL;
	closesocket((rtp_handle)->sock);
	free(rtp_handle);
	(rtp_handle)= NULL;
	return;
		
}



RTP_Handle* rtp_audio_open_url(SDP_Info* sdp_info)
{	

	int addrlen = sizeof(struct sockaddr_in);
	RTP_Handle* rtp_audio_handle =(RTP_Handle*)malloc(sizeof(RTP_Handle)) ;	
	struct ip_mreq mreq;
	int ret = 0 ;
	int buf_size=1024*1024*10;
	int optlen=sizeof(buf_size);
	struct sockaddr_in peeraddr;
	unsigned int threadHandle;

	rtp_audio_handle->port = sdp_info->audio_port;
	memcpy(rtp_audio_handle->serv_addr,sdp_info->IP,16);	
	
	rtp_audio_handle->frame.type = 0;
	rtp_audio_handle->frame.width = 0;
	rtp_audio_handle->frame.high = 0;
	rtp_audio_handle->thread_ext = 0;
	rtp_audio_handle->frame.I_frame = 0;
	rtp_audio_handle->flag = 0;
	rtp_audio_handle->fGetVideoEs = NULL;
	rtp_audio_handle->fCCEvent = NULL;

	rtp_audio_handle->frame.audio_config = sdp_info->config;	
	
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,1),&wsaData)) //调用Windows Sockets DLL
	 { 
		PRINTF("rtp_audio_open_url:WSAStartup ERROR\n");
	        WSACleanup();
	       return  NULL;
	 }

	rtp_audio_handle->sock = socket(AF_INET, SOCK_DGRAM, 0);  
	if(rtp_audio_handle->sock <0)
	{
		PRINTF("rtp_audio_open_url:socket ERROR\n");
		return NULL;
	}	

	
	char val = 1; 
	//SO_REUSEADDR选项就是可以实现端口重绑定的	
	if(setsockopt(rtp_audio_handle->sock,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val))<0) 
	{ 
	 PRINTF("TS_RTP_open_url:error!setsockopt failed!/n"); 
	 return NULL; 
	} 

		
		
	memset(&peeraddr, 0, addrlen);	
	peeraddr.sin_family = AF_INET;	
	peeraddr.sin_port = htons(rtp_audio_handle->port);
	peeraddr.sin_addr.s_addr = INADDR_ANY;    //inet_addr(url);
		
	ret = bind(rtp_audio_handle->sock,(struct sockaddr*) &peeraddr,addrlen);
	if(ret <0)
	{
		PRINTF("rtp_audio_open_url:bind ERROR\n");
		return NULL;
	}

	
	if(1 == sdp_info->multicast){
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		mreq.imr_multiaddr.s_addr = inet_addr(rtp_audio_handle->serv_addr);

		if (setsockopt(rtp_audio_handle->sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const char *)&mreq,sizeof(mreq))< 0) {
		
			PRINTF("rtp_audio_open_url:setsockopt ERROR\n");
			return NULL;
		}
	}

	ret = setsockopt(rtp_audio_handle->sock,SOL_SOCKET,SO_RCVBUF,(char *)&buf_size,sizeof(int));//SOL_S0CKET
	if( ret < 0 )
	{
		ret = getsockopt(rtp_audio_handle->sock,SOL_SOCKET,SO_RCVBUF,(char*)&buf_size,(int*)&optlen);
		if( ret < 0 )
			PRINTF("rtp_audio_open_url:getsockopt ERROR\n");
		return NULL;
	}

	struct	 timeval   timeout={10000,0};
	setsockopt( rtp_audio_handle->sock, SOL_SOCKET, SO_RCVTIMEO, ( char * )&timeout, sizeof( timeout ) ); 
	if( ret < 0 ){
		PRINTF("rtp_audio_open_url:set timeout fail!\n");
		return NULL;
	}

	threadHandle =  _beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))rtp_audio_thread_recv,rtp_audio_handle, 0, 0);
	
	return rtp_audio_handle ;
}

RTP_Handle* rtp_video_open_url(SDP_Info *sdp_info)
{	
	RTP_Handle* rtp_handle = (	RTP_Handle*)malloc(sizeof(	RTP_Handle));
	struct ip_mreq mreq;
//	RTP_FIXED_HEADER* rtp_header=NULL;
	struct sockaddr_in peeraddr;
	int addrlen = sizeof(struct sockaddr_in);
	int ret = 0 ;
	int buf_size=1024*1024*10;
	int optlen=sizeof(buf_size);
//	unsigned char buf[BUF_SIZE];
	unsigned int threadHandle;

	rtp_handle->frame.type = 0;
	rtp_handle->frame.loss=0;
	rtp_handle->frame.width = 0;
	rtp_handle->thread_ext = 0;
	rtp_handle->frame.high = 0;
	rtp_handle->frame.I_frame = 0;
	rtp_handle->fGetVideoEs = NULL;
	rtp_handle->fCCEvent = NULL;

	strncpy(rtp_handle->serv_addr,sdp_info->IP,16);
	rtp_handle->port = sdp_info->video_port;	
	
	WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,1),&wsaData)) //调用Windows Sockets DLL
	{ 
			PRINTF("rtp_video_open_url:WSAStartup ERROR\n");
	         WSACleanup();
	         return NULL;
	 }

	rtp_handle->sock = socket(AF_INET, SOCK_DGRAM, 0);  
	if(rtp_handle->sock <0)
	{
		PRINTF("rtp_video_open_url:socket ERROR");
		return NULL ;
	}	

	char val = 1; 
	//SO_REUSEADDR选项就是可以实现端口重绑定的	
	if(setsockopt(rtp_handle->sock,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val))<0) 
	{ 
		PRINTF("rtp_video_open_url:error!setsockopt failed!/n"); 
		return NULL; 
	} 
	
	memset(&peeraddr, 0, addrlen);	
	peeraddr.sin_family = AF_INET;	
	peeraddr.sin_port = htons(rtp_handle->port);
	peeraddr.sin_addr.s_addr = INADDR_ANY;    //inet_addr(url);
	
	ret = bind(rtp_handle->sock,(struct sockaddr*) &peeraddr,addrlen);
	if(ret <0)
	{
		PRINTF("rtp_video_open_url:bind ERROR");
		return NULL ;
	}

	ret = setsockopt(rtp_handle->sock,SOL_SOCKET,SO_RCVBUF,(char *)&buf_size,sizeof(int));//SOL_S0CKET
	if( ret < 0 )
	{
		ret = getsockopt(rtp_handle->sock,SOL_SOCKET,SO_RCVBUF,(char*)&buf_size,(int*)&optlen);
		if( ret < 0 )
			PRINTF("rtp_video_open_url:getsockopt ERROR");
		return NULL;
	}

	
	if(1 == sdp_info->multicast){
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		mreq.imr_multiaddr.s_addr = inet_addr(rtp_handle->serv_addr);

		if (setsockopt(rtp_handle->sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const char *)&mreq,sizeof(mreq))< 0) {
			
			PRINTF("rtp_video_open_url:setsockopt ERROR");
			return NULL;
		}
	}
	
	struct	 timeval   timeout={10000,0};
	ret = setsockopt( rtp_handle->sock, SOL_SOCKET, SO_RCVTIMEO, ( char * )&timeout, sizeof( timeout ) ); 
	if( ret < 0 ){
		PRINTF("rtp_video_open_url:set timeout fail!\n");
		return NULL;
	}
	
	//rtp+264
	threadHandle =  _beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))rtp_stream_thread,rtp_handle, 0, 0);
	return rtp_handle ;	
}


int rtp_set_output_func(RTP_Handle *handle,void* func)
{
	if( handle->fGetVideoEs  == NULL)
		handle->fGetVideoEs =(VideoEsCallBack)func;
	return 0; 
}

int rtp_set_msg_func(RTP_Handle *handle,void *func)
{
	if(	handle->fCCEvent == NULL)
		handle->fCCEvent = (Msg_func)func;
	return 0;
}

int rtp_close_stream(RTP_Handle  **handle)
{
	if( *handle != NULL){
		(*handle)->thread_ext = 1;
		(*handle)->fGetVideoEs = NULL;
		(*handle)->fCCEvent = NULL;
	}
	return 0;
}

static uint16_t tim_seq = 0;
int is_loss_frame(unsigned short prev_seq_no,unsigned short seq_no,uint16_t tim)
{
	int loss;
	int ret =0;
	if( prev_seq_no == 0xffff )
		return 0;
	
	loss =  seq_no - prev_seq_no  ;
	if( loss > 1 ){
		if(tim_seq != tim){
			ret =  -1;		
			PRINTF("tim_seq=%x,tim=%x\n",tim_seq,tim);
		}else 
			ret = 1;// 1 忽略
		tim_seq = tim;
		
	}else {
		ret = 0;
	}
	return ret;
}

//接收H264 数据
static int RTPPacket_to_H264Data(		unsigned char* buf,int len,
		RTP_FIXED_HEADER *rtp_prev_hdr,	RTP_Handle *rtp_handle,unsigned char* frame_buf,unsigned int *frame_len)
{
	FU_INDICATOR	             *fu_ind;
	FU_HEADER		             *fu_hdr;
	RTP_FIXED_HEADER       	 	 *rtp_hdr=(RTP_FIXED_HEADER*)buf; 
	int ret =0;

	fu_ind = (FU_INDICATOR*)&(buf[12]);
	fu_hdr = (FU_HEADER*)&(buf[13]);
	
	//分片
	if( fu_ind->TYPE ==  28 ) 
	{
		
		if( rtp_handle->flag == 1 ){
			 if(fu_hdr->S != 0x1)
				return 1;
			else if(  fu_hdr->S == 0x1 ){
				memcpy(rtp_prev_hdr ,buf,sizeof(RTP_FIXED_HEADER));
				rtp_handle->flag = 0;
				}
		}

		ret = is_loss_frame( ntohs(rtp_prev_hdr->seq_no),ntohs(rtp_hdr->seq_no),ntohs(rtp_hdr->timestamp));
		if(  (-1 == ret) && (rtp_handle->frame.I_frame != 0)){
			memcpy(rtp_prev_hdr ,buf,sizeof(RTP_FIXED_HEADER));
			rtp_handle->flag = 1;			
			return -1;
		}else if(ret == 1){
			memcpy(rtp_prev_hdr ,buf,sizeof(RTP_FIXED_HEADER));
			return ret ;
		}
		//起始  新帧
		if( fu_hdr->S == 0x1)
				/* && (rtp_hdr->timestamp  !=  rtp_prev_hdr->timestamp) */
		{
			//表示是新帧
		frame_buf[(*frame_len)++]=0;
		frame_buf[(*frame_len)++]=0;
		frame_buf[(*frame_len)++]=0;
		frame_buf[(*frame_len)++]=1;
		frame_buf[(*frame_len)++]=((fu_ind->F)<<7)|((fu_ind->NRI)<<5)|fu_hdr->TYPE;
		memcpy((frame_buf+(*frame_len)),(buf+14),len-14);
		((*frame_len)) += len - 14;
			
		memcpy(rtp_prev_hdr ,buf,sizeof(RTP_FIXED_HEADER));
		rtp_handle->flag = 0;

		}else{ //  if( rtp_hdr->timestamp  ==  rtp_prev_hdr->timestamp ){
			memcpy(frame_buf+((*frame_len)),(buf+14),len-14);

			(*frame_len) += len - 14;
			memcpy(rtp_prev_hdr , buf,sizeof(RTP_FIXED_HEADER));
		//	rtp_prev_hdr = rtp_hdr;
			if(fu_hdr->E && 0x1){
					return 0;	
				}
		}
	}
	else{
//		if( frame_buf[4] != 0x67 || frame_buf[4] != 0x68 || frame_buf[4]!= 65)
//			return -2;//非RTP包
		
		frame_buf[(*frame_len)++]=0;
		frame_buf[(*frame_len)++]=0;
		frame_buf[(*frame_len)++]=0;
		frame_buf[(*frame_len)++]=1;
		memcpy((frame_buf+(*frame_len)),(buf+12),len-12);
		(*frame_len) += len-12;
		//rtp_prev_hdr = rtp_hdr;
		memcpy(rtp_prev_hdr, buf,sizeof(RTP_FIXED_HEADER));
		
		if(frame_buf[4] != 0x67 ||frame_buf[4] != 0x68)
			return 0;
	}
	return 1;
}

// aac

static void rtp_push_msg(RTP_Handle *rtp_handle,int errorcode)
{
	if(rtp_handle != NULL && rtp_handle->fCCEvent != NULL)
	{
		rtp_handle->fCCEvent(RTP_STREAM,errorcode);
	}
	return ;
}

static int parse_aac_config(int config,int *samplerate,int *channel)
{
	*samplerate = (config>>7) & 0x0f;
	*channel =   (config>>3) & 0x07;
	return 0;
}

static int OnAudioAacFrame(int samplerate,int frame_length,unsigned char *adts_header)
{
	int channels = 2;
 	unsigned int obj_type = 1;
    	unsigned int num_data_block = frame_length / 1024;
   	 // include the header length also
    	frame_length += 7;
   	 /* We want the same metadata */
   	 /* Generate ADTS header */
    	if(adts_header == NULL)
			return 0;
    	/* Sync point over a full byte */
    	adts_header[0] = 0xFF;
    	/* Sync point continued over first 4 bits + static 4 bits
    	* (ID, layer, protection)*/
    	adts_header[1] = 0xF1;
   	 /* Object type over first 2 bits */
   	 adts_header[2] = obj_type << 6;//
   	 /* rate index over next 4 bits */
   	 adts_header[2] |= (samplerate << 2);
   	 /* channels over last 2 bits */
    	adts_header[2] |= (channels & 0x4) >> 2;
    	/* channels continued over next 2 bits + 4 bits at zero */
    	adts_header[3] = (channels & 0x3) << 6;
   	 /* frame size over last 2 bits */
   	 adts_header[3] |= (frame_length & 0x1800) >> 11;
    	/* frame size continued over full byte */
   	 adts_header[4] = (frame_length & 0x1FF8) >> 3;
    	/* frame size continued first 3 bits */
    	adts_header[5] = (frame_length & 0x7) << 5;
   	 /* buffer fullness (0x7FF for VBR) over 5 last bits*/
   	 adts_header[5] |= 0x01;
   	 /* buffer fullness (0x7FF for VBR) continued over 6 first bits + 2 zeros
   	  * number of raw data blocks */
	adts_header[6] = 0x8c;// one raw data blocks .
	adts_header[6] |= num_data_block & 0x03; //Set raw Data blocks.
	return 0;
}

static int aac_payload_demux(unsigned char *buf,int len,int status,Frame_Info *info,unsigned char* frame_buf,unsigned int *frame_len)
{
	int i = 0;
	char header[32] = {0};

	int samplerate = 0x07;
	int channel = 2;
	if(info->end == 1)
	{
		(*frame_len) = 0;
	}
	int a_len = 0;

	a_len = (buf[3]>>3 | ((buf[2]&0x07) <<5)) | ((buf[2]>>3)<<8);

	if((*frame_len) == 0)
		(*frame_len) +=7;
	memcpy(frame_buf + (*frame_len) ,buf + 4 ,len -4);
	(*frame_len) += (len-4);
	
	if(status == 1 && (*frame_len) != a_len+7)
	{

		(*frame_len) = 0;
		return -1;
	}
//	if(status == 1)
		info->end = status;
	if(status == 1)
	{
		parse_aac_config(info->audio_config,&(info->sample_rate),&channel);			
		OnAudioAacFrame((info->sample_rate),(*frame_len),(unsigned char*)header);
		memcpy(frame_buf,header,7);
		return 2;
	}

	return -1;

}

static int RTPPacket_to_AACData(unsigned char* buf,int len,RTP_RECV_INFO *info,RTP_Handle *rtp_handle,unsigned char* frame_buf,unsigned int *frame_len)
{
	if(buf == NULL || len  < 14)
	{
		//push msg
		rtp_push_msg(rtp_handle,MSG_MEM_ERROR);
		return -1;
	}
	int ret = 0;
	RTP_FIXED_HEADER *rtp_head = (RTP_FIXED_HEADER *)buf;
	unsigned short seq_no = ntohs(rtp_head->seq_no);
	int market = rtp_head->marker;
	int ptype = rtp_head->payload;
	unsigned  long timestamp = rtp_head->timestamp;

	Frame_Info *frame = &(rtp_handle->frame);

	
	if(info->p_type== 0)
	{
		info->p_type = ptype;
		info->seq_no = seq_no;	
		info->market = MARKET_INVAILD;
	}

	if(( seq_no -info->seq_no ) != 1 && (info->seq_no - seq_no) != 0xffff)
	{
//	PRINTF("RTPPacket_to_AACData:Error,seq_no is failled. last =%d,new=%d\n",info->seq_no,seq_no);

		info->seq_no = seq_no;
		
		(*frame_len) = 0;
		
		//info->market = MARKET_INVAILD;
		//加入丢包统计
		return -1;
	}
	info->seq_no = seq_no;
	
	if( info->market == MARKET_INVAILD)
	{
		if(market == 1 )
		{
			info->market = MARKET_BEGIN;
			
			(*frame_len) = 0;
			
//			PRINTF("Error,the market have failed ,last market=%d,new=%d\n",info->market,market);
			
		}
		return -1;
	}
	if(market == 0)
		info->market = MARKET_MIDDLE;
	else
		info->market = MARKET_END;

	if(info->market ==  MARKET_END)
		ret = aac_payload_demux(buf+12,len-12,1, &(rtp_handle->frame),frame_buf,frame_len);
	else
		ret = aac_payload_demux(buf+12,len-12,0, &(rtp_handle->frame),frame_buf,frame_len);
	
	return ret;
		

}


static void* rtp_audio_thread_recv(RTP_Handle *rtp_audio_handle)
{
	struct sockaddr_in addr;
	int addrlen = sizeof(struct sockaddr_in);
	int ret = 0;
	unsigned int frame_len =0 ;
	
	Frame_Info media_info = {0,0,0,0,0,0,0,0,0};
	char buf[BUF_SIZE];
	unsigned char* IDR_flag= 0;
	unsigned char* frame_buf = (unsigned char*)malloc(AUDIO_FRAME_SIZE);
	if(frame_buf == NULL){
		if(frame_buf != NULL){
			PRINTF("rtp_audio_thread_recv:free!\n");
			free(frame_buf);
			frame_buf=NULL;
		}
		if(rtp_audio_handle->fCCEvent)
			rtp_audio_handle->fCCEvent(RTP_STREAM,MSG_CREATE_FAIL);
		frame_len=0;
		if(rtp_audio_handle->fGetVideoEs != NULL)
			rtp_audio_handle->fGetVideoEs = NULL;	
		if(	rtp_audio_handle->fCCEvent != NULL)
			rtp_audio_handle->fCCEvent = NULL;
		closesocket(rtp_audio_handle->sock);

		free(rtp_audio_handle);
		rtp_audio_handle= NULL;
		return NULL;
	}
	memset(frame_buf,0,AUDIO_FRAME_SIZE);
	
	RTP_RECV_INFO r_info;
	memset(&r_info,0,sizeof(RTP_RECV_INFO));
	r_info.market = MARKET_INVAILD;

	while(1){	
		if(1 == rtp_audio_handle->thread_ext ){
			PRINTF("rtp_audio_thread_recv: thread Exit!\n");
		//	if(rtp_audio_handle->fCCEvent)
			//	rtp_audio_handle->fCCEvent(RTP_STREAM,MSG_THREAD_EXIT);
		//	Sleep(1000);
			break;
		}
		
		memset(buf,0,BUF_SIZE);
		ret = recvfrom(rtp_audio_handle->sock ,buf, BUF_SIZE, 0,(struct sockaddr *)&addr, &addrlen);
		if( ret < 0 )
		{		
			continue;
		}
		ret =  RTPPacket_to_AACData((unsigned char*)buf, ret,&r_info,rtp_audio_handle,frame_buf,(unsigned int*)&frame_len);
		if(ret == 2)
		{
			if(rtp_audio_handle->fGetVideoEs != NULL){
				rtp_audio_handle->fGetVideoEs(frame_buf,frame_len,&(rtp_audio_handle->frame));
				frame_len=0;
				memset(frame_buf,0,AUDIO_FRAME_SIZE);
				
				}
		}else if( ret == -1){
			;//if(rtp_audio_handle->fCCEvent!= NULL)
			//	rtp_audio_handle->fCCEvent(RTP_STREAM, MSG_LOST_FRAME);				
			}
	}

	if(frame_buf != NULL){
		PRINTF("rtp_audio_thread_recv:free!\n");
		free(frame_buf);
		frame_buf=NULL;
	}
	frame_len=0;
	if(rtp_audio_handle->fGetVideoEs != NULL)
		rtp_audio_handle->fGetVideoEs = NULL;	
	if(	rtp_audio_handle->fCCEvent != NULL)
		rtp_audio_handle->fCCEvent = NULL;
	closesocket(rtp_audio_handle->sock);

	free(rtp_audio_handle);
	rtp_audio_handle= NULL;
	return NULL;
}
