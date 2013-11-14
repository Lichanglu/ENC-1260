/*****************************************************************************************
*  用来放置通信的接口，后续WEB,TCP_SDK，或者预计的串口中控，都是通过改接口返回。
*
*
*
*
*******************************************************************************************/
#ifndef _MID_CONTROL_H__
#define _MID_CONTROL_H__


//判断命令字段
#define 	WEB_IDENTIFIER 			0xaaaaaaaa
#define 	TCP_SDK_IDENTIFIER  0xaaaaaaab


#define ENCODESERVER_PORT   20001
#define SDK_LISTEN_PORT		4000

typedef enum __webparsetype
{
	INT_TYPE = 0,
	STRING_TYPE,
	STRUCT_TYPE,
}webparsetype;

typedef struct __webMsgInfo_1260
{
	int 			identifier;  //0xaaaaaaaa 标识一个有效数据
	int				len;      //总长
	webparsetype	type;     
}webMsgInfo_1260; 


//##############################################结构体##end####################################################
typedef struct _DATE_TIME_INFO {
	int year;
	int month;
	int mday;
	int hours;
	int min;
	int sec;
} DATE_TIME_INFO;

typedef enum __inputtype {
    VGA = 0,
    YCBCR,
    DVI,
    SDI,
    AUTO
} inputtype;

typedef enum __ENUM_PROTOCOL__ {
    OUTPUT_TS_STREAM = 0,
    OUTPUT_RTP_STREAM,
    OUTPUT_RTSP_STREAM,
    OUTPUT_RTMP_STREAM,
    OUTPUT_MAX_PROTOCOL
} Enum_Protocol;

/*system param table*/
typedef struct __SYSPARAMS {
	unsigned char szMacAddr[8];				//MAC address
	unsigned int dwAddr;					//IP address
	unsigned int dwGateWay;					//getway
	unsigned int dwNetMark;					//sub network mask
	char strName[16];						//encode box name
	char strVer[10];						//Encode box version
	unsigned short unChannel;							//channel numbers
	char bType; 							//flag  0 -------VGABOX     3-------200 4-------110 5-------120 6--------1200
	char bTemp[3]; 							//bTemp[0] ------1260
	int nTemp[7];							//reserve  nTemp[0] 表示DHCP FLAG
} SYSPARAMS;

/*video param*/
typedef struct __VIDEOPARAM {
	unsigned int nDataCodec;   					//Encode Mode
	//video: mmioFOURCC('H','2','6','4');
	unsigned int nFrameRate;   					//vdieo: FrameRate
	unsigned int nWidth;       					//video: width
	unsigned int nHight;       					//video: height
	unsigned int nColors;      					//video: colors
	unsigned int nQuality;							//video: qaulity
	unsigned short sCbr;								//video: Qaulity/BitRate
	unsigned short sBitrate;							//video: bitrate
} VideoParam;

/*audio param*/
typedef struct __AUDIOPARAM {
	unsigned int Codec;  							//Encode Type
	unsigned int SampleRate; 						//SampleRate
	unsigned int BitRate;  						//BitRate
	unsigned int Channel;  						//channel numbers
	unsigned int SampleBit;  						//SampleBit
	unsigned char  LVolume;					//left volume       0 --------31
	unsigned char  RVolume;					//right volume      0---------31
	unsigned short  InputMode;               //1------ MIC input 0------Line Input
} AudioParam;

//色彩空间转换
typedef struct pic_revise {
	short hporch;    //col
	short vporch;    //row
	int temp[4];
} pic_revise ;

typedef enum _POS_TYPE_T{
    ABSOLUTE2= 0,
    TOP_LEFT ,
    TOP_RIGHT ,
    BOTTOM_LEFT ,
    BOTTOM_RIGHT ,
    CENTERED 
} POS_TYPE;

typedef struct __textinfo {
	int xpos;
	int ypos;
	int showtime;
	int enable;
	int alpha;
	char msgtext[128];
	int postype;  //postype 
} textinfo;

typedef enum __colorspace {
    RGB = 0,
    YUV,
} ColorSpace;
//	输入信号源的信息。
typedef struct InputSignalInfo {
	int  mode;			//信号源分辨率信息。
	ColorSpace  spaceFlag;			//彩色空间标志。
	unsigned int 	hdcpFlag;			//开启HDCP版权保护标志。
	unsigned int		reservation;		//保留。
} InputSignalInfo;

