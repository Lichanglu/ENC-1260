#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <stdint.h>
#endif
#include "mpegts.h"
#include "stream.h"

#define TS_PACKET_SIZE 188

#define STREAM_TYPE_VIDEO_MPEG1     0x01
#define STREAM_TYPE_VIDEO_MPEG2     0x02
#define STREAM_TYPE_AUDIO_MPEG1     0x03
#define STREAM_TYPE_AUDIO_MPEG2     0x04
#define STREAM_TYPE_PRIVATE_SECTION 0x05
#define STREAM_TYPE_PRIVATE_DATA    0x06
#define STREAM_TYPE_AUDIO_AAC       0x0f
#define STREAM_TYPE_AUDIO_AAC_LATM  0x11
#define STREAM_TYPE_VIDEO_MPEG4     0x10
#define STREAM_TYPE_VIDEO_H264      0x1b
#define STREAM_TYPE_VIDEO_VC1       0xea
#define STREAM_TYPE_VIDEO_DIRAC     0xd1

#define STREAM_TYPE_AUDIO_AC3       0x81
#define STREAM_TYPE_AUDIO_DTS       0x8a

#define  Peek_Short(ptr) (*(ptr) << 8 | *((ptr) + 1))


#define MAX_PAT_BUF_SIZE 1024
#define MAX_PMT_BUF_SIZE 1024
#define MAX_VIDEO_DATA_BUF_SIZE (1024*1024)
#define MAX_AUDIO_DATA_BUF_SIZE (8*1024)


int32_t mpegts_init(MpegTSContext * pMpegTSCtx)
{
	pMpegTSCtx->bGetPAT = 0;
	pMpegTSCtx->bGetPMT = 0;
	pMpegTSCtx->iAudioPid = 0x1fff;
	pMpegTSCtx->iVideoPid = 0x1fff;
	pMpegTSCtx->uiVideoPts = -1;
	pMpegTSCtx->uiVideoDts = -1;
	pMpegTSCtx->uiAudioPts = -1;
	pMpegTSCtx->iLastAudioCC = -1;
	pMpegTSCtx->iLastVideoCC = -1;
	pMpegTSCtx->pAudioDataBuf = (unsigned char *) malloc(MAX_AUDIO_DATA_BUF_SIZE);
	if (pMpegTSCtx->pAudioDataBuf == NULL)
	{
		PRINTF("mpegts_init:malloc fail error!!!");
		return -1;
	}
	pMpegTSCtx->pVideoDataBuf = (unsigned char *) malloc(MAX_VIDEO_DATA_BUF_SIZE);
	if (pMpegTSCtx->pVideoDataBuf == NULL)
	{
		PRINTF("mpegts_init:malloc fail error!!!");
		return -1;
	}
	pMpegTSCtx->iVideoDataBufSize = 0;
	pMpegTSCtx->iAudioDataBufSize = 0;
	pMpegTSCtx->uiAudioPesLen = 0;
	pMpegTSCtx->uiVideoPesLen = 0;
	/*
	pMpegTSCtx->fGetAudioEs = NULL;
	pMpegTSCtx->fGetVideoEs = NULL;
	pMpegTSCtx->fCCEvent = NULL;
	*/
	return 0;
}

int32_t mpegts_destroy(MpegTSContext * pMpegTSCtx)
{
	if (pMpegTSCtx)
	{
		if (pMpegTSCtx->pAudioDataBuf)
		{
			free(pMpegTSCtx->pAudioDataBuf);
			pMpegTSCtx->pAudioDataBuf = NULL;
		}
		if (pMpegTSCtx->pVideoDataBuf)
		{
			free(pMpegTSCtx->pVideoDataBuf);
			pMpegTSCtx->pVideoDataBuf = NULL;
		}
	}
	return 0;
}
int32_t mpegts_register_videoes_cb(TS_Handle* ts_handle,VideoEsCallBack fCallBack)
{
	if(fCallBack)
	{
		ts_handle->fGetVideoEs = fCallBack;
	}
	return 0;
}
int32_t mpegts_register_audioes_cb(TS_Handle * ts_handle,AudioEsCallBack fCallBack)
{
	if(fCallBack)
	{
		ts_handle->fGetAudioEs = fCallBack;
	}
	return 0;
}
int32_t mpegts_register_ccevent_cb(TS_Handle * ts_handle,Msg_func fCallBack)
{
	if(fCallBack)
	{
		ts_handle->fCCEvent = fCallBack;
	}
	return 0;
}

