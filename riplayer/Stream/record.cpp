/*******************************************************************
*  ·â×ªMP4Â¼ÖÆµÄ¿â
*
*
********************************************************************/
#include "stdafx.h"
#include "../Riplayer.h"
#include "../RiplayerDlg.h"
#include "../RecordDlg.h"
#include "../PictureDlg.h"
#include "../SetEncodeDlg.h"
#include "../SetDlg.h"
#include "../InfoDlg.h"
#include "../PlayVedioDlg.h"
#include "../RMedia.h"


#include "../mp4/mp4_lib.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

#include "record.h"
//#pragma comment (lib,"mp4/mp4_lib.lib")







/*==============================================================================
    Function: <record_open>
    Descripe:  filename : the record file name  .streamnum : 1 only video ,2 video and audio
    Param: 
    Created By zm 2012.05.16 11:59:24 
==============================================================================*/
record_handle * record_open(char *filename,unsigned int streamnum,record_info_t *info)
{
	if(filename == NULL || strlen(filename) == 0 ||info == NULL)
	{
		PRINTF("Error.\n");
		return NULL;
	}
	unsigned int width = info->width;
	unsigned int height = info->height;
	unsigned int samplerate = info->samplerate;
	unsigned int timetick = info->timetick;
	
	
	record_handle *handle = NULL;
	char *mp4_handle = NULL;
	int ret = 0;
	handle = (record_handle *)malloc(sizeof(record_handle));
	if(handle == NULL)
	{
		PRINTF("Error,malloc handle is failed.\n");
		return NULL;
	}
	memset(handle,0,sizeof(record_handle));

	mp4_handle =  SysInit(filename, streamnum);
	if(mp4_handle == NULL)
	{
		PRINTF("Eroor,mp4 handle create is failed\n");
		free(handle);
		handle = NULL;
		return NULL;
	}

	ret = CreatMpgContainer(0,mp4_handle);


	AddStream(MP4_VIDEO, width, height, 0, 0,timetick, mp4_handle);
	if(streamnum == 2)
	{
		AddStream(MP4_AUDIO, 0, 0, 1, samplerate,timetick, mp4_handle);
	}


	handle->mp4_handle = mp4_handle;
	handle->streamnum = streamnum;
	memcpy(&(handle->record_info), info,sizeof(record_info_t));

	return handle;
	
}

int record_set_msg_func(record_handle *handle,void *func)
{
	if(handle != NULL)
	{
		handle->fCCEvent = (record_msg_func)func;
	}
	return 0;
}

static char record_stat_flag=0;
int record_push_data(record_handle *handle,  char *buff,int len,record_info_t *info)
{
	if(handle == NULL || buff == NULL || info == NULL)
	{
		return -1;
	}
	if(record_stat_flag == 0 && buff[4] == 0x67)
	{
	
		record_stat_flag=1;
	}
	if( record_stat_flag != 1)
		return -1 ;
	
	
	record_stat_flag=1;
		unsigned int id = 0;
		unsigned int type = 0;
		unsigned int ukey = info->iframe;
		unsigned int timetick = info->timetick;
		int ret = 0;
		if(info->type == TYPE_VIDEO)
		{
			id = MP4_VIDEO;
			type = 0;
		}
		else 
		{
			id = MP4_AUDIO;
			type = 1;		
		}
		
		ret = WtOneFrame(buff, len, id, type,ukey, timetick, handle->mp4_handle);	
	return 0;
	
}

int record_close(record_handle **handle,char *filename,unsigned int timetick)
{
	if(handle == NULL || filename == NULL)
		return -1;
	char *mp4_handle = (*handle)->mp4_handle;
	record_stat_flag=0;
	
	FinishMpgContainer(filename,timetick, mp4_handle);	

	free(*handle);
	*handle = NULL;
	
	return 0;
}


