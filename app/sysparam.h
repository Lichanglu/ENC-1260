#ifndef __SYSPARAM__H
#define __SYSPARAM__H

#include "common.h"

#include "middle_control.h"

#define 	H264_ENC_PARAM_FLAG			0x55AA55AA
#define     MAX_FAR_CTRL_NUM		    6
#define MODE_MAX                33 //video max mode 
#define     MAX_FAR_CTRL_NUM		    6

#define 	LOW_VIDEO							0
#define 	HIGH_VIDEO						1
#define 	MAX_VAL						2

#define     MAX_MODE_VALUE			    50
#define     RUN_LED_GPIO				1
typedef int 			SOCKET;
typedef int 			DSPFD;
typedef unsigned short 	WORD ;
typedef unsigned int   	DWORD ;
typedef unsigned char  	BYTE;


/*��Ƶ/��Ƶ/ϵͳ������*/
typedef struct __SAVEPARAM {
	SYSPARAMS sysPara;                 				//standard param table
	VideoParam videoPara[PORT_NUM];				 	//video param table
	AudioParam audioPara[PORT_NUM];					//audio param table
} SavePTable;

typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

/*������Ƶ������*/
typedef struct __VENC_PARAMS_ {
	UINT32 uflag;  		/*video encode params available flag
						   flag:0x55AA55AA for param availability
						   flag:0x0  for param not availability
						   ������ʱ��flag: 0x55555555 ��߷����仯
						             flag: 0xAAAAAAAA ���ʷ����仯
						 */
	UINT32 req_I;       /*ǿ��I frames*/
	UINT32 framerate;	/*control frame rate*/
	UINT32 quality;		/*quality*/
	UINT32 width;		/*encoder lib width*/
	UINT32 height;		/*encoder lib height*/
	UINT16 bitrate;		/*bit rate*/
	UINT16 cbr;	  		/*1:bitrate / 0:quality*/
} VENC_params;

/*������Ƶ����������*/
typedef struct __AENC_PARAMS__ {
	UINT32 uflag;  		/* audio encode params available flag
						   flag:0x55AA55AA for param availability
						   flag:0x0  for param not availability
						 */
	UINT32 samplerate;		/*audio encode sample rate*/
	UINT32 bitrate;			/*audio encode bit rate*/
	UINT32 channel;			/*audio channels*/
	UINT32 samplebit;		/*sample bit*/
	UINT8  lvolume;   		/*left volume*/
	UINT8  rvolume;			/*right volume*/
	UINT16 inputmode;		/*0: mic input 1:line input*/
} AENC_params;

/*���ñ���������*/
typedef struct __VAENC_PARAM__ {
	VENC_params venc;
	VENC_params lowvenc;
	AENC_params aenc;
} VAENC_params, *pVAENC_Params;

/*Resize Param table*/
typedef struct __RESIZE_PARAM__ {
	unsigned int dbstatus;
	unsigned int flag ;
	unsigned int re_enc ; // receate encode lbi
	int black;
	int enc_w;			//source width
	int enc_h;			//source height
	int dst_w;			//dest  width
	int dst_h;			//dest height
	int x;
	int y;

	int nFrame;		//֡��
	int nBitrate;   //����
	int nTemp;		//����
} ResizeParam;

/*�����ʲ���*/
typedef struct strLOW_BITRATE {
	int nWidth;		//��
	int nHeight;		//��
	int nFrame;		//֡��
	int nBitrate;   //����
	int nTemp;		//����
} LOW_BITRATE;

/*ƫ����*/
typedef struct __HV__ {
	short	hporch;
	short	vporch;
} HV;
typedef struct __HV_Table__ {
	HV	analog[MAX_MODE_VALUE];
	HV	digital[MAX_MODE_VALUE];
} HVTable;

#include <pthread.h>

typedef pthread_mutex_t Pthread_t;
#define Lock(m_mutex) pthread_mutex_lock(&m_mutex);
#define unLock(m_mutex) pthread_mutex_unlock(&m_mutex);
#define Pthread_init(m_mutex)	pthread_mutex_init(&m_mutex,NULL);


/*����У������*/
typedef struct __GREENSCREEN__ {
	Pthread_t m_protect;
	short green[MAX_MODE_VALUE];		//����
} GreenScreen;

/*����У����*/
typedef struct __GREENTABLE__ {
	GreenScreen tbl;
	int (*CopyTable)(GreenScreen *Old, GreenScreen *New);
	int (*initTable)(GreenScreen *gs);
	int (*readTable)(char *filename, GreenScreen *gs);
	int (*saveTable)(char *filename, GreenScreen *Old, GreenScreen *New);
} GreenTable;
/*�̻߳��ⶨ��*/
typedef struct __PthreadLock__ {
	pthread_mutex_t net_send_m;
	pthread_mutex_t sys_m;
	pthread_mutex_t save_flush_m;
	pthread_mutex_t send_m;
	pthread_mutex_t send_audio_m;
	pthread_mutex_t audio_video_m;
	pthread_mutex_t save_sys_m;
#ifdef CL4000_DVI_SDI
	pthread_mutex_t rtp_send_m;
#else
	pthread_mutex_t save_dtype_m;
#endif

} PLock;