const int avpriv_mpeg4audio_sample_rates[16] = {
    96000, 88200, 64000, 48000, 44100, 32000,
		24000, 22050, 16000, 12000, 11025, 8000, 7350
};

int aac_parse_header(AACADTSHeaderInfo *hdr,uint8_t * pBuf,int iBufLen)
{
    int size, rdb, ch, sr;

	if (!pBuf || iBufLen < 4) return -1;
	
    if((Peek_Short(pBuf) & 0xfff0) != 0xfff0)
        return -1;

    
    sr      = (pBuf[2] >> 2) & 0x03;  /* sample_frequency_index */
	if (!avpriv_mpeg4audio_sample_rates[sr])
	{
		PRINTF("audio sample rate index %d error!",sr);
		return -1;
	}
    ch    =  ((pBuf[2] << 2 ) & 0x04) | (pBuf[3] >> 6);  /* channel_configuration */
	hdr->sample_rate = avpriv_mpeg4audio_sample_rates[sr];
	hdr->chan_config = ch;
    return 0;
}

int32_t mpegts_parse(MpegTSContext * pMpegTSCtx,TS_Handle *ts_handle,uint8_t * pBuf)
{
	int32_t i = 0;
	uint8_t * pTsBuf;
	int32_t iBufOffset = 0;
	int32_t iTsBufOffset = 0;
	int32_t iPid = 0x1fff;
	ePidType pidtype = PID_INVALID;
	uint8_t bPayloadStart = 0;
	uint8_t adaptation_field_control;
	uint8_t adaptation_field_length;
	uint8_t uiCC;
	int32_t iBufSize = ts_handle->len;

	if (!pMpegTSCtx || !pBuf || iBufSize <= 0)
		return -1;

	while(pBuf[i] != 0x47 && i <iBufSize)
	{
		i++;
	}
	iBufOffset += i;
	pTsBuf = pBuf + iBufOffset;
	if(pTsBuf[0] != 0x47)
	{
		PRINTF("mpegts_parse:can't find mpeg ts sync code error!!!\n");
		return -2;//·ÇÊÓÆµÁ÷£¬³ÌÐòÍ£Ö¹
	}
	
	while(iBufOffset < iBufSize)
	{
		iTsBufOffset = 0;
		iPid = Peek_Short(pTsBuf + 1) & 0x1fff;
		if (!pMpegTSCtx->bGetPAT)
		{
			if (iPid == 0)
			{
				pidtype = PID_PAT;
			}
			else
			{
				iBufOffset += TS_PACKET_SIZE;
				pTsBuf = pBuf + iBufOffset;
				continue;
			}
		}
		else if (!pMpegTSCtx->bGetPMT)
		{
			if (iPid == pMpegTSCtx->iPMTPid)
			{
				pidtype = PID_PMT;
			}
			else
			{
				iBufOffset += TS_PACKET_SIZE;
				pTsBuf = pBuf + iBufOffset;
				continue;
			}
		}
		else if(iPid == pMpegTSCtx->iVideoPid)
		{
			pidtype = PID_VIDEO;
		}
		else if (iPid == pMpegTSCtx->iAudioPid)
		{
			pidtype = PID_AUDIO;
		}
		else if (iPid == 0x1fff)
		{
			iBufOffset += TS_PACKET_SIZE;
			pTsBuf = pBuf + iBufOffset;
			continue;
		}
		else 
		{
			//	PRINTF("mpegts_parse:warning skip pid %d!!!\n",iPid);
			pidtype = PID_INVALID;
		}

		if ((pTsBuf[1] & 0x80) == 0x80)
		{
			PRINTF("mpegts_parse:mpeg ts transport error!!!");
			return -1;
		}
		bPayloadStart = (pTsBuf[1] & 0x40) == 0x40;

		adaptation_field_control = (pTsBuf[3] >> 4) & 0x03;
		uiCC = pTsBuf[3] & 0x0f;

		pTsBuf += 4;
		iTsBufOffset +=4;
		i = 0;
		if (adaptation_field_control == 2 || adaptation_field_control == 3)
		{
			adaptation_field_length = pTsBuf[0];
			pTsBuf += 1;
			iTsBufOffset += 1;
			if (adaptation_field_length > 0)
			{
				pTsBuf += adaptation_field_length;
				iTsBufOffset += adaptation_field_length;
			}
		}
		if (pidtype == PID_PAT || pidtype == PID_PMT)
		{
			if(bPayloadStart)
			{
				pTsBuf += pTsBuf[0] + 1;
				iTsBufOffset +=  pTsBuf[0] + 1;
			}
		}
		switch(pidtype)
		{
			case PID_INVALID:
				break;
			case PID_PAT:
				if (bPayloadStart)
				{
					uint16_t section_length;
					uint16_t transport_stream_id;
					uint32_t iPrograms = 0;
					uint16_t program_number;
					uint16_t uiPid;

					assert(pTsBuf[0] == 0);
					assert((pTsBuf[1] & 0xc0) == 0x80);
					section_length = Peek_Short(pTsBuf + 1) & 0x0fff;
					transport_stream_id = pTsBuf[3] << 8 | pTsBuf[4];
					PRINTF("mpegts_parse:ts pat transport stream id %d\n",transport_stream_id);
					
					pTsBuf += 5;
					pTsBuf += 3;
					iTsBufOffset += 8;

					iPrograms = (TS_PACKET_SIZE - 4 - iTsBufOffset)/4;

					for(i = 0;i < iPrograms;i++)
					{
						program_number = Peek_Short(pTsBuf);
						PRINTF("mpegts_parse:program number %d\n",program_number);
						uiPid = Peek_Short(pTsBuf + 2) & 0x1fff;
						if (program_number == 0)
						{
							PRINTF("mpegts_parse:network pid %d",uiPid);
						}
						else
						{
							PRINTF("mpegts_parse:pmt pid %d\n",uiPid);
							pMpegTSCtx->iPMTPid = uiPid;
						}
					}
					pMpegTSCtx->bGetPAT = 1;
					PRINTF("mpegts_parse:get ts pat!!!\n");
				}
				else
				{
					PRINTF("mpegts_parse:pat packets num > 1 is not support!!!");
				}
				break;
			case PID_PMT:
				if (bPayloadStart)
				{
					uint16_t section_length;
					//uint16_t transport_stream_id;
					uint32_t iPrograms = 0;
					uint16_t program_number;
					uint16_t uiPid;
					uint16_t uiPcrPid;
					uint16_t program_info_length;
					
					assert(pTsBuf[0] == 0x02);
					assert((pTsBuf[1] & 0xc0) == 0x80);
					section_length = Peek_Short(pTsBuf +1) & 0x0fff;
					program_number = Peek_Short(pTsBuf +3);
					
					pTsBuf += 5;
					pTsBuf += 3;
					iTsBufOffset += 8;

					uiPcrPid = Peek_Short(pTsBuf) & 0x1fff;
					
					program_info_length = Peek_Short(pTsBuf + 2) & 0x0fff;

					pTsBuf += 4 + program_info_length;
					iTsBufOffset += 4 + program_info_length;

					while(iTsBufOffset < TS_PACKET_SIZE - 4)
					{
						uint8_t stream_type;
						uint16_t es_info_length;

						stream_type = pTsBuf[0];
						uiPid = Peek_Short(pTsBuf + 1) & 0x1fff;
						switch (stream_type)
						{
						case STREAM_TYPE_AUDIO_MPEG1:
						case STREAM_TYPE_AUDIO_MPEG2:
						case STREAM_TYPE_AUDIO_AAC:
						case STREAM_TYPE_AUDIO_AAC_LATM:
						case STREAM_TYPE_AUDIO_AC3:
						case STREAM_TYPE_AUDIO_DTS:
							PRINTF("mpegts_parse:dts audio\n");
							pMpegTSCtx->iAudioPid = uiPid;
							pMpegTSCtx->iAudioStreamType = stream_type;
							break;

						case STREAM_TYPE_VIDEO_MPEG1:
						case STREAM_TYPE_VIDEO_MPEG2:
						case STREAM_TYPE_VIDEO_MPEG4:
						case STREAM_TYPE_VIDEO_H264:
						case STREAM_TYPE_VIDEO_VC1:
						case STREAM_TYPE_VIDEO_DIRAC:
							pMpegTSCtx->iVideoPid = uiPid;
							pMpegTSCtx->iVideoStreamType = stream_type;
							break;

						default:
							PRINTF("mpegts_parse:warning skip stream type %d!!!",stream_type);
							break;
						}
						PRINTF("mpegts_parse:stream type %d pid %d \n",stream_type,uiPid);
						pTsBuf += 3;
						iTsBufOffset += 3;
						if (iTsBufOffset +2 <= TS_PACKET_SIZE - 4)
						{
							es_info_length = Peek_Short(pTsBuf) & 0xfff;
						    pTsBuf += 2 + es_info_length;
						    iTsBufOffset += 2 + es_info_length;
						}
					}
					pMpegTSCtx->bGetPMT = 1;
					PRINTF("mpegts_parse:get ts pmt!!!\n");
				}
				else
				{
					PRINTF("mpegts_parse:pmt packets num > 1 is not support!!!");
				}
				
				break;
			case PID_VIDEO:
			case PID_AUDIO:
				if (pidtype == PID_VIDEO) 
				{
					if ((pMpegTSCtx->iLastVideoCC + 1) % 16 != uiCC && pMpegTSCtx->iLastVideoCC != -1)
					{
						if (ts_handle->fCCEvent)
							ts_handle->fCCEvent(TS_STREAM,MSG_LOST_FRAME);
						PRINTF("mpegts_parse:warning video cc is discontinuous!! last cc %d current cc %d\n",pMpegTSCtx->iLastVideoCC,uiCC);
					}
					pMpegTSCtx->iLastVideoCC = uiCC;
				}
				if (pidtype == PID_AUDIO) 
				{
					if ((pMpegTSCtx->iLastAudioCC + 1) % 16 != uiCC && pMpegTSCtx->iLastAudioCC != -1)
					{
						if (ts_handle->fCCEvent)
							ts_handle->fCCEvent(TS_STREAM,MSG_LOST_FRAME);
						PRINTF("mpegts_parse:warning audio cc is discontinuous!! last cc %d currentt cc %d\n",pMpegTSCtx->iLastAudioCC,uiCC);
					}
					pMpegTSCtx->iLastAudioCC = uiCC;
				}
				
				if (bPayloadStart)
				{
					uint8_t stream_id;
					uint16_t uiPes_packet_length = 0;
					uint8_t pts_dts_flags;
					uint8_t pes_header_data_length;
					uint64_t pts = 0;
					uint64_t dts = 0;
					int32_t iPayloadLen;
					
					if (pMpegTSCtx->iVideoDataBufSize > 0 && pidtype == PID_VIDEO)
					{
#if 0		
//¼æÈÝÀÊ³Û
				if (ts_handle->fGetVideoEs){	
					TRACE("pVideoDataBuf[3]\[4]=0x%x,0x%x,0x%x,0x%x,0x%x\n",pMpegTSCtx->pVideoDataBuf[3],pMpegTSCtx->pVideoDataBuf[4],
					pMpegTSCtx->pVideoDataBuf[5],pMpegTSCtx->pVideoDataBuf[6],pMpegTSCtx->pVideoDataBuf[7]);
					TRACE("pVideoDataBuf[5]\[6]=0x%x,0x%x,0x%x,0x%x,0x%x\n",pMpegTSCtx->pVideoDataBuf[8],pMpegTSCtx->pVideoDataBuf[9],
					pMpegTSCtx->pVideoDataBuf[10],pMpegTSCtx->pVideoDataBuf[11],pMpegTSCtx->pVideoDataBuf[12]);
														
					
				if( pMpegTSCtx->pVideoDataBuf[10] == 0x67)
				{
					TRACE(" pMpegTSCtx->pVideoDataBuf[10]=0x%x", pMpegTSCtx->pVideoDataBuf[10]);
					ts_handle->frame.I_frame = 1;
					read_gaps_in_frame(pMpegTSCtx->pVideoDataBuf+10,&(ts_handle->frame.width),&(ts_handle->frame.high),pMpegTSCtx->iVideoDataBufSize);
					ts_handle->frame.width = 704;
					ts_handle->frame.high = 576;
					flag=1;
				}else 
					ts_handle->frame.I_frame = 0;
					if(flag==1)
						ts_handle->fGetVideoEs(pMpegTSCtx->pVideoDataBuf+6,pMpegTSCtx->iVideoDataBufSize-6,&(ts_handle->frame));
					}										

#endif
						if (ts_handle->fGetVideoEs){									
							if( pMpegTSCtx->pVideoDataBuf[4] == 0x67)
							{
								ts_handle->frame.I_frame = 1;
								read_gaps_in_frame(pMpegTSCtx->pVideoDataBuf+4,&(ts_handle->frame.width),&(ts_handle->frame.high),pMpegTSCtx->iVideoDataBufSize);
							}else 
								ts_handle->frame.I_frame = 0;
							ts_handle->fGetVideoEs(pMpegTSCtx->pVideoDataBuf,pMpegTSCtx->iVideoDataBufSize,&(ts_handle->frame));
							

							pMpegTSCtx->uiVideoPesLen = 0;
							pMpegTSCtx->iVideoDataBufSize = 0;							
						
							}
					}
					else if (pMpegTSCtx->iAudioDataBufSize > 0 && pidtype == PID_AUDIO)
					{
						static int32_t sr = 48000,ch = 2;
						
						if (pMpegTSCtx->uiAudioPesLen != pMpegTSCtx->iAudioDataBufSize)
							PRINTF("mpegts_parse:pes audio data error pes len %d data len %d!!!\n",pMpegTSCtx->uiAudioPesLen,pMpegTSCtx->iAudioDataBufSize);

						if (pMpegTSCtx->iAudioStreamType == STREAM_TYPE_AUDIO_AAC)
						{
							static AACADTSHeaderInfo AdtsHeader = {0};
							if (AdtsHeader.sample_rate == 0)
							{
								aac_parse_header(&AdtsHeader,pMpegTSCtx->pAudioDataBuf,pMpegTSCtx->iAudioDataBufSize);
								sr = AdtsHeader.sample_rate;
								ch = AdtsHeader.chan_config;
							}

						}

						if (ts_handle->fGetAudioEs){
							rtsp_stream_audio_filter_sdp_info(pMpegTSCtx->pAudioDataBuf,&(ts_handle->frame.sample_rate));
							ts_handle->fGetAudioEs(pMpegTSCtx->pAudioDataBuf,pMpegTSCtx->iAudioDataBufSize,&(ts_handle->frame));
	
							pMpegTSCtx->uiAudioPesLen = 0;
							pMpegTSCtx->iAudioDataBufSize = 0;
							}
					}

					assert(pTsBuf[0] == 0 && pTsBuf[1] == 0 && pTsBuf[2] == 1);
					stream_id = pTsBuf[3];
					uiPes_packet_length = Peek_Short(pTsBuf + 4);

					pTsBuf += 6;
					iTsBufOffset += 6;
					if ((stream_id & 0xe0) == 0xe0 || ((stream_id & 0xc0) == 0xc0))
					{
						pts_dts_flags = pTsBuf[1]>>6;
						pes_header_data_length = pTsBuf[2];
						
						if (pts_dts_flags)
						{
							if (pts_dts_flags & 0x2)
							{
								pts = ((pTsBuf[3] >> 1) & 0x7);
								pts <<= 8;
								pts |= pTsBuf[4];
								pts <<= 7;
								pts |= ((pTsBuf[5] >> 1) & 0x7f);
								pts <<= 8;
								pts |= pTsBuf[6];
								pts <<= 7;
								pts |= ((pTsBuf[7] >> 1) & 0x7f);
#ifdef WIN32
						//		PRINTF("mpegts_parse:pts %I64d \n",pts);
#else
						//		PRINTF("mpegts_parse:pts %llu \n",pts);
#endif
								dts = pts;
							}
							if (pts_dts_flags & 0x1)
							{
								dts = ((pTsBuf[8] >> 1) & 0x7);
								dts <<= 8;
								dts |= pTsBuf[9];
								dts <<= 7;
								dts |= ((pTsBuf[10] >> 1) & 0x7f);
								dts <<= 8;
								dts |= pTsBuf[11];
								dts <<= 7;
								dts |= ((pTsBuf[12] >> 1) & 0x7f);
#ifdef WIN32
							//	PRINTF("mpegts_parse:dts %I64d \n",dts);
#else
							//	PRINTF("mpegts_parse:dts %llu \n",dts);
#endif
							}
						}
						
						pTsBuf += pes_header_data_length + 3;
					    iTsBufOffset += pes_header_data_length + 3;
						iPayloadLen = TS_PACKET_SIZE - iTsBufOffset;
						if (pidtype == PID_VIDEO)
						{
							pMpegTSCtx->uiVideoPts = pts;
							pMpegTSCtx->uiVideoDts = dts;
							if (uiPes_packet_length == 0) pMpegTSCtx->uiVideoPesLen =0;
							else pMpegTSCtx->uiVideoPesLen = uiPes_packet_length - pes_header_data_length - 3;
							assert(uiPes_packet_length <= MAX_VIDEO_DATA_BUF_SIZE);
							if (pMpegTSCtx->pVideoDataBuf && iPayloadLen + pMpegTSCtx->iVideoDataBufSize <= MAX_VIDEO_DATA_BUF_SIZE)
							{
								memcpy(pMpegTSCtx->pVideoDataBuf,pTsBuf,iPayloadLen);
								pMpegTSCtx->iVideoDataBufSize = iPayloadLen;
							}
							else
							{
								PRINTF("mpegts_parse:video data buf size %d is not enough!!!",MAX_VIDEO_DATA_BUF_SIZE);
								pMpegTSCtx->iVideoDataBufSize = 0;
								return -1;
							}

						}
						else if (pidtype == PID_AUDIO)
						{
							pMpegTSCtx->uiAudioPesLen = uiPes_packet_length - pes_header_data_length - 3;
							assert(uiPes_packet_length <= MAX_AUDIO_DATA_BUF_SIZE);
							if (pMpegTSCtx->pAudioDataBuf && iPayloadLen + pMpegTSCtx->iAudioDataBufSize <= MAX_AUDIO_DATA_BUF_SIZE)
							{
								memcpy(pMpegTSCtx->pAudioDataBuf,pTsBuf,iPayloadLen);
								pMpegTSCtx->iAudioDataBufSize = iPayloadLen;
							}
							else
							{
								PRINTF("mpegts_parse:audio data buf size %d is not enough!!!",MAX_AUDIO_DATA_BUF_SIZE);
								pMpegTSCtx->iAudioDataBufSize = 0;
								return -1;
							}
						}


					}
				}
				else
				{
					int32_t iPayloadLen;
					iPayloadLen = TS_PACKET_SIZE - iTsBufOffset;
					if (pidtype == PID_VIDEO && pMpegTSCtx->iVideoDataBufSize != 0 )
					{
						if (pMpegTSCtx->pVideoDataBuf && iPayloadLen + pMpegTSCtx->iVideoDataBufSize <= MAX_VIDEO_DATA_BUF_SIZE)
						{
							memcpy(pMpegTSCtx->pVideoDataBuf + pMpegTSCtx->iVideoDataBufSize,pTsBuf,iPayloadLen);
							pMpegTSCtx->iVideoDataBufSize += iPayloadLen;
						}
						else
						{
							PRINTF("mpegts_parse:video data buf size %d is not enough!!!",MAX_VIDEO_DATA_BUF_SIZE);
							return -1;
						}
					}
					else if (pidtype == PID_AUDIO && pMpegTSCtx->iAudioDataBufSize != 0)
					{
						if (pMpegTSCtx->pAudioDataBuf && iPayloadLen + pMpegTSCtx->iAudioDataBufSize <= MAX_AUDIO_DATA_BUF_SIZE)
						{
							memcpy(pMpegTSCtx->pAudioDataBuf + pMpegTSCtx->iAudioDataBufSize,pTsBuf,iPayloadLen);
							pMpegTSCtx->iAudioDataBufSize += iPayloadLen;	
							
					
						}
						else
						{
							PRINTF("mpegts_parse:audio data buf size %d is not enough!!!",MAX_AUDIO_DATA_BUF_SIZE);
							return -1;
						}
					}
					
				}
				break;


		}
	    iBufOffset += TS_PACKET_SIZE;
		pTsBuf = pBuf + iBufOffset;
	}
	return 0;
}


