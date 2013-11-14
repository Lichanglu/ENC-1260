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

#define MSG_QUALITYVALUE			46 //���ø������������ģʽ(0:�� 1:�� 2:��) 

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


#define MSG_PIC_DATA                0x94        //�Զ�ppt����
#define MSG_LOW_SCREENDATA          0x95        //����������
#define MSG_LOW_BITRATE             0x96        //���������
#define MSG_MUTE                    0x97        //������1��������0��������������������ʾ��ѯ����״̬��
#define MSG_PHOTO                   0x98        //���๦��
#define MSG_LOCK_SCREEN    0x99//������Ļ
#define MSG_GSCREEN_CHECK           0x9a   //����У�����޲���
#define MSG_SETVGAADJUST            43 //�߿����,��Ϣͷ+pic_revise
#define MSG_CAMERACTRL_PROTOCOL     0x9b   //����ͷ����Э���޸� ��Ϣͷ���һ���ֽڣ�

#define MSG_TRACKAUTO				0x9f
/*��Ϣͷ�ṹ*/
typedef struct __MSGHEAD__ {
	/*
	## nLen:
	## ͨ��htonsת����ֵ
	## �����ṹ�屾�������
	##
	*/
	WORD	nLen;
	WORD	nVer;							//�汾��(�ݲ���)
	BYTE	nMsg;							//��Ϣ����
	BYTE	szTemp[3];						//�����ֽ�
} MSGHEAD;

/*��Ƶ����Ƶ���緢������ͷ*/
typedef struct __HDB_FRAME_HEAD {
	DWORD ID;								//=mmioFOURCC('4','D','S','P');
	DWORD nTimeTick;    					//ʱ���
	DWORD nFrameLength; 					//֡����
	DWORD nDataCodec;   					//��������
	//���뷽ʽ
	//��Ƶ :mmioFOURCC('H','2','6','4');
	//��Ƶ :mmioFOURCC('A','D','T','S');
	DWORD nFrameRate;   					//��Ƶ  :֡��
	//��Ƶ :������ (default:44100)
	DWORD nWidth;       					//��Ƶ  :��
	//��Ƶ :ͨ���� (default:2)
	DWORD nHight;       					//��Ƶ  :��
	//��Ƶ :����λ (default:16)
	DWORD nColors;      					//��Ƶ  :��ɫ��  (default: 24)
	//��Ƶ :��Ƶ���� (default:64000)
	DWORD dwSegment;						//�ְ���־λ
	DWORD dwFlags;							//��Ƶ:  I ֡��־
	//��Ƶ:  ����
	DWORD dwPacketNumber; 					//�����
	DWORD nOthers;      					//����
} FRAMEHEAD;

/*����Ļ�ṹ����*/
typedef struct __RecAVTitle {
	int  x;                     //x pos
	int  y;            //y pos
	int len;   //Textʵ�ʳ���
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
/*����MAC��ַ*/
extern int SetMacAddr(int fd);
/*��Ϣͷ���*/
extern void PackHeaderMSG(BYTE *data, BYTE type, WORD len);
/*�������û��������ʼ��*/
extern int initSetParam(void);
/*����ICMP ����socket*/
extern void InitICMP(void);
/*�ر�ICMP ����socket*/
extern int cleanICMP(void);
/*����ICMP ������Ҫ�ǽ�����ڲ��֮�����粻ͨ��bug*/
extern void SendICMPmessage(void);
/*��������ip��ַ*/
extern void SetEthConfigIP(unsigned int ipaddr, unsigned netmask);
/*��������*/
extern void SetEthConfigGW(unsigned int gw);
/*TCP task mode*/
extern void TCPServerTask(void *pParam);
/*����OSD*/
extern int CreateTextOsdBuffer(void);
/*���ʱ����Ļ*/
extern void drawtimebufferThread(void *pParam);
/*�������ݼ���*/
extern int SendPthreadLock();
/*�������ݼ���*/
extern int SendPthreadunLock();
extern void ParsePackageLock();
extern void ParsePackageunLock();
/*broadcast message*/
extern void LowRateBroadCastMsg(int index, BYTE cmd, unsigned char *data, int len);
/*send encode data to every client*/
extern void SendLowRateToClient(int nLen, unsigned char *pData, int nFlag, unsigned char index);
/*send encode audio data to every client*/
extern void SendAudioToClient(int nLen, unsigned char *pData, int nFlag, unsigned char index, unsigned int samplerate);
/*�����Ļ��ʱ��*/
extern int AddOsdText(int num, unsigned char *data, int len);
extern void FarCtrlCamera(int dsp, unsigned char *data, int len);
/*���������*/
extern int RequestLowRate(int num, unsigned char *data, int len, int nPos);
/*����ͼ��ı߿�*/
extern int RevisePictureFrame(unsigned char *data, int len);
/*����У��*/
extern int GreenScreenAjust(void);
/*send encode data to every client*/
extern void SendDataToClient(int nLen, unsigned char *pData, int nFlag, unsigned char index, int width, int height);

#endif //__TCPCOM__H
