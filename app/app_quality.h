#ifndef _APP_QUALITY_H__
#define _APP_QUALITY_H__

#ifdef HAVE_QUALITY_MODULE
/*define*/
#define ZOOMIN		(0xAAAAAA55)  // 0
#define ZOOMOUT		(0xAA55AA55)  // 1
#define RESIZE_INVALID		(-1)

#define ISZOOMOUT		(ZOOMOUT == gResizeP.flag)//
#define ISZOOMIN		(ZOOMIN == gResizeP.flag)//放大
#define NORESIZE		(RESIZE_INVALID == gResizeP.flag)

#define RECREATE_OK				(0x55AA55AA)
#define RECREATE_INVALID		(-1)

#define NOT_QUA_LEVEL               0       //非高质量
#define QUA_LEVEL                   1       //高质量

typedef enum {
    LOCK_OUT = 0,
    LOCK_IN
} CurrentResolutionStatus;

int AppIsResizeFlag(void);
int AppGetResizeReset(void);
void AppSetResizeReset(int flag);
int AppGetResizeHeight(void);
void AppSetResizeHeight(int height);
int AppGetResizeWidth(void);
void AppSetResizeWidth(int width);
int AppSetQualityResize(unsigned char data[], int len);
int AppISresize(void);
int AppQuality422YUV(Resize_Handle hRsz, Buffer_Handle hSrcBuf, Buffer_Handle hDstResize);

int LockCurrentResolution(unsigned char *data, int len);
void unLockCurrentResolution(void);

int calcLockResolution(OutputResolution val);


#endif


#endif