typedef enum __SceneConfiguration {
    movies = 0,
    motionless,
    motion,
    custom,
} SceneConfiguration;

typedef struct LogoInfo {
	int x;	//显示位置坐标,左上角为0,0
	int y;
	int width;	//logo图片宽高
	int height;
	int enable;	//是否显示logo	,显示:1,不显示:0
	char filename[32];//图片名称15个字符,默认为logo.png
	int alpha;//透明度[0,100]
	int isThrough;//是否透过某一像素值,实现镂空的效果,透过1,不透2
	int filter;//如果isThrough为1，可以设置一个要透的值，一般为0x00或0xFF
	int errcode;//错误码为0表示没有错误，1表示无效的logo文件
	int nTemp;	//保留
	int postype; // pos type
} LogoInfo;

/*protocol struct*/
typedef struct __PROTOCOL__ {
	/*protocol count*/
	unsigned char count ;
	/*protocol open status  1 -- open */
	/*0--- close all */
	unsigned int status: 1 ;
	/*TS stream*/
	unsigned int TS: 1;
	/*RTP*/
	unsigned int RTP: 1;
	/*RTSP*/
	unsigned int RTSP: 1;
	/*RTMP*/
	unsigned int RTMP: 1;
	/*reserve*/
	unsigned char temp[7];
} Protocol;

/*mult addr*/
typedef struct __MULT_ADDR_PORT {
	char chIP[16];
	unsigned short nPort;
	short nStatus;
} MultAddr;

typedef enum __OutputResolution {
    LOCK_BEGIN = 0,
    LOCK_1920x1080P,
    LOCK_1280x720P,
    LOCK_1400x1050_VGA,
    LOCK_1366x768_VGA,
    LOCK_1280x1024_VGA,
    LOCK_1280x768_VGA,
    LOCK_1024x768_VGA,
    LOCK_720x480_D1,
    LOCK_352x288_D1,
    LOCK_MAX
} OutputResolution;

//输出视频信号的参数设置
typedef struct OutputVideoInfo {
	int resolution;		//设置输出分辨率。
	unsigned int 		 resizeMode;		//缩放模式选择。0：比例拉伸 1：全屏拉伸
	unsigned int			 encodelevel;		//编码级别。0：high profile   1：baseline profile
	SceneConfiguration   preset ;			//场景配置选项。
	unsigned int 		 nFrameRate;   	//视频帧率
	unsigned int			 IFrameinterval; 	//I 帧间隔
	unsigned int 			 sBitrate;			//视频码率。
	int					logo_show;
	int 					text_show;
	//unsigned int			logotext;		//是否显示logo/text。
} OutputVideoInfo;

typedef struct InputDetailInfo {
	char inputSignal[128];
	unsigned int 	hpv;
	unsigned int  	tmds;
	unsigned int 	vsyncF;
	unsigned int 	hdcp;
} InputDetailInfo;


//H264编码参数设置
typedef struct H264EncodeParam {
	unsigned int		param1;
	unsigned int		param2;
	unsigned int		param3;
	unsigned int		param4;
	unsigned int		param5;
	unsigned int		reservation;		//保留。

} H264EncodeParam;
//媒体流协议参数设置
typedef struct StreamParam {
	Enum_Protocol   	protocol;			//协议选项。
	char				ip[16];			//ip
	unsigned int			port;				//端口号。
	unsigned int			MTUParam;		//MTU值。
	unsigned int			TTLParam;		//TTL值。
	unsigned int			TOSParam;		//TOS值。
	unsigned int			shapFlag;			//流量整型标志。
	unsigned int			actualBandwidth;	//实际带宽。
	unsigned int			reservation;		//保留。
} StreamParam;

typedef enum __StreamProtocol {
    TSoverUDP,
    TSoverRTP,
    DirectRTP,
    QuickTime,
} StreamProtocol;
//rtsp参数设置
typedef struct RtspParam {
	unsigned int 		keepAtctive;
	unsigned int			MTUParam;				//MTU值。
	StreamProtocol	protocol;					//rtsp流传输协议。
	char				boardCastip[16];			//组播ip;
	unsigned int			boardCastPort;				//组播端口号;
	unsigned int			reservation;				//保留。
} RtspParam;



