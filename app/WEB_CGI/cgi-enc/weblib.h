#ifndef _WEBINTERFACE_H
#define _WEBINTERFACE_H 



#include "../../middle_control.h"




/*得到Audio参数*/
extern int WebGetVideoParam(VideoParam *video);
/*得到Video参数*/
extern int WebGetAudioParam(AudioParam *audio);
/*得到系统参数*/
extern int WebGetSysParam(SYSPARAMS* sys);
/*添加字幕*/
extern int Webaddtexttodisplay(textinfo* info);

/*返回值0成功，-1失败*/
/*设置音频参数*/
extern int WebSetAudioParam(AudioParam *inaudio,AudioParam *outaudio);

/*设置视频参数*/
extern int WebSetVideoParam(VideoParam* invideo,VideoParam *outvideo);
/*设置系统参数*/
extern int WebSetSysParam(SYSPARAMS *insys,SYSPARAMS *outsys);
/*Web get input source*/
extern int  WebGetinputSource(inputtype *outval);
/*切换输入源*/
extern int WebSetinputSource(inputtype inval,inputtype *outval);
/*4.系统升级*/
extern int WebUpdateFile(char* filename);
/*上传logo 图片*/
extern int WebUploadLogo(char* filename);
/*5.系统重启*/
extern int webRebootSystem(void);
/*时间同步*/
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
//获取色彩空间值，YUV or RGB
extern int webGetColorSpace(ColorSpace *inval);
//设置色彩空间值，
extern int webSetColorSpace(ColorSpace inval,ColorSpace *outval);

extern int webAddLOGODisplay(LogoInfo * info);
//获取设备型号。
extern int WebGetDevideType(char *outval,int len);
extern int WebSetDeviceType(const char *inval,char *outval);    // devicetype len must be 15 bit
//回复出厂设置
extern int restoreSet(void);
//获取动态ip标志。
//extern int webGetDHCPFlag(int *outval);
//设置是否动态获取ip。
//extern int webSetDHCPFlag(int inval,int *outval);
//获取rtsp流协议参数
extern int webGetRtspParam(RtspParam *outval);
//设置rtsp流协议参数
extern int webSetRtspParam(RtspParam  *inval,RtspParam  *outval);
//获取媒体流协议参数。
extern int webGetStreamParam(StreamParam *outval);
//设置媒体流协议参数。
extern int webSetStreamParam(StreamParam *inval,StreamParam *outval);
//获取H264高级编码参数。
extern int webGetH264encodeParam(H264EncodeParam *outval);
//H264高级编码参数设置。
extern int webSetH264encodeParam(H264EncodeParam *inval,H264EncodeParam *outval);
//获取视频输出参数。
int webGetOutputVideoParam(OutputVideoInfo *outval);
//设置输出视频参数。
extern int webSetOutputVideoParam(OutputVideoInfo *inval,OutputVideoInfo *outval);
//输入信号源的详细信息。
extern int webSignalDetailInfo(char *buf,int len);
//屏幕微调。
extern int webRevisePicture(short hporch,short vporch);
//获取输入信号源信息。
extern int webgetInputSignalInfo(char *outval);
//获取cpu 占用率。
extern int webGetCPULoad(int *outval);
//获取软件版本编译时间。
extern int webGetSoftConfigTime(void);
//extern int WebGetPHYmod(void);
//extern int WebSetPHYmod(int mod);









#endif
