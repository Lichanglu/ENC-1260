#ifndef __TCPCOM__H
#define __TCPCOM__H
#include "sysparam.h"

#define HEAD_LEN			sizeof(MSGHEAD)
#define  MSG_ADDCLIENT      	1
#define  MSG_DELETECLIENT   	2
#define  MSG_CONNECTSUCC    	3
#define  MSG_PASSWORD_ERR   	4
#define  MSG_MAXCLIENT_ERR  	5
#define  MSG_AUDIODATA			6
#define  MSG_SCREENDATA     	7
#define  MSG_HEARTBEAT      	8
#define  MSG_PASSWORD       	9
#define  MSG_DATAINFO       	10
#define  MSG_REQ_I          	11
#define  MSG_SET_FRAMERATE  	12
#define  MSG_PPT_INDEX  		15

#define MSG_SYSPARAMS			16
#define MSG_SETPARAMS			17
#define MSG_RESTARTSYS			18

#define MSG_UpdataFile			19
#define MSG_SAVEPARAMS			20
#define MSG_UpdataFile_FAILS		21
#define MSG_UpdataFile_SUCC			22

#define MSG_DECODE_STATUS			23
#define MSG_DECODE_DISCONNECT		24
#define MSG_DECODE_CONNECT			25

#define MSG_UPDATEFILE_DECODE 		26
#define MSG_UPDATEFILE_ROOT 		27
#define MSG_UPDATEFILE_WEB 			28

#define MSG_MODE_CODE				29
#define MSG_MODE_DECODE				30

#define MSG_ADD_TEXT				33

#define MSG_MOUT          			40
#define MSG_SENDFLAG    			41
#define MSG_FARCTRL      			42
#define MSG_VGA_ADJUST				43

#define MSG_QUALITYVALUE			46 //设置高清编码器码流模式(0:低 1:中 2:高) 

#define MSG_GET_VIDEOPARAM			0x70
#define MSG_SET_VIDEOPARAM			0x71
#define MSG_GET_AUDIOPARAM			0x72
#define MSG_SET_AUDIOPARAM			0x73
#define MSG_REQ_AUDIO				0x74
#define MSG_CHG_PRODUCT				0x75

#define MSG_SET_SYSTIME				0x77
#define MSG_SET_DEFPARAM			0x79

#define MSG_SET_PICPARAM			0x90
#define MSG_GET_PICPARAM			0x91


#define MSG_CHANGE_INPUT			0x92


#define MSG_SEND_INPUT				0x93


#define MSG_PIC_DATA                0x94        //自动ppt索引
#define MSG_LOW_SCREENDATA          0x95        //低码率数据
#define MSG_LOW_BITRATE             0x96        //请求低码流
#define MSG_MUTE                    0x97        //静音（1：静音，0：不静音，不带参数表示查询静音状态）
#define MSG_PHOTO                   0x98        //照相功能
#define MSG_LOCK_SCREEN    0x99//锁定屏幕
#define MSG_GSCREEN_CHECK           0x9a   //绿屏校正，无参数
#define MSG_SETVGAADJUST            43 //边框调整,消息头+pic_revise
#define MSG_CAMERACTRL_PROTOCOL     0x9b   //摄像头控制协议修改 消息头后跟一个字节，

#define MSG_TRACKAUTO				0x9f
/*消息头结构*/
typedef struct __MSGHEAD__ {
	/*
	## nLen:
	## 通过htons转换的值
	## 包括结构体本身和数据
	##
	*/
	WORD	nLen;
	WORD	nVer;							//版本号(暂不用)
	BYTE	nMsg;							//消息类型
	BYTE	szTemp[3];						//保留字节
} MSGHEAD;

/*音频和视频网络发送数据头*/
typedef struct __HDB_FRAME_HEAD {
	DWORD ID;								//=mmioFOURCC('4','D','S','P');
	DWORD nTimeTick;    					//时间戳
	DWORD nFrameLength; 					//帧长度
	DWORD nDataCodec;   					//编码类型
	//编码方式
	//视频 :mmioFOURCC('H','2','6','4');
	//音频 :mmioFOURCC('A','D','T','S');
	DWORD nFrameRate;   					//视频  :帧率
	//音频 :采样率 (default:44100)
	DWORD nWidth;       					//视频  :宽
	//音频 :通道数 (default:2)
	DWORD nHight;       					//视频  :高
	//音频 :采样位 (default:16)
	DWORD nColors;      					//视频  :颜色数  (default: 24)
	//音频 :音频码率 (default:64000)
	DWORD dwSegment;						//分包标志位
	DWORD dwFlags;							//视频:  I 帧标志
	//音频:  保留
	DWORD dwPacketNumber; 					//包序号
	DWORD nOthers;      					//保留
} FRAMEHEAD;

/*加字幕结构标题*/
typedef struct __RecAVTitle {
	int  x;                     //x pos
	int  y;            //y pos
	int len;   //Text实际长度
	char Text[128];//text
} RecAVTitle;


#define LOGIN_USER				0
#define LOGIN_ADMIN				1
#define INVALID_SOCKET 			-1
#define INVALID_FD					-1
#define	A(bit)		rcvd_tbl[(bit)>>3]	/* identify byte in array */
#define	B(bit)		(1 << ((bit) & 0x07))	/* identify bit in byte */
#define	SET(bit)	(A(bit) |= B(bit))
#define	CLR(bit)	(A(bit) &= (~(B(bit))))
#define	TST(bit)	(A(bit) & B(bit))