//##############################################结构体##end####################################################

//##############################################命令字段##begin####################################################


#define MSG_GETSOFTCONFIGTIME 			0x1110
#define	MSG_GETCPULOAD  				0x1111
#define	MSG_GETINPUTSIGNALINFO 		0x1112
#define	MSG_SETINPUTSIGNALINFO			0x1113
#define	MSG_SIGNALDETAILINFO			0x1114	
#define	MSG_REVISE_PICTURE				0x1115
#define	MSG_GETOUTPUTVIDEOINFO			0x1117
#define	MSG_SETOUTPUTVIDEOINFO			0x1118
#define	MSG_SETH264ENCODEPARAM		0x1119
#define	MSG_GETAUDIOPARAM 				0x111A
#define	MSG_GETVIDEOPARAM				0x111B
#define	MSG_GETSYSPARAM					0x111C
#define	MSG_GETINPUTTYPE				0x111D

#define	MSG_SETAUDIOPARAM				0x111E
#define	MSG_SETVIDEOPARAM				0x111F
#define	MSG_SETSYSPARAM					0x1120
#define	MSG_SETINPUTTYPE					0x1121
#define	MSG_GETSTREAMPARAM			0x1122
#define	MSG_SETSTREAMPARAM				0x1123
#define	MSG_GETRTSPPARAM				0x1124
#define	MSG_SETRTSPPARAM				0x1125
#define	MSG_FAR_CTRL						0x1126
#define MSG_GET_CTRL_PROTO				0x1127
#define	MSG_SET_CTRL_PROTO				0x1128
//#define	MSG_GETDHCPFLAG					0x1129
//#define	MSG_SETDHCPFLAG					0x112A
#define	MSG_UPDATESYS					0x112B
#define	MSG_REBOOTSYS					0x112C
#define	MSG_SYNCTIME					0x112D
#define	MSG_SAVEPARAM					0x112E
#define	MSG_START_MULT					0x112F
#define	MSG_STOP_MULT					0x1130
#define	MSG_GET_PROTOCOL				0x1131
#define	MSG_SETDEVICETYPE				0x1132
#define	MSG_GETDEVICETYPE				0x1133
#define MSG_BLUE_REVISE					0x1134
#define MSG_RESTORESET					0x1135
#define MSG_GETCOLORSPACE				0x1136
#define MSG_SETCOLORSPACE				0x1137
#define MSG_UPLOADIMG      				0x1139	//上传logo图片
#define MSG_GET_LOGOINFO  				0x113A	//获取logo图片信息
#define MSG_SET_LOGOINFO  				0x113B	//设置logo图片信息
#define MSG_GETH264ENCODEPARAM 			0x113D
#define MSG_GETHDCPVALUE				0x113F
#define MSG_LOCK_RESOLUTION				0x1140
#define MSG_GETOUTPUTRESOLUTION			0x1141
#define MSG_RESIZE_MODE					0x1142
#define MSG_GETRESIZEMODE				0x1143
#define MSG_GETENCODELV					0x1144
#define MSG_SETENCODELV					0x1145
#define MSG_GETSCENE					0x1146
#define MSG_SETSCENE					0x1147
#define MSG_GETFRAMERATE				0x1148
#define MSG_SETFRAMERATE				0x1149
#define MSG_GETIFRAMEINTERVAL			0x114A
//#define MSG_SETIFRAMEINTERVAL			0x114B
#define MSG_GETBITRATE					0x114C
#define MSG_SETBITRATE					0x114D
#define MSG_SETMUTE						0x1150
//#define MSG_GETSHOWTEXTLOGO				0x1151
//#define MSG_SETSHOWTEXTLOGO				0x1152

#define MSG_GETMUTE						0x1153
#define MSG_GET_TEXTINFO  				0x1154	
#define MSG_SET_TEXTINFO  				0x1155
#define MSG_GETENCODETIME				0x1156
#define MSG_SDK_LOGIN 						0x1157
#define MSG_GETSOFTVERSION                     0x1158
//#define MSG_GET_SHOWTEXT_FLAG 		0x1159
//#define MSG_SET_SHOWTEXT_FLAG 		0x115A
//#define MSG_GET_SHOWLOGO_FLAG 		0x115B
//#define MSG_SET_SHOWLOGO_FLAG 		0x115C

