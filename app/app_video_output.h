#ifndef _APP_VIDEO_OUTPUT_H
#define _APP_VIDEO_OUTPUT_H

int initOutputVideoParam(void);
void getOutputvideohandle(OutputVideoInfo *out);
int writeOutputVideoParam(void);
int DoLockResolution(int in, int *out);
int webgetOutputResolution(int in, int *out);
int getOutputResolution(void);
void setOutputResolution(int val);
int getResizeMode(int in, int *out);
int setResizeMode(int in , int *out);
int getEncodelevel(int in , int *out);
int setEncodelevel(int in, int *out);
int setSceneconfig(int in, int *out);
int getSceneconfig(int data, int *out);
int getFrameRate(int data, int *out);
int setFrameRate(int data, int *out);
int getIFrameInterval(int data, int *out);
//int setIFrameInterval(int data, int *out);
int getBitRate(int data, int *out);
int setBitRate(int data, int *out);
void setlogotextflag(int flag);
int readOutputVideoParam(char *config_file, OutputVideoInfo *Param);
int setOutputParame(OutputVideoInfo *pnew, OutputVideoInfo *pold);


#endif
