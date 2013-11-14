
//add by zhangmin
#ifdef GDM_RTSP_SERVER
#ifndef _APP_RTSP_CENTER_H__
#define _APP_RTSP_CENTER_H__

enum{
	APP_CENTER_STOP = 0,
	APP_CENTER_START = 1,
	APP_CENTER_MAX
};


int app_center_init(char* pCom);
int app_center_set_stream_flag(int flag);
int app_center_get_stream_flag(void);

#endif


#endif

