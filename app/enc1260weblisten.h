#ifndef _ENC1260WEBLISTEN_H
#define _ENC1260WEBLISTEN_H

#include "WEB_CGI/cgi-enc/webTcpCom.h"





extern void msgPacket(int identifier,unsigned char *data, webparsetype type, int len, int cmd, int ret);
extern int rebootSys(int data, int *out);
extern int getinputType(int in, int *out);
extern int getsoftconfigtime(int identifier,int fd, int cmd, char *data, int vallen);
extern int getcpuload(int in, int *out);
extern int setColorSpace(int data, int *out);
extern int getColorSpace(int *out);
extern int restoresetparse(int in, int *out);
extern int setInputType(int in , int *out);
extern int getCtrlProto(int in, int *out);
extern int setCtrlProto(int data, int *out);
extern int saveParams(int fd, int cmd, int *data);
extern int getHDCPValue(int data, int *out);
extern int webGetMute(int data, int *out);
extern int webSetMute(int data, int *out);
extern int revisePicture(int identifier,int fd, int cmd, char *data, int vallen);
extern int getSignaldetailInfo(int identifier,int fd, int cmd, char *data, int vallen);
extern int getInputSignalInfo(int identifier,int fd, int cmd, char *data, int vallen);
extern int app_get_build_time(int identifier,int fd, int cmd, char *data, int vallen);
extern int getOutputVideoInfo(int identifier,int fd, int cmd, char *data, int vallen);
extern int setOutputVideoInfo(int identifier,int fd, int cmd, char *data, int vallen);
extern int getSysParams(int identifier,int fd, int cmd, char *data, int vallen);
extern int setSysParams(int identifier,int fd, int cmd, char *data, int vallen);
extern int getEncodeTime(int identifier,int fd, int cmd, char *data, int vallen);
extern int farControl(int identifier,int fd, int cmd, char *data, int vallen);
extern int SetDeviceType_1260(int identifier,int fd, int cmd, char *data, int vallen);
extern int getDeviceType_1260(int identifier,int fd, int cmd, char *data, int vallen);
extern int GetAudioParam_1260(int identifier,int fd, int cmd, char *data, int vallen);
extern int setAudioParam_1260(int identifier,int fd, int cmd, char *data, int vallen);
extern int syncTime(int identifier,int fd, int cmd, char *data, int vallen);
extern int midParseInt(int identifier,int fd, char *data, int len);
extern int midParseString(int identifier,int fd, char *data, int len);
extern int midParseStruct(int identifier,int fd, char *data, int len);
extern int app_web_stream_output_process_int(int cmd, int in, int *out);
extern int app_web_stream_output_process_struct(int identifier,int fd, int cmd, char *data, int valen);
#endif