/*if use client*/
#define ISUSED(dsp,cli)				(gDSPCliPara[dsp].cliDATA[cli].bUsed == TRUE)
/*set client used*/
#define SETCLIUSED(dsp,cli,val)		(gDSPCliPara[dsp].cliDATA[cli].bUsed=val)
/*if login client*/
#define ISLOGIN(dsp,cli)			(gDSPCliPara[dsp].cliDATA[cli].bLogIn == TRUE)
/*set client login succeed or failed*/
#define SETCLILOGIN(dsp,cli,val)	(gDSPCliPara[dsp].cliDATA[cli].bLogIn=val)
/*get socket fd*/
#define GETSOCK(dsp,cli)			(gDSPCliPara[dsp].cliDATA[cli].sSocket)
/*set socket fd*/
#define SETSOCK(dsp,cli,val)		(gDSPCliPara[dsp].cliDATA[cli].sSocket=val)
/*current socket if valid*/
#define ISSOCK(dsp,cli)				(gDSPCliPara[dsp].cliDATA[cli].sSocket != INVALID_SOCKET)
/*get dsp handle*/
#define GETDSPFD(dsp)				(gDSPCliPara[dsp].dspFD)
/*set dsp handle*/
#define SETDSPFD(dsp,val)			(gDSPCliPara[dsp].dspFD=val)
/*if handle valid*/
#define ISDSPFD(dsp)				(gDSPCliPara[dsp].dspFD != INVALID_FD)
/*client login type*/
#define SETLOGINTYPE(dsp,cli,val)  	(gDSPCliPara[dsp].cliDATA[cli].nLogInType=val)
/*get login type*/
#define GETLOGINTYPE(dsp,cli)  		(gDSPCliPara[dsp].cliDATA[cli].nLogInType)
/*set resize flag*/
#define SETLOWRATEFLAG(dsp,cli,val)  	(gDSPCliPara[dsp].cliDATA[cli].LowRateflag=val)
/*get resize flag*/
#define GETLOWRATEFLAG(dsp,cli)  		(gDSPCliPara[dsp].cliDATA[cli].LowRateflag)

/*get connect time*/
#define GETCONNECTTIME(dsp,cli)  		(gDSPCliPara[dsp].cliDATA[cli].connect_time)


#define SETLOWRATESTART(val)            (gLowRate.flag = val)
#define IISLOWRATESTART()               (gLowRate.flag)

/*client infomation*/
typedef struct _ClientData {
	int bUsed;
	SOCKET sSocket;
	int bLogIn;
	int nLogInType;
	int LowRateflag;
	unsigned long connect_time;
} ClientData;

/*DSP client param*/
typedef struct __DSPCliParam {
	ClientData cliDATA[MAX_CLIENT]; //client param infomation
} DSPCliParam;


//extern int WriteData(int s, void *pBuf, int nSize);
/*broadcast message*/
extern void SendBroadCastMsg(int index, BYTE cmd, unsigned char *data, int len);
/*设置MAC地址*/
extern int SetMacAddr(int fd);
/*消息头打包*/
extern void PackHeaderMSG(BYTE *data, BYTE type, WORD len);
/*参数设置互斥变量初始化*/
extern int initSetParam(void);
/*建立ICMP 报文socket*/
extern void InitICMP(void);
/*关闭ICMP 报文socket*/
extern int cleanICMP(void);
/*发送ICMP 报文主要是解决网口插拔之后网络不通的bug*/
extern void SendICMPmessage(void);
/*设置网络ip地址*/
extern void SetEthConfigIP(unsigned int ipaddr, unsigned netmask);
/*设置网关*/
extern void SetEthConfigGW(unsigned int gw);
/*TCP task mode*/
extern void TCPServerTask(void *pParam);
/*创建OSD*/
extern int CreateTextOsdBuffer(void);
/*添加时间字幕*/
extern void drawtimebufferThread(void *pParam);
/*发送数据加锁*/
extern int SendPthreadLock();
/*发送数据加锁*/
extern int SendPthreadunLock();
extern void ParsePackageLock();
extern void ParsePackageunLock();
/*broadcast message*/
extern void LowRateBroadCastMsg(int index, BYTE cmd, unsigned char *data, int len);
/*send encode data to every client*/
extern void SendLowRateToClient(int nLen, unsigned char *pData, int nFlag, unsigned char index);
/*send encode audio data to every client*/
extern void SendAudioToClient(int nLen, unsigned char *pData, int nFlag, unsigned char index, unsigned int samplerate);
/*添加字幕和时间*/
extern int AddOsdText(int num, unsigned char *data, int len);
extern void FarCtrlCamera(int dsp, unsigned char *data, int len);
/*请求低码流*/
extern int RequestLowRate(int num, unsigned char *data, int len, int nPos);
/*调节图像的边框*/
extern int RevisePictureFrame(unsigned char *data, int len);
/*绿屏校正*/
extern int GreenScreenAjust(void);
/*send encode data to every client*/
extern void SendDataToClient(int nLen, unsigned char *pData, int nFlag, unsigned char index, int width, int height);

#endif //__TCPCOM__H