//#define MSG_GET_TEXTPOS_TYPE         0x115A
//#define MSG_GET_LOGOPOS_TYPE		0x115B
//#define MSG_SET_TEXTPOS_TYPE		0x115C
//#define MSG_SET_LOGOPOS_TYPE 		0x115D


/*stream output */
/*rtsp*/
#define MSG_RTSP_GET_USED      0X1201  		/*rtsp is used*/
#define MSG_RTSP_GET_GINFO    0x1202  		/*rtsp get server global info*/
#define MSG_RTSP_SET_GINFO     0X1203 		/*rtsp set server global info*/ 
#define MSG_RTSP_GET_CINFO    0x1204 		 /*rtsp get common info*/
#define MSG_RTSP_SET_CINFO    0x1205  		/*rtsp set common info*/
#define MSG_RTSP_SET_STATUS   0x1206           /*rtsp set start,stop,pause status*/
#define MSG_RTSP_ADD_SERVER   0x1208          /*add rtsp server*/
#define MSG_RTSP_DEL_SERVER    0x1209        /*del rtsp server*/
#define MSG_GET_SDP_INFO   0x120a
//#define MSG_RTSP_SET_STATUS  0x1310

/*multcast */
#define MSG_MULT_GET_NUM      0X1301  		/*get mult server total num*/
#define MSG_MULT_GET_CINFO    0x1304 		 /*mult get common info*/
#define MSG_MULT_SET_CINFO    0x1305  		/*mult set common info*/
#define MSG_MULT_SET_STATUS   0x1306           /*mult set start,stop,pause status*/
#define MSG_MULT_ADD_SERVER   0x1308          /*add mult server*/
#define MSG_MULT_DEL_SERVER    0x1309        /*del mult server*/
#define MSG_MULT_GET_TS_RATE   0x1310           /*get mult  ts rate*/
#define MSG_MULT_GET_RTP_RATE 0x1311 		/*get mult rtp rate*/
#define MSG_MULT_GET_RTPTS_RATE 0x1312 		/*get mult rtpts rate*/
//#define MSG_MULT_SET_STATUS  0x1310


/*******先临时添加****************/
//#define MSG_SETSHOWTEXTLOGO   0x9901
#define MSG_GET_BUILD_TIME 0x9901

//##############################################命令字段##end####################################################

//##############################################错误码##begin####################################################

#define 				CLIENT_RETSUCCESSVALUE 				0
#define 				CLIENT_ERR_UNKNOWCMD					0xf1
#define					CLIENT_ERR_PARAMLEN						0xf2
#define 				CLIENT_ERR_TCPCONNECT					0xf3
#define   			CLIENT_ERR_TCPSEND						0xf4
#define 				CLIENT_ERR_TCPRECV						0xf5
#define 				CLIENT_ERR_PARAMSVALUE				0xf6
#define  				CLIENT_ERR_USERNAME						0xf7
#define 				CLIENT_ERR_PASSWORD						0xf8


#define    			SERVER_RET_OK 								0X00
#define        	SERVER_RET_UNKOWN_CMD 				0xf01
#define        	SERVER_RET_INVAID_PARM_LEN  	0xf02               //编码器通信结构体长度不一致，需要查对应通信的头文件
#define        	SERVER_RET_INVAID_PARM_VALUE  0xf03 
#define        	SERVER_RET_SOCK_MAX_NUM  			0xf04
#define        	SERVER_RET_RECV_FAILED  			0xf05
#define        	SERVER_RET_SEND_FAILED  			0xf06
#define        	SERVER_RET_ID_ERROR  					0xf07
#define       	SERVER_RET_USER_INVALIED  		0xf08
#define 				SERVER_INTERNAL_ERROR					0xf09                //编码器内部错误，出现该错误，必须马上查原因


//##############################################错误码##end####################################################

#define MSGINFOHEAD			sizeof(webMsgInfo_1260)




#endif

