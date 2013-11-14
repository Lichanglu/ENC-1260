#ifndef _RECORD_H__
#define _RECORD_H__

typedef void (*record_msg_func)(int,void *);
typedef struct RECORD_INFO_T{
	unsigned int type; // TYPE_VIDEO, TYPE_AUDIO , 0 meas video and audio
	unsigned int width;
	unsigned int height;
	unsigned int iframe; // 1 i frame 0 other frame
	unsigned int samplerate;
	unsigned int timetick;
}record_info_t;


typedef struct RECORD_HANDLE_T{
	record_info_t record_info;
	unsigned int streamnum; // 1: only video 2:video and audio		
	record_msg_func fCCEvent; //just for some msg inform
	char *mp4_handle;
}record_handle;

#define TYPE_VIDEO 3
#define TYPE_AUDIO 4

#ifndef PRINTF
#define PRINTF debug_output_file
#endif

record_handle * record_open(char *filename,unsigned int streamnum,record_info_t *info);
int record_set_msg_func(record_handle *handle,void *func);
int record_push_data(record_handle *handle,  char *buff,int len,record_info_t *info);
int record_close(record_handle **handle,char *filename,unsigned int timetick);

#endif

