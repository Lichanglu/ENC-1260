#ifndef _MULTICAST_OUTPUT_STRUCT_H__
#define _MULTICAST_OUTPUT_STRUCT_H__


#define INVID_SERVER_NUM 0xFF
#define RTSP_MAX_CLIENT_NUM 8
#define OUTPUT_SERVER_MAX_NUM   8

typedef enum {
    NOTUSED = 0,
    ISUSED = 1
} U_STATUS;

/*active  正常状态 ，pause 不发流状态，stop  关闭socket状态*/
typedef enum {
    S_BEGIN = 0,
    S_ACTIVE = 1,
    S_PAUSE = 2,
    S_STOP = 3,
    S_DEL = 4
} S_STATUS;

typedef enum {
    TYPE_INVAID = 0,
    TYPE_TS = 1,
    TYPE_RTP = 2,
    TYPE_RTSP = 3,
    TYPE_TS_OVER_RTP = 4,
    TYPE_MAX_NUM,
} SERVER_TYPE;

typedef struct STREAM_OUTPUT_SERVER_CONFIG_T {
	int  num ;   // 0-MAX
	int 	type ;   // 1 TS  2 DIRECT RTP  3 RTSP multicast 4 rtsp unicast
	int 	status; // active,pause,stop
	char  main_ip[16]; //send ip or rtsp is localip
	int video_port; //rtsp server tcp listen port or rtp client video port
	// int video_port;
	int audio_port;
	int mtu;
	int ttl;
	int tos;
	//int active;
	int tc_flag;
	int tc_rate;  // == tc ceil
} stream_output_server_config;


typedef struct STREAM_CLIENT_SEND_T {
	int num;
	int type; 			 //TYPE TS OR TYPE RTP
	int c_used;
	int usenum ;//just for rtsp multicast
	int send_status;//just for pause
	struct sockaddr_in video_addr;
	struct sockaddr_in audio_addr;
	int video_socket;
	int audio_socket;

	int width;
	int height ;
	int samplerate;
	int media_status;  //0  invaild  1  vaild

	int video_send; //vlc必须先发送视频	
} Stream_Output_Client;


typedef struct RTSP_SERVER_CONFIG_T {
	int s_port;      //rtsp server port
	int active;      //rtsp tcp active
	int s_mtu;      //rtsp tcp mtu
	int mult_flag;  // //1 multicast 0 unicast
	char mult_ip[16];   //组播ip
	int  mult_port;     //video port ,audio_port =video_port+2

} rtsp_server_config;

#if 1
typedef struct APP_MEDIA_INFO_T {
	int width;
	int height;
	int sample;
} APP_MEDIA_INFO;
#endif

#endif

