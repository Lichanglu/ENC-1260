/*********************************************************************
��˾����:     Reach Software
�ļ���:	   remotectrl.h
�� �� �ˣ�    yangshaohua                    ���ڣ�2009-01-14
�� �� �ˣ�    DUHAO                            ���ڣ�2010-6-22
��������������ͷԶң����ģ��ͷ�ļ�

����˵���� ͨ������ͨѶ��������ͷ����ת�Ȳ���
**********************************************************************/
#include "sysparam.h"
#include "demo.h"
#ifndef __REMOTECTRL__
#define __REMOTECTRL__
#define SLID_MAX_NUM  			11
#define ADDR_MAX_NUM			101   //struct __ADDRESS__  default addr 1-100
#define ADDR_CMD_MAX_NUM		50   //struct __CTRLCODE__

#define UPSTART					0
#define UPSTOP					1

#define DOWNSTART				2
#define DOWNSTOP				3

#define LEFTSTART				4
#define LEFTSTOP				5

#define RIGHTSTART				6
#define RIGHTSTOP				7

#define DADDSTART				8
#define DADDSTOP				9

#define DDECSTART				10
#define DDECSTOP				11

#define POSITIONPRESET			12//Ԥ��λ
#define POSITIONPRESETSET		13//����Ԥ��λ

#define CTRL_CODE_MAX_NUM		14   //struct __CTRLCODE__
#define PROTNUM 				5	//Զ��ң��Э���ܸ���
/*Զҡ���ƽṹ�������ļ�*/
typedef struct __HVSEEPSLID__ {
	BYTE speed[SLID_MAX_NUM];
} HVSSlid;

typedef struct __ADDRESS__ {
	BYTE addr[ADDR_MAX_NUM];
} ADDR;

typedef struct __CTRLCODE__ {
	char comm[ADDR_CMD_MAX_NUM];
} CTRLCODE;

typedef struct __COMPARAM__ {
	int stopbits;
	int databits;
	int parity;
	int baud;
} COMPARAM;

typedef struct __REMOTE_CONFIG__ {
	BYTE ptzName[50];
	/*ˮƽ����*/
	HVSSlid HSSlid;
	/*��ֱ����*/
	HVSSlid VSSlid;
	/*ָ���ַ*/
	ADDR Addr;
	/*��������*/
	CTRLCODE CCode[CTRL_CODE_MAX_NUM];
	/*��������*/
	COMPARAM comm;
} RemoteConfig;


/*����ͷ�Ŀ���*/
extern int CameraControl();
/*��ʼ��Զҡ�������ļ�*/
extern int InitRemoteStruct(int);

/*����ͷ��ʼ��*/
extern int CameraCtrlInit(int com);
extern int InitPort(int port_num, int baudrate, int databits, int stopbits, int parity);
/*ԶҡAPI */
extern int CCtrlUpStart(int fd, int addr, int speed);
extern int CCtrlUpStop(int fd, int addr);
extern int CCtrlDownStart(int fd, int addr, int speed);
extern int CCtrlDownStop(int fd, int addr);
extern int CCtrlLeftStart(int fd, int addr, int speed);
extern int CCtrlLeftStop(int fd, int addr);
extern int CCtrlRightStart(int fd, int addr, int speed);
extern int CCtrlRightStop(int fd, int addr);
extern int CCtrlZoomAddStart(int fd, int addr);
extern int CCtrlZoomAddStop(int fd, int addr);
extern int CCtrlZoomDecStart(int fd, int addr);
extern int CCtrlZoomDecStop(int fd, int addr);
extern void FarCtrlCamera(int dsp, unsigned char *data, int len);
extern int gRemoteFD;
extern RemoteConfig gRemote;
extern char FarCtrlList[7][20];
#endif   //__REMOTECTRL__


