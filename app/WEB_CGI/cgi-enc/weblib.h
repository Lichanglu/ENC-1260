#ifndef _WEBINTERFACE_H
#define _WEBINTERFACE_H 



#include "../../middle_control.h"




/*�õ�Audio����*/
extern int WebGetVideoParam(VideoParam *video);
/*�õ�Video����*/
extern int WebGetAudioParam(AudioParam *audio);
/*�õ�ϵͳ����*/
extern int WebGetSysParam(SYSPARAMS* sys);
/*�����Ļ*/
extern int Webaddtexttodisplay(textinfo* info);

/*����ֵ0�ɹ���-1ʧ��*/
/*������Ƶ����*/
extern int WebSetAudioParam(AudioParam *inaudio,AudioParam *outaudio);

/*������Ƶ����*/
extern int WebSetVideoParam(VideoParam* invideo,VideoParam *outvideo);
/*����ϵͳ����*/
extern int WebSetSysParam(SYSPARAMS *insys,SYSPARAMS *outsys);
/*Web get input source*/
extern int  WebGetinputSource(inputtype *outval);
/*�л�����Դ*/
extern int WebSetinputSource(inputtype inval,inputtype *outval);
/*4.ϵͳ����*/
extern int WebUpdateFile(char* filename);
/*�ϴ�logo ͼƬ*/
extern int WebUploadLogo(char* filename);
/*5.ϵͳ����*/
extern int webRebootSystem(void);
/*ʱ��ͬ��*/
extern int Websynctime(DATE_TIME_INFO* inval,DATE_TIME_INFO *outval);

//extern void WebGetkernelversion(char* version);
//extern void WebGetfpgaversion(char* version);
extern int WebSaveParam(void);
/*get Multi */
extern int WebGetMultiAddr(MultAddr *ip,int protocol);
/*start Multi */
extern int WebSetMultiStart(MultAddr *ip,int protocol);
/*start Multi */
extern int WebSetMultiStop(int outval);
/*get protocol*/
extern int WebGetProtocol(Protocol *pro);
/*web far control start*/
extern int webFarCtrlCamera(int addr,int type,int speed);
/*remote control protocol*/
extern int WebGetCtrlProto(int *index);
/*set control protocol*/
extern int WebSetCtrlProto(int index,int *outdex);
/*web revise picture*/
extern int WebRevisePicture(short hporch,short vporch);
//extern int WebBlueRevise(void);
//��ȡɫ�ʿռ�ֵ��YUV or RGB
extern int webGetColorSpace(ColorSpace *inval);
//����ɫ�ʿռ�ֵ��
extern int webSetColorSpace(ColorSpace inval,ColorSpace *outval);

extern int webAddLOGODisplay(LogoInfo * info);
//��ȡ�豸�ͺš�
extern int WebGetDevideType(char *outval,int len);
extern int WebSetDeviceType(const char *inval,char *outval);    // devicetype len must be 15 bit
//�ظ���������
extern int restoreSet(void);
//��ȡ��̬ip��־��
//extern int webGetDHCPFlag(int *outval);
//�����Ƿ�̬��ȡip��
//extern int webSetDHCPFlag(int inval,int *outval);
//��ȡrtsp��Э�����
extern int webGetRtspParam(RtspParam *outval);
//����rtsp��Э�����
extern int webSetRtspParam(RtspParam  *inval,RtspParam  *outval);
//��ȡý����Э�������
extern int webGetStreamParam(StreamParam *outval);
//����ý����Э�������
extern int webSetStreamParam(StreamParam *inval,StreamParam *outval);
//��ȡH264�߼����������
extern int webGetH264encodeParam(H264EncodeParam *outval);
//H264�߼�����������á�
extern int webSetH264encodeParam(H264EncodeParam *inval,H264EncodeParam *outval);
//��ȡ��Ƶ���������
int webGetOutputVideoParam(OutputVideoInfo *outval);
//���������Ƶ������
extern int webSetOutputVideoParam(OutputVideoInfo *inval,OutputVideoInfo *outval);
//�����ź�Դ����ϸ��Ϣ��
extern int webSignalDetailInfo(char *buf,int len);
//��Ļ΢����
extern int webRevisePicture(short hporch,short vporch);
//��ȡ�����ź�Դ��Ϣ��
extern int webgetInputSignalInfo(char *outval);
//��ȡcpu ռ���ʡ�
extern int webGetCPULoad(int *outval);
//��ȡ����汾����ʱ�䡣
extern int webGetSoftConfigTime(void);
//extern int WebGetPHYmod(void);
//extern int WebSetPHYmod(int mod);









#endif