/*�̻߳��������ʼ��*/
extern int initMutexPthread(void);
/*�̻߳��������ʼ��*/
extern int DestroyMutexPthread(void);
/*��ʼ��������ṹ��*/
extern void InitSysParams();
/*��ʼ����Ƶ�������*/
extern int InitVideoEncParams(VideoParam *vparam);
/*��ʼ����������Ƶ�������*/
extern int InitLowRateParams(VideoParam *vparam);
/*��ʼ����Ƶ�������*/
extern int InitAudioEncParams(AudioParam *aparam);
/*���ñ���ĸ�*/
extern int SetH264Width(int width);
/*���ñ���ĸ�*/
extern int SetH264Height(int height);
/*��ȡ�����*/
extern unsigned int H264Width(void);
/*��ȡ�����*/
extern unsigned int H264Height(void);
/*�������ļ��ж�ȡ����*/
extern int ReadEncodeParamTable(char *config_file, SavePTable *tblParam);
/*�������ļ��ж�ȡ�����ʲ���*/
extern int ReadLowbitParamTable(char *config_file, SavePTable *tblParam);
/*�������ļ��ж�ȡIP��Ϣ*/
extern int ReadIPParamTable(char *config_file, SavePTable *tblParam);
/*����������ļ���*/
extern int SaveParamsToFlash();
/*��������ʲ������ļ���*/
extern int SaveLowRateParamsToFlash();
/**********************************************************************
* Function Name:  ConfigGetKeys
* Function scriptor:  Get All Key
* Access table:   NULL
* Modify Table:	  NULL
* Input param��   void *CFG_file File    void *section
* Output Param:    char *keys[]   : save key name
* Return:  		  key count   error: < 0
***********************************************************************/
extern int  ConfigGetKeys(void *CFG_file, void *section, char *keys[]);
/**********************************************************************
* Function Name:  ConfigSetKey
* Function scriptor:  Set Key Value
* Access table:   NULL
* Modify Table:	  NULL
* Input param��   void *CFG_file File   void *section   void *key
					void *buf
* Output Param:   void *buf  key Value
* Return:  		  0 -- OK  no 0-- FAIL
***********************************************************************/
extern int  ConfigSetKey(void *CFG_file, void *section, void *key, void *buf);
/*
## ��MAC��ַ���ַ���ת�����ַ�������
##
## ��:  src = "00:11:22:33:44:55" --->
##		dst[0] = 0x00 dst[1] = 0x11 dst[2] = 0x22;
##
*/
extern int SplitMacAddr(char *src, unsigned char *dst, int num);

/*�� GPIO �˿�*/
extern int open_gpio_port();

/*get audio param*/
extern void GetAudioParam(int socket, int dsp, unsigned char *data, int len);
/*get video param*/
extern void GetVideoParam(int socket, int dsp, unsigned char *data, int len);
/*set video param table*/
extern int SetVideoParam(int dsp, unsigned char *data, int len);
/*set audio param table*/
extern int SetAudioParam(int dsp, unsigned char *data, int len);
/*SetSysParams*/
extern int SetSysParams(SYSPARAMS *pold, SYSPARAMS *pnew);
/*����ı�����*/
extern int GetTextLen(char *Data);
extern int GetSizeLen(char *Data);
/*Force I frame */
extern int ForceIframe(int start);
/*update program*/
extern int DoUpdateProgram(int sSocket, unsigned char *data, int len);
//*********************************************************************
//���úͻ�ȡ��������
//*********************************************************************
extern int SetMuteParams(unsigned char *data, int len);
/*Print current avaliable users*/
extern void PrintClientNum();
//*********************************************************************
//��ȡ�߿�ֵ����ʼ���Լ����ڱ߿�
//*********************************************************************
/*read Hportch Vportch Table*/
extern int ReadHVTable(const HVTable *pp, int digital);
extern int SaveHVTable(HVTable *pOld, HVTable *pNew, int digital);
/*compare beyond HV table*/
extern int BeyondHVTable(int mode, int digital, short hporch, short vporch, HVTable *pp);

/*initial hporch and vporch value*/
extern  int InitHVTable(HVTable *pp);

/*save remote control index*/
extern int SaveRemoteCtrlIndex(int sock, int index, int length);
extern int SaveLocalRemoteCtrlIndex(void);
/*����ͼ��ı߿�*/
extern int RevisePictureFrame(unsigned char *data, int len);

/*Read Remote Protocol*/
extern int ReadRemoteCtrlIndex(char *config_file, int *index);

extern int  ConfigGetKey(void *CFG_file, void *section, void *key, void *buf);
/*get control frame rate value*/
extern unsigned int GetCtrlFrame();
/*get control frame low rate value*/
extern unsigned int GetCtrlLowRateFrame();
/*�� I ֡���*/
extern int ReadIframeInterval(char *config_file);


#endif //__SYSPARAM__H
