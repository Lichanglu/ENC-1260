#include "stdafx.h"
#include <stdio.h> 
#include <stdarg.h>
#include "stream.h"

#define DEBUG_FILE  "debugInfo.ini"
static bool print_dbg =false;
static FILE* dbg_fd=NULL;

enum _RESOLUTION{
	RESOLUTION_AUTO = 0,
	P1920X1080,
	P1280X720,
	P1400X1050,
	P1366X768,
	P1280X1024,
	P1280X768,
	P1024X768,
	P720X480,
	P352X288	
}Resolution;

bool open_debug_file()
{
	char buf[32],temp[2];
	int ret=0,i=0;
	if( !print_dbg ){
		FILE* fd = fopen("Riplayer.config","r+");
		if(!fd)
			return false;
		ret = fread(buf,sizeof(char),sizeof(buf),fd);
		if(ret < 0)
			return false;
		while(buf[i] != '\0'){
			if(buf[i] == '='){
				temp[0]=buf[i+1];
				break;
			}
			i++;
		}

		print_dbg = atoi(temp);

		i=0;
		memset(buf,0,32);
		sprintf(buf,"%s",DEBUG_FILE);
		if(print_dbg && (!dbg_fd)){
			while(1){
				i++;
				dbg_fd = fopen(buf,"w");
				if(!dbg_fd){
					sprintf(buf,"%s(%d)",DEBUG_FILE,i);
				}else 
					break;
			}
		}		
	}
	return print_dbg;
}

int  debug_output_file(char* format,...)
{
	char str[1024];
	va_list arg_ptr;
	int ret =0;
	
	CTime time=CTime::GetCurrentTime();
	int hour=time.GetHour();
	int min=time.GetMinute();
	int sec = time.GetSecond();
	sprintf(str,"%d:%d:%d    ",hour,min,sec);
	if(print_dbg&&dbg_fd){
		ret = fwrite(str,sizeof(char),strlen(str),dbg_fd);
		if(ret < 0)
			return -1;
	}
	memset(str,0,1024);
	
	va_start(arg_ptr,format);
	_vsnprintf(str,sizeof(str)-1,format,arg_ptr);
	if(print_dbg&&dbg_fd){
		ret = fwrite(str,sizeof(char),strlen(str),dbg_fd);
		if(ret <0)
			return -1;
	}
	fflush(dbg_fd);
	return ret;
}

void close_debug_file()
{
	if(dbg_fd)
		fclose(dbg_fd);
}



static int get_rate_takt(void)
{
	int takt = 1;
	return  takt;	
}

static void parse_encoder_thread(void* arg)
{	
	Encoder_Handle* encoder_handle = (Encoder_Handle*)arg;
	int tim=0;

	while(1){
		if(1 == encoder_handle->thread_ext)
			break;
		tim++;
		encoder_handle->encoder_info.total_frame += encoder_handle->total_frame_cnt;
		encoder_handle->encoder_info.lost_frame = encoder_handle->lost_frame_cnt;
		if(tim == get_rate_takt()){
			encoder_handle->encoder_info.frame_rate = encoder_handle->total_frame_cnt/tim;
			encoder_handle->encoder_info.bit_rate = (encoder_handle->total_byte_cnt*8)/(1000*tim);
			encoder_handle->total_frame_cnt = 0;
			encoder_handle->total_byte_cnt=0;
			tim=0;
		}
		if(encoder_handle->get_encoder_info)
			encoder_handle->get_encoder_info(&(encoder_handle->encoder_info));
				
		Sleep(1000);
		}
	memset(&(encoder_handle->encoder_info),0,sizeof(Encoder_Info));
	if(encoder_handle->get_encoder_info )
		encoder_handle->get_encoder_info = NULL;
	encoder_handle=NULL;
	return ;
}
/*
	1920X1080,
	1280X720,
	1400X1050,
	1366X768,
	1280X1024,
	1280X768,
	1024X768,
	720X480,
	352X288	
*/

void get_encoder_resolution(Encoder_Info* info ,int width ,int high)
{
	info->width =width;
	info->high = high;
}


void encoder_info_set_cbfunc(Encoder_Handle* handle ,void* function)
{
	if(handle){
	if(NULL == handle->get_encoder_info)
	{
		handle->get_encoder_info = (Get_Encoder_Info_func)function;
	}else 
		PRINTF("encoder_info_set_cbfunc:set fail\n");
		}
}

Encoder_Handle* open_Encoder(const char* src)
{

	Encoder_Handle* encoder_handle = (Encoder_Handle*)malloc(sizeof(Encoder_Handle));

	if( !encoder_handle){
		debug_output_file("open_Encoder:malloc encoder_handle faild\n");
		return NULL;
	}
	memset(encoder_handle,0,sizeof(Encoder_Handle));
	encoder_handle->get_encoder_info = NULL;
	strcpy(encoder_handle->encoder_info.origin,src);
	encoder_handle->thread_ext = 0 ;
	encoder_handle->total_byte_cnt=0;
	encoder_handle->total_frame_cnt=0;
	encoder_handle->lost_frame_cnt=0;
	_beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))parse_encoder_thread,encoder_handle, 0, 0);

	return encoder_handle;
}

void close_Encoder_Info(Encoder_Handle* handle)
{
	handle->thread_ext = 1;
}


