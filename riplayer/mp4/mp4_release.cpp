#include "mp4_common.h"
#include "mp4_release.h"


UINT32  MP4R_VideoHeadRecover(char* pacBuf, STR_MP4_RELEASE_INFO* pstMp4RInfo)
{
	UINT32 uiSliceLen = 0, uiCacheType = 0, uiReadLen = 0;
	UINT32 uiWholeFrameLen = pstMp4RInfo->stCurrent.uiLen;

	//fseek(pstMp4RInfo->pFile, pstMp4RInfo->uiCurFilePos, SEEK_SET);
	
	if(uiWholeFrameLen > 2000000)
	{
		printf("MP4R_VideoHeadRecover, frame len is %d\n", uiWholeFrameLen);
		return FALSE;
	}
	
	while(uiReadLen < uiWholeFrameLen)
	{
		fread(&pacBuf[uiReadLen], sizeof(char), sizeof(UINT32), pstMp4RInfo->pFile);
		uiSliceLen = Swap32(*((UINT32*)(&pacBuf[uiReadLen])));
		if(uiSliceLen < 4 || uiSliceLen > 2000000)
		{
			printf("MP4R_VideoHeadRecover, slicelen is %d\n", uiSliceLen);
			pstMp4RInfo->uiCurFilePos += uiWholeFrameLen;
			return FALSE;
		}
		pacBuf[uiReadLen] = 0;
		pacBuf[uiReadLen + 1] = 0;
		pacBuf[uiReadLen + 2] = 0;
		pacBuf[uiReadLen + 3] = 1;
		
		if(uiReadLen + uiSliceLen < uiWholeFrameLen)
		{
			fread(&pacBuf[uiReadLen + 4], sizeof(char), uiSliceLen, pstMp4RInfo->pFile);
			uiReadLen += (uiSliceLen + sizeof(UINT32));
		}
		else if(uiReadLen > uiWholeFrameLen)
		{
			printf("MP4R_VideoHeadRecover, ReadLen is %d, framelen is %d\n", uiReadLen, uiWholeFrameLen);
			pstMp4RInfo->uiCurFilePos += uiWholeFrameLen;
			return FALSE;
		}
	}

	pstMp4RInfo->uiCurFilePos += uiWholeFrameLen;

	return TRUE;

}


//now should be aac-lc
UINT32  MP4R_AudioHeadRecover(char* pacBuf, STR_MP4_RELEASE_INFO* pstMp4RInfo)
{
	UINT32  uiFrameLen = 0;
	/*STR_ADTS_HEAD*  pAdts = NULL;
	pAdts = (STR_ADTS_HEAD*)pacBuf;
	pAdts->syncword = 0xfff;
	pAdts->ID = 0;
	pAdts->layer = 0;
	pAdts->protection_absent = 1;
	pAdts->profile = 1;              //AAC-LC
	if(pstMp4RInfo->uiSamplerate == 44100)
	{
		pAdts->sampling_frequency_index = 4;           //44100
	}
	else
	{
		pAdts->sampling_frequency_index = 3;          //48000
	}
	pAdts->private_bit = 0;
	pAdts->channel_configuration = 2;          //channel
	pAdts->original = 0;
	pAdts->home = 0;
	pAdts->copyright_identification_bit = 0;
	pAdts->copyright_identification_start = 0;
	uiFrameLen = pstMp4RInfo->stAudio.uiLen + ADTS_LEN;
	pAdts->frame_length = uiFrameLen;
	pAdts->adts_buffer_fullness = 0x7ff;    //可变码率
	pAdts->number_of_raw_data_blocks_in_frame = 0;*/
	memset(pacBuf, 0 ,7);
	pacBuf[0] = 0xff;
	pacBuf[1] = 0xf1;
	//pacBuf[2] |= (1 << 6);   //AAC-LC
	pacBuf[2] |= (5 << 4);   //AAC-HE
	
	if(pstMp4RInfo->uiSamplerate == 44100)    //samplerate
	{
		pacBuf[2] |= (4 << 2);
	}
	else
	{
		pacBuf[2] |= (3 << 2);
	}
	
	pacBuf[2] |= ((2 & 4) >> 2);   //channal  2
	//pacBuf[2] = 0x4c;
	
	pacBuf[3] |= ((2 & 3) << 6); 

	uiFrameLen = pstMp4RInfo->stAudio.uiLen + ADTS_LEN;  //framelen
	pacBuf[3] |= ((uiFrameLen & 0x1800) >> 11);
	pacBuf[4] |= ((uiFrameLen & 0x1ff8) >> 3);
	pacBuf[5] |= ((uiFrameLen & 7) << 5);

	pacBuf[5] |= 0x1f;
	pacBuf[6] |= 0xfc;

	fread(&pacBuf[ADTS_LEN], sizeof(char), (uiFrameLen - ADTS_LEN), pstMp4RInfo->pFile);
	//printf("MP4R_AudioHeadRecover, read len is %x\n", (uiFrameLen - ADTS_LEN));
	pstMp4RInfo->uiCurFilePos += pstMp4RInfo->stAudio.uiLen;
	return TRUE;
}


//如果当前帧类型为AUDIO，则刷新AUDIO队列的时戳
//如果当前帧为VIDEO，则刷新VIDEO队列的时戳
UINT32  MP4R_CalcTimestamp(STR_MP4_RELEASE_INFO* pstMp4RInfo)
{
	UINT32  uiTimestamp = 0, uiCurrentType = 0, uiLoadLen = 0;

	uiCurrentType = pstMp4RInfo->stCurrent.uiType;

	if(uiCurrentType == AUDIO)
	{
		uiTimestamp = pstMp4RInfo->stAudio.uiTimeStamp + pstMp4RInfo->stAudio.uiCycle;
		pstMp4RInfo->stAudio.uiTimeStamp = uiTimestamp;
	}
	else
	{
		uiTimestamp = pstMp4RInfo->stVideo.uiTimeStamp + pstMp4RInfo->stVideo.uiCycle;
		pstMp4RInfo->stVideo.uiTimeStamp = uiTimestamp;
	}
	//pstMp4RInfo->stCurrent.uiTimeStamp = uiTimestamp;

	if(pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime < pstMp4RInfo->stStts[uiCurrentType].uiTotalPerTime)
	{
		if(pstMp4RInfo->stStts[uiCurrentType].uiLeftPerBlock >= 1)
		{
			pstMp4RInfo->stStts[uiCurrentType].uiLeftPerBlock--;
		}
		else
		{
			pstMp4RInfo->stStts[uiCurrentType].uiNum++; 
			pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime++;
			pstMp4RInfo->stStts[uiCurrentType].uiLeftPerBlock = 
				Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2]);

			if(uiCurrentType == VIDEO)
			{
				pstMp4RInfo->stVideo.uiTimeStamp += pstMp4RInfo->stVideo.uiCycleGain;
				pstMp4RInfo->stVideo.uiCycle = (Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2 + 1])
					* 1000) / pstMp4RInfo->uiTimeScale;
				pstMp4RInfo->stVideo.uiCycleGain = Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2 + 1])
					- pstMp4RInfo->stVideo.uiCycle * Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2]);
			}
			else
			{
				pstMp4RInfo->stAudio.uiTimeStamp += pstMp4RInfo->stAudio.uiCycleGain;
				pstMp4RInfo->stAudio.uiCycle = (Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2 + 1])
					* 1000) / pstMp4RInfo->uiSamplerate;
				pstMp4RInfo->stAudio.uiCycleGain = Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2 + 1])
					- pstMp4RInfo->stAudio.uiCycle * Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2]);
			}
		}
	}
	else
	{
		fseek(pstMp4RInfo->pFile, pstMp4RInfo->stStts[uiCurrentType].uiCurPosInFile, SEEK_SET);
		uiLoadLen = pstMp4RInfo->stStts[uiCurrentType].uiTotal - pstMp4RInfo->stStts[uiCurrentType].uiNum;
		if((pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime == pstMp4RInfo->stStts[uiCurrentType].uiTotalPerTime)
			&& (pstMp4RInfo->stStts[uiCurrentType].uiNum != pstMp4RInfo->stStts[uiCurrentType].uiTotal))
		{
			MP4R_LoadStts(pstMp4RInfo, uiCurrentType, uiLoadLen);
		}
		
		if(pstMp4RInfo->stStts[uiCurrentType].uiLeftPerBlock >= 1)
		{
			pstMp4RInfo->stStts[uiCurrentType].uiLeftPerBlock--;
		}
		else
		{
			pstMp4RInfo->stStts[uiCurrentType].uiNum++; 
			pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime++;
			pstMp4RInfo->stStts[uiCurrentType].uiLeftPerBlock = 
				Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2]);

			if(uiCurrentType == VIDEO)
			{
				pstMp4RInfo->stVideo.uiTimeStamp += pstMp4RInfo->stVideo.uiCycleGain;
				pstMp4RInfo->stVideo.uiCycle = (Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2 + 1])
					* 1000 )/ pstMp4RInfo->uiTimeScale;
				pstMp4RInfo->stVideo.uiCycleGain = Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2 + 1])
					- pstMp4RInfo->stVideo.uiCycle * Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2]);
			}
			else
			{
				pstMp4RInfo->stAudio.uiTimeStamp += pstMp4RInfo->stAudio.uiCycleGain;
				pstMp4RInfo->stAudio.uiCycle = (Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2 + 1])
					* 1000) / pstMp4RInfo->uiSamplerate;
				pstMp4RInfo->stAudio.uiCycleGain = Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2 + 1])
					- pstMp4RInfo->stAudio.uiCycle * Swap32(pstMp4RInfo->stStts[uiCurrentType].aucDat[pstMp4RInfo->stStts[uiCurrentType].uiNumPerTime * 2]);
			}
		}
	}
	
	return uiTimestamp;
}



UINT32  MP4R_FreshCurStream(STR_MP4_RELEASE_INFO* pstMp4RInfo)
{
	pstMp4RInfo->stVideo.uiPos = Swap32(pstMp4RInfo->stStco[VIDEO].aucDat[pstMp4RInfo->stStco[VIDEO].uiNumPerTime]);
	pstMp4RInfo->stAudio.uiPos = Swap32(pstMp4RInfo->stStco[AUDIO].aucDat[pstMp4RInfo->stStco[AUDIO].uiNumPerTime]);
	pstMp4RInfo->stVideo.uiLen= Swap32(pstMp4RInfo->stStsz[VIDEO].aucDat[pstMp4RInfo->stStsz[VIDEO].uiNumPerTime]);
	pstMp4RInfo->stAudio.uiLen = Swap32(pstMp4RInfo->stStsz[AUDIO].aucDat[pstMp4RInfo->stStsz[AUDIO].uiNumPerTime]);

	if(pstMp4RInfo->usStreamNum == 2)
	{
		//NO DATA
		if(pstMp4RInfo->stVideo.uiLen == 0 && pstMp4RInfo->stAudio.uiLen == 0)
		{
			pstMp4RInfo->stCurrent.uiLen = 0;
			printf("MP4R_FreshCurStream, data finished\n");
			return FALSE;
		}

		//ONLY AUDIO LEFT
		if(pstMp4RInfo->stVideo.uiLen == 0)
		{
			pstMp4RInfo->stCurrent.uiType = AUDIO;
			pstMp4RInfo->stCurrent.uiLen = pstMp4RInfo->stAudio.uiLen;
			pstMp4RInfo->stCurrent.uiPos = pstMp4RInfo->stAudio.uiPos;
			pstMp4RInfo->stCurrent.uiTimeStamp = pstMp4RInfo->stAudio.uiTimeStamp;
			printf("MP4R_FreshCurStream, video finished\n");
			return TRUE;
		}

		//ONLY VIDEO LEFT
		if(pstMp4RInfo->stAudio.uiLen == 0)
		{
			pstMp4RInfo->stCurrent.uiType = VIDEO;
			pstMp4RInfo->stCurrent.uiLen = pstMp4RInfo->stVideo.uiLen;
			pstMp4RInfo->stCurrent.uiPos = pstMp4RInfo->stVideo.uiPos;
			pstMp4RInfo->stCurrent.uiTimeStamp = pstMp4RInfo->stVideo.uiTimeStamp;
			printf("MP4R_FreshCurStream, audio finished\n");
			return TRUE;
		}
		
		if(pstMp4RInfo->stVideo.uiPos > pstMp4RInfo->stAudio.uiPos)
		{
			pstMp4RInfo->stCurrent.uiType = AUDIO;
			pstMp4RInfo->stCurrent.uiLen = pstMp4RInfo->stAudio.uiLen;
			pstMp4RInfo->stCurrent.uiPos = pstMp4RInfo->stAudio.uiPos;
			pstMp4RInfo->stCurrent.uiTimeStamp = pstMp4RInfo->stAudio.uiTimeStamp;
			//printf("MP4R_FreshCurStream, next is [audio] ,");
		}
		else
		{
			pstMp4RInfo->stCurrent.uiType = VIDEO;
			pstMp4RInfo->stCurrent.uiLen = pstMp4RInfo->stVideo.uiLen;
			pstMp4RInfo->stCurrent.uiPos = pstMp4RInfo->stVideo.uiPos;
			pstMp4RInfo->stCurrent.uiTimeStamp = pstMp4RInfo->stVideo.uiTimeStamp;
			//printf("MP4R_FreshCurStream, next is [video] ,");
		}
		//printf("video pos is %x, audio pos is %x\n", pstMp4RInfo->stVideo.uiPos, pstMp4RInfo->stAudio.uiPos);
	}
	else
	{
		if(pstMp4RInfo->usStreamType == AUDIO)
		{
			if(pstMp4RInfo->stAudio.uiLen == 0)
			{
				pstMp4RInfo->stCurrent.uiLen = 0;
				return FALSE;
			}
			pstMp4RInfo->stCurrent.uiType = AUDIO;
			pstMp4RInfo->stCurrent.uiLen = pstMp4RInfo->stAudio.uiLen;
			pstMp4RInfo->stCurrent.uiPos = pstMp4RInfo->stAudio.uiPos;
			pstMp4RInfo->stCurrent.uiTimeStamp = pstMp4RInfo->stAudio.uiTimeStamp;
		}
		else
		{
			if(pstMp4RInfo->stVideo.uiLen == 0)
			{
				pstMp4RInfo->stCurrent.uiLen = 0;
				return FALSE;
			}
			pstMp4RInfo->stCurrent.uiType = VIDEO;
			pstMp4RInfo->stCurrent.uiLen = pstMp4RInfo->stVideo.uiLen;
			pstMp4RInfo->stCurrent.uiPos = pstMp4RInfo->stVideo.uiPos;
			pstMp4RInfo->stCurrent.uiTimeStamp = pstMp4RInfo->stVideo.uiTimeStamp;
		}
	}

	return TRUE;
}


UINT32  MP4R_FreshCurPos(STR_MP4_RELEASE_INFO* pstMp4RInfo)
{
	UINT32 uiPos = 0;
	UINT32 uiStreamNum = pstMp4RInfo->usStreamNum;

	if(uiStreamNum == 2)                                   //音频+ 视频
	{
		if(pstMp4RInfo->stCurrent.uiType == VIDEO)               //当前帧为视频帧
		{
			if(pstMp4RInfo->stStco[VIDEO].uiTotal > pstMp4RInfo->stStco[VIDEO].uiNum + 1)     //还有未视频帧
			{
				if(pstMp4RInfo->stStco[VIDEO].uiNumPerTime + 1< pstMp4RInfo->stStco[VIDEO].uiTotalPerTime)      //不需要刷新队列
				{
					pstMp4RInfo->stVideo.uiPos = Swap32(pstMp4RInfo->stStco[VIDEO].aucDat[pstMp4RInfo->stStco[VIDEO].uiNumPerTime]);
					pstMp4RInfo->stStco[VIDEO].uiNumPerTime++;
					pstMp4RInfo->stStco[VIDEO].uiCurPosInFile++;
					pstMp4RInfo->stStco[VIDEO].uiNum++;
				}
				else
				{
					//fseek(pstMp4RInfo->pFile, pstMp4RInfo->stStco[VIDEO].uiCurPosInFile, SEEK_SET);
					UINT32 uiFilePos = ftell(pstMp4RInfo->pFile);
					MP4R_LoadStco(pstMp4RInfo, VIDEO, (pstMp4RInfo->stStco[VIDEO].uiTotal - pstMp4RInfo->stStco[VIDEO].uiNum)*sizeof(UINT32), 
						pstMp4RInfo->stStco[VIDEO].uiNum);
					pstMp4RInfo->stVideo.uiPos = Swap32(pstMp4RInfo->stStco[VIDEO].aucDat[pstMp4RInfo->stStco[VIDEO].uiNumPerTime]);
					pstMp4RInfo->stStco[VIDEO].uiNumPerTime++;
					pstMp4RInfo->stStco[VIDEO].uiCurPosInFile++;
					pstMp4RInfo->stStco[VIDEO].uiNum++;
					fseek(pstMp4RInfo->pFile, uiFilePos, SEEK_SET);
				}
			}
			else
			{
				pstMp4RInfo->stVideo.uiPos = 0xffffffff;
				if(pstMp4RInfo->stAudio.uiPos == 0xffffffff)              //文件读完
				{
					pstMp4RInfo->stCurrent.uiPos = 0xffffffff;
					return 0;
				}
				return FALSE;
			}
		}
		else
		{
			if(pstMp4RInfo->stStco[AUDIO].uiTotal > pstMp4RInfo->stStco[AUDIO].uiNum + 1)
			{
				if(pstMp4RInfo->stStco[AUDIO].uiNumPerTime + 1< pstMp4RInfo->stStco[AUDIO].uiTotalPerTime)
				{
					pstMp4RInfo->stAudio.uiPos = Swap32(pstMp4RInfo->stStco[AUDIO].aucDat[pstMp4RInfo->stStco[AUDIO].uiNumPerTime]);
					pstMp4RInfo->stStco[AUDIO].uiNumPerTime++;
					pstMp4RInfo->stStco[AUDIO].uiCurPosInFile++;
					pstMp4RInfo->stStco[AUDIO].uiNum++;
				}
				else
				{
					//fseek(pstMp4RInfo->pFile, pstMp4RInfo->stStco[AUDIO].uiCurPosInFile, SEEK_SET);
					UINT32 uiFilePos = ftell(pstMp4RInfo->pFile);
					MP4R_LoadStco(pstMp4RInfo, AUDIO, 
						(pstMp4RInfo->stStco[AUDIO].uiTotal - pstMp4RInfo->stStco[AUDIO].uiNum)*sizeof(UINT32), pstMp4RInfo->stStco[AUDIO].uiNum);
					pstMp4RInfo->stAudio.uiPos = Swap32(pstMp4RInfo->stStco[AUDIO].aucDat[pstMp4RInfo->stStco[AUDIO].uiNumPerTime]);
					pstMp4RInfo->stStco[AUDIO].uiNumPerTime++;
					pstMp4RInfo->stStco[AUDIO].uiCurPosInFile++;
					pstMp4RInfo->stStco[AUDIO].uiNum++;
					fseek(pstMp4RInfo->pFile, uiFilePos, SEEK_SET);
				}
			}
			else
			{
				pstMp4RInfo->stAudio.uiPos = 0xffffffff;
				if(pstMp4RInfo->stVideo.uiPos == 0xffffffff)         //文件读完
				{
					pstMp4RInfo->stCurrent.uiPos = 0xffffffff;
					return 0;
				}
				return FALSE;
			}
		}
	}
	else
	{
		if(pstMp4RInfo->stStco[0].uiTotal != pstMp4RInfo->stStco[0].uiNum)
		{
			if(pstMp4RInfo->stStco[0].uiNumPerTime + 1< pstMp4RInfo->stStco[0].uiTotalPerTime) 	 //不需要刷新队列
			{
				if(pstMp4RInfo->usStreamType == VIDEO)
				{
					pstMp4RInfo->stVideo.uiPos = Swap32(pstMp4RInfo->stStco[0].aucDat[pstMp4RInfo->stStco[0].uiNumPerTime]);
				}
				else
				{
					pstMp4RInfo->stAudio.uiPos = Swap32(pstMp4RInfo->stStco[0].aucDat[pstMp4RInfo->stStco[0].uiNumPerTime]);
				}
				pstMp4RInfo->stStco[0].uiNumPerTime++;
				pstMp4RInfo->stStco[0].uiCurPosInFile++;
			}
			else
			{
				//fseek(pstMp4RInfo->pFile, pstMp4RInfo->stStco[0].uiCurPosInFile, SEEK_SET);
				UINT32 uiFilePos = ftell(pstMp4RInfo->pFile);
				MP4R_LoadStco(pstMp4RInfo, 0, (pstMp4RInfo->stStco[0].uiTotal - pstMp4RInfo->stStco[0].uiNum)*sizeof(UINT32), 
					pstMp4RInfo->stStco[0].uiNum);
				if(pstMp4RInfo->usStreamType == VIDEO)
				{
					pstMp4RInfo->stVideo.uiPos = Swap32(pstMp4RInfo->stStco[0].aucDat[pstMp4RInfo->stStco[0].uiNumPerTime]);
				}
				else
				{
					pstMp4RInfo->stAudio.uiPos = Swap32(pstMp4RInfo->stStco[0].aucDat[pstMp4RInfo->stStco[0].uiNumPerTime]);
				}
				pstMp4RInfo->stStco[0].uiNumPerTime++;
				pstMp4RInfo->stStco[0].uiCurPosInFile++;
				pstMp4RInfo->stStco[0].uiNum++;
				fseek(pstMp4RInfo->pFile, uiFilePos, SEEK_SET);
			}
		}
		else
		{
			pstMp4RInfo->stCurrent.uiPos = 0xffffffff;
			return 0;
		}
	}

	//printf("MP4R_FreshCurPos, v pos is %x, a pos is %x\n", pstMp4RInfo->stVideo.uiPos, pstMp4RInfo->stAudio.uiPos);
	return TRUE;

}


UINT32  MP4R_FreshCurLen(STR_MP4_RELEASE_INFO* pstMp4RInfo)
{
	UINT32 uiPos = 0;
	UINT32 uiStreamNum = pstMp4RInfo->usStreamNum;

	if(uiStreamNum == 2)                                   //音频+ 视频
	{
		if(pstMp4RInfo->stCurrent.uiType == VIDEO)               //当前帧为视频帧
		{
			if(pstMp4RInfo->stStsz[VIDEO].uiTotal > pstMp4RInfo->stStsz[VIDEO].uiNum + 1)     //还有未视频帧
			{
				if(pstMp4RInfo->stStsz[VIDEO].uiNumPerTime + 1< pstMp4RInfo->stStsz[VIDEO].uiTotalPerTime)      //不需要刷新队列
				{
					pstMp4RInfo->stVideo.uiLen = Swap32(pstMp4RInfo->stStsz[VIDEO].aucDat[pstMp4RInfo->stStsz[VIDEO].uiNumPerTime]);
					pstMp4RInfo->stStsz[VIDEO].uiNumPerTime++;
					pstMp4RInfo->stStsz[VIDEO].uiCurPosInFile++;
					pstMp4RInfo->stStsz[VIDEO].uiNum++;
				}
				else
				{
					//fseek(pstMp4RInfo->pFile, pstMp4RInfo->stStsz[VIDEO].uiCurPosInFile, SEEK_SET);
					UINT32 uiFilePos = ftell(pstMp4RInfo->pFile);
					MP4R_LoadStsz(pstMp4RInfo, VIDEO, 
						(pstMp4RInfo->stStsz[VIDEO].uiTotal - pstMp4RInfo->stStsz[VIDEO].uiNum)*sizeof(UINT32), pstMp4RInfo->stStsz[VIDEO].uiNum);
					pstMp4RInfo->stVideo.uiLen = Swap32(pstMp4RInfo->stStsz[VIDEO].aucDat[pstMp4RInfo->stStsz[VIDEO].uiNumPerTime]);
					pstMp4RInfo->stStsz[VIDEO].uiNumPerTime++;
					pstMp4RInfo->stStsz[VIDEO].uiCurPosInFile++;
					pstMp4RInfo->stStsz[VIDEO].uiNum++;
					fseek(pstMp4RInfo->pFile, uiFilePos, SEEK_SET);
				}
			}
			else
			{
				pstMp4RInfo->stVideo.uiLen = 0;
				if(pstMp4RInfo->stAudio.uiLen == 0)              //文件读完
				{
					pstMp4RInfo->stCurrent.uiLen = 0;
					return FALSE;
				}
				return FALSE;
			}
		}
		else
		{
			if(pstMp4RInfo->stStsz[AUDIO].uiTotal > pstMp4RInfo->stStsz[AUDIO].uiNum + 1)
			{
				if(pstMp4RInfo->stStsz[AUDIO].uiNumPerTime + 1< pstMp4RInfo->stStsz[AUDIO].uiTotalPerTime)
				{
					pstMp4RInfo->stAudio.uiLen = Swap32(pstMp4RInfo->stStsz[AUDIO].aucDat[pstMp4RInfo->stStsz[AUDIO].uiNumPerTime]);
					pstMp4RInfo->stStsz[AUDIO].uiNumPerTime++;
					pstMp4RInfo->stStsz[AUDIO].uiCurPosInFile++;
					pstMp4RInfo->stStsz[AUDIO].uiNum++;
				}
				else
				{
					//fseek(pstMp4RInfo->pFile, pstMp4RInfo->stStsz[AUDIO].uiCurPosInFile, SEEK_SET);
					UINT32 uiFilePos = ftell(pstMp4RInfo->pFile);
					MP4R_LoadStsz(pstMp4RInfo, AUDIO, 
						(pstMp4RInfo->stStsz[AUDIO].uiTotal - pstMp4RInfo->stStsz[AUDIO].uiNum)*sizeof(UINT32), pstMp4RInfo->stStsz[AUDIO].uiNum);
					pstMp4RInfo->stAudio.uiLen = Swap32(pstMp4RInfo->stStsz[AUDIO].aucDat[pstMp4RInfo->stStsz[AUDIO].uiNumPerTime]);
					pstMp4RInfo->stStsz[AUDIO].uiNumPerTime++;
					pstMp4RInfo->stStsz[AUDIO].uiCurPosInFile++;
					pstMp4RInfo->stStsz[AUDIO].uiNum++;
					fseek(pstMp4RInfo->pFile, uiFilePos, SEEK_SET);
				}
			}
			else
			{
				pstMp4RInfo->stAudio.uiLen = 0;
				if(pstMp4RInfo->stVideo.uiLen == 0)         //文件读完
				{
					pstMp4RInfo->stCurrent.uiLen = 0;
					return FALSE;
				}
				return FALSE;
			}
		}
	}
	else
	{
		if(pstMp4RInfo->stStsz[0].uiTotal != pstMp4RInfo->stStsz[0].uiNum)
		{
			if(pstMp4RInfo->stStsz[0].uiNumPerTime + 1 < pstMp4RInfo->stStsz[0].uiTotalPerTime) 	 //不需要刷新队列
			{
				if(pstMp4RInfo->usStreamType == AUDIO)
				{
					pstMp4RInfo->stAudio.uiLen = Swap32(pstMp4RInfo->stStsz[0].aucDat[pstMp4RInfo->stStsz[0].uiNumPerTime]);
				}
				else
				{
					pstMp4RInfo->stVideo.uiLen = Swap32(pstMp4RInfo->stStsz[0].aucDat[pstMp4RInfo->stStsz[0].uiNumPerTime]);
				}
				pstMp4RInfo->stStsz[0].uiNumPerTime++;
				pstMp4RInfo->stStsz[0].uiCurPosInFile++;
				pstMp4RInfo->stStsz[0].uiNum++;
			}
			else
			{
				//fseek(pstMp4RInfo->pFile, pstMp4RInfo->stStsz[0].uiCurPosInFile, SEEK_SET);
				UINT32 uiFilePos = ftell(pstMp4RInfo->pFile);
				MP4R_LoadStsz(pstMp4RInfo, 0, (pstMp4RInfo->stStsz[0].uiTotal - pstMp4RInfo->stStsz[0].uiNum)*sizeof(UINT32), 
					pstMp4RInfo->stStsz[0].uiNum);
				if(pstMp4RInfo->usStreamType == AUDIO)
				{
					pstMp4RInfo->stAudio.uiLen = Swap32(pstMp4RInfo->stStsz[0].aucDat[pstMp4RInfo->stStsz[0].uiNumPerTime]);
				}
				else
				{
					pstMp4RInfo->stVideo.uiLen = Swap32(pstMp4RInfo->stStsz[0].aucDat[pstMp4RInfo->stStsz[0].uiNumPerTime]);
				}
				pstMp4RInfo->stStsz[0].uiNumPerTime++;
				pstMp4RInfo->stStsz[0].uiCurPosInFile++;
				pstMp4RInfo->stStsz[0].uiNum++;
				fseek(pstMp4RInfo->pFile, uiFilePos, SEEK_SET);
			}
		}
		else
		{
			pstMp4RInfo->stCurrent.uiLen = 0;
			return FALSE;
		}
	}

	return TRUE;

}

UINT32  MP4R_FreshStreamStruct(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiTime)
{
	UINT32 uiTimestamp = 0;
	if(pstMp4RInfo->usStreamNum == 2)
	{
		pstMp4RInfo->stVideo.uiLen = Swap32(pstMp4RInfo->stStsz[VIDEO].aucDat[0]);
		pstMp4RInfo->stVideo.uiPos = Swap32(pstMp4RInfo->stStco[VIDEO].aucDat[0]);
		pstMp4RInfo->stVideo.uiTime = uiTime;
		pstMp4RInfo->stVideo.uiCycle = (Swap32(pstMp4RInfo->stStts[VIDEO].aucDat[pstMp4RInfo->stStts[VIDEO].uiNumPerTime * 2 + 1])
			* 1000 )/ pstMp4RInfo->uiTimeScale;    //  /600
		pstMp4RInfo->stVideo.uiCycleGain = Swap32(pstMp4RInfo->stStts[VIDEO].aucDat[pstMp4RInfo->stStts[VIDEO].uiNumPerTime * 2 + 1])
			- pstMp4RInfo->stVideo.uiCycle * Swap32(pstMp4RInfo->stStts[AUDIO].aucDat[pstMp4RInfo->stStts[AUDIO].uiNumPerTime * 2]);// * 600	
		
		pstMp4RInfo->stAudio.uiLen = Swap32(pstMp4RInfo->stStsz[AUDIO].aucDat[0]);
		pstMp4RInfo->stAudio.uiPos = Swap32(pstMp4RInfo->stStco[AUDIO].aucDat[0]);
		pstMp4RInfo->stAudio.uiTime = uiTime;
		pstMp4RInfo->stAudio.uiCycle = (Swap32(pstMp4RInfo->stStts[AUDIO].aucDat[pstMp4RInfo->stStts[AUDIO].uiNumPerTime * 2 + 1])
			* 1000) / pstMp4RInfo->uiSamplerate;
		pstMp4RInfo->stAudio.uiCycleGain = Swap32(pstMp4RInfo->stStts[AUDIO].aucDat[pstMp4RInfo->stStts[AUDIO].uiNumPerTime * 2 + 1])
			- pstMp4RInfo->stAudio.uiCycle * Swap32(pstMp4RInfo->stStts[AUDIO].aucDat[pstMp4RInfo->stStts[AUDIO].uiNumPerTime * 2]);	
		
		if(pstMp4RInfo->stVideo.uiPos < pstMp4RInfo->stAudio.uiPos)
		{		
			pstMp4RInfo->stCurrent.uiType = VIDEO;
			pstMp4RInfo->stCurrent.uiPos = pstMp4RInfo->stVideo.uiPos;
			pstMp4RInfo->stCurrent.uiLen = pstMp4RInfo->stVideo.uiLen;
		}
		else
		{
			pstMp4RInfo->stCurrent.uiType = AUDIO;
			pstMp4RInfo->stCurrent.uiPos = pstMp4RInfo->stAudio.uiPos;
			pstMp4RInfo->stCurrent.uiLen = pstMp4RInfo->stAudio.uiLen;
		}	
	}
	else
	{
		if(pstMp4RInfo->usStreamType == AUDIO)
		{
			pstMp4RInfo->stAudio.uiLen = Swap32(pstMp4RInfo->stStsz[0].aucDat[0]);
			pstMp4RInfo->stAudio.uiPos = Swap32(pstMp4RInfo->stStco[0].aucDat[0]);
			pstMp4RInfo->stAudio.uiTime = Swap32(pstMp4RInfo->stStts[0].aucDat[0]);
			pstMp4RInfo->stAudio.uiCycle = (Swap32(pstMp4RInfo->stStts[0].aucDat[pstMp4RInfo->stStts[0].uiNumPerTime * 2 + 1])
				* 1000) / pstMp4RInfo->uiSamplerate;
			pstMp4RInfo->stAudio.uiCycleGain = Swap32(pstMp4RInfo->stStts[0].aucDat[pstMp4RInfo->stStts[0].uiNumPerTime * 2 + 1])
				- pstMp4RInfo->stAudio.uiCycle * Swap32(pstMp4RInfo->stStts[0].aucDat[pstMp4RInfo->stStts[0].uiNumPerTime * 2]);	

			pstMp4RInfo->stCurrent.uiType = AUDIO;
			pstMp4RInfo->stCurrent.uiPos = pstMp4RInfo->stAudio.uiPos;
			pstMp4RInfo->stCurrent.uiLen = pstMp4RInfo->stAudio.uiLen;

		}
		else
		{
			pstMp4RInfo->stVideo.uiLen = Swap32(pstMp4RInfo->stStsz[0].aucDat[0]);
			pstMp4RInfo->stVideo.uiPos = Swap32(pstMp4RInfo->stStco[0].aucDat[0]);
			pstMp4RInfo->stVideo.uiTime = Swap32(pstMp4RInfo->stStts[0].aucDat[0]);
			pstMp4RInfo->stVideo.uiCycle = (Swap32(pstMp4RInfo->stStts[0].aucDat[pstMp4RInfo->stStts[0].uiNumPerTime * 2 + 1])
				* 1000) / pstMp4RInfo->uiTimeScale;
			pstMp4RInfo->stVideo.uiCycleGain = Swap32(pstMp4RInfo->stStts[0].aucDat[pstMp4RInfo->stStts[0].uiNumPerTime * 2 + 1])
				- pstMp4RInfo->stVideo.uiCycle * Swap32(pstMp4RInfo->stStts[0].aucDat[pstMp4RInfo->stStts[0].uiNumPerTime * 2]);	

			pstMp4RInfo->stCurrent.uiType = VIDEO;
			pstMp4RInfo->stCurrent.uiPos = pstMp4RInfo->stVideo.uiPos;
			pstMp4RInfo->stCurrent.uiLen = pstMp4RInfo->stVideo.uiLen;
		}
	}

	uiTimestamp = MP4R_CalcTimestamp(pstMp4RInfo);
	pstMp4RInfo->stCurrent.uiTimeStamp = uiTimestamp;
	fseek(pstMp4RInfo->pFile, 0x30, SEEK_SET);

	return TRUE;
}


//UINT32  MP4R_ReadOneFrame(char* pacBuf, STR_MP4_RELEASE_INFO* pstMp4RInfo,
//	UINT32* puiLen, UINT32* puiTimestamp, UINT32* puiType)
DWORD  WINAPI  MP4R_ReadOneFrame(LPVOID  lparam)
{
	UINT32  uiTimestamp = 0, uiLen = 0, uiType = 0, uiRealtime = 0, uiIFrame = 0;
	STR_MP4_PARAM_PASS*  pstParam = (STR_MP4_PARAM_PASS*)lparam;
	STR_MP4_RELEASE_INFO* pstMp4RInfo = pstParam->pstMp4info;
	CALLBACK_MP4READ CallbackMp4read = pstParam->pCallback;
		
	char* pacBuf = pstParam->pBuff;
//FILE *p1 = fopen("e://mp4//release lib test//test.264", "w+b");
//FILE *p2 = fopen("e://mp4//release lib test//test.aac", "w+b");

	while(pstMp4RInfo->ucCommandState != MP4_COMMAND_STOP)
	{
		if(pstMp4RInfo->ucCommandState == MP4_COMMAND_START
			|| pstMp4RInfo->ucCommandState == MP4_COMMAND_GOON)
		{
			uiRealtime = timeGetTime() - pstMp4RInfo->uiLoadStartTime 
				- pstMp4RInfo->uiPausetimeTotal + pstMp4RInfo->iCorrectSeektime;
			
		      //read one frame start
			uiTimestamp = MP4R_CalcTimestamp(pstMp4RInfo);
			  
			if(uiRealtime < uiTimestamp - 15)
			{
				Sleep(uiTimestamp - uiRealtime -15);
			}
			
			if(!MP4R_FreshCurLen(pstMp4RInfo))
			{
				printf("MP4R_ReadOneFrame,  file over while calc len\n");
				(*CallbackMp4read)(0, 0, 0, 0, NULL, 0, 0, 0);
				break;//return FALSE;
			}	

			if(!MP4R_FreshCurPos(pstMp4RInfo))
			{
				printf("MP4R_ReadOneFrame,  file over while calc pos\n");
				(*CallbackMp4read)(0, 0, 0, 0, NULL, 0, 0, 0);
				break;//return FALSE;
			}

			if(pstMp4RInfo->stCurrent.uiType == VIDEO)
			{
				//printf("MP4R_ReadOneFrame, read video pos is %x\n", pstMp4RInfo->uiCurFilePos);
				if(!MP4R_VideoHeadRecover(pacBuf, pstMp4RInfo))
				{
					printf("MP4R_ReadOneFrame,  file over\n");
					(*CallbackMp4read)(0, 0, 0, 0, NULL, 0, 0, 0);
					break;
				}
				uiLen = pstMp4RInfo->stCurrent.uiLen;
			}
			else
			{
				//printf("MP4R_ReadOneFrame, read audio pos is %x\n", pstMp4RInfo->uiCurFilePos);
				if(!MP4R_AudioHeadRecover(pacBuf, pstMp4RInfo))
				{
					printf("MP4R_ReadOneFrame,  file over\n");
					(*CallbackMp4read)(0, 0, 0, 0, NULL, 0, 0, 0);
					break;
				}
				uiLen = pstMp4RInfo->stCurrent.uiLen + 7;
			}

			//*puiLen = pstMp4RInfo->stCurrent.uiLen;// + 7;
			uiTimestamp = uiTimestamp;//pstMp4RInfo->stCurrent.uiTimeStamp;
			uiType = pstMp4RInfo->stCurrent.uiType;

			if(!MP4R_FreshCurStream(pstMp4RInfo))
			{
				printf("MP4R_ReadOneFrame,  file over while fresh stream\n");
				break;//return FALSE;
			}
			//read one frame stop
			uiIFrame = 0;
			if(MP4R_IndentIFrame(pstMp4RInfo))
			{
				uiIFrame = 1;
			}
			
			(*CallbackMp4read)(1, uiLen, uiTimestamp, uiType, pacBuf, pstMp4RInfo->uiWidth, pstMp4RInfo->uiHeight, uiIFrame);
/*if(uiType == 0)
{
	fwrite(pacBuf, sizeof(char), uiLen, p1);
}
else
{
	fwrite(pacBuf, sizeof(char), uiLen, p2);
}*/

			pstMp4RInfo->uiFrameCnt++;
		}
		else if(pstMp4RInfo->ucCommandState == MP4_COMMAND_SEEK)
		{
			MP4R_Seek((char*)pstMp4RInfo, pstMp4RInfo->uiSetSeektime);
			pstMp4RInfo->iCorrectSeektime = pstMp4RInfo->uiSetSeektime - uiRealtime;
			pstMp4RInfo->ucCommandState = MP4_COMMAND_GOON;
		}
		else
		{
			pstMp4RInfo->uiPausetimeTotal += (timeGetTime() - pstMp4RInfo->uiPausetimeStart);
			Sleep(1);
		}
	}

	Sleep(1000);
	MP4R_LoadFinish((char*)pstMp4RInfo);
	free(pacBuf);
	
	return TRUE;
}


UINT32  MP4R_LoadHeadInfo(FILE* pFile, UINT32* puiType)
{
	char acCache[HEAD_SIZE_8BYTE];
	UINT32 uiCacheLen = 0, uiCacheType = 0;
	fread(acCache, sizeof(char), (sizeof(UINT32)*2), pFile);
	uiCacheLen = Swap32(*((UINT32*)(&acCache[0])));
	uiCacheType = *((UINT32*)(&acCache[4]));
	if(uiCacheType == FTYP || uiCacheType == FREE || uiCacheType == MDAT || uiCacheType == MOOV || uiCacheType == MVHD 
		|| uiCacheType == TRAK || uiCacheType == TKHD || uiCacheType == MDHD || uiCacheType == HDLR
		|| uiCacheType == MDIA || uiCacheType == MINF || uiCacheType == VMHD || uiCacheType == SMHD || uiCacheType == DINF 
		|| uiCacheType == DREF || uiCacheType == URL_ || uiCacheType == STBL || uiCacheType == STSD || uiCacheType == AVC1 
		|| uiCacheType == AVCC || uiCacheType == STTS || uiCacheType == STSS || uiCacheType == STSC || uiCacheType == STSZ
		|| uiCacheType == STCO)
	{	
		printf("MP4R_LoadHeadInfo, type is %x\n", uiCacheType);
		*puiType = uiCacheType;
		return uiCacheLen;
	}
	
	printf("MP4R_LoadHeadInfo,err type is %x\n", uiCacheType);
	return 0;

}

UINT32  MP4R_LoadTkhd(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiCacheLen)
{
	char acCache[1024];
	UINT32 uiCacheType = 0, uiRet = 0, uiWidth = 0, uiHeight = 0;
	fread(&acCache[HEAD_SIZE_8BYTE], sizeof(char), (uiCacheLen - HEAD_SIZE_8BYTE), pstMp4RInfo->pFile);
	pstMp4RInfo->uiCurFilePos += (uiCacheLen - HEAD_SIZE_8BYTE);
	STR_TKHD* pTkhd = (STR_TKHD*)acCache;

	uiWidth = (Swap32(pTkhd->uiWidth) >> 16);
	uiHeight = (Swap32(pTkhd->uiHeight) >> 16);
	if(uiWidth != 0 && uiWidth <= 1920
		&& uiHeight != 0 && uiHeight <= 1088)
	{
		pstMp4RInfo->uiWidth = uiWidth;
		pstMp4RInfo->uiHeight = uiHeight;
		if(pstMp4RInfo->usStreamNum == 0)
		{				
			pstMp4RInfo->usStreamType = VIDEO;
		}
		else
		{
			pstMp4RInfo->usStreamType = AVMIX;
		}
		uiRet = VIDEO;
	}
	else
	{
		if(pstMp4RInfo->usStreamNum == 0)
		{
			pstMp4RInfo->usStreamType = AUDIO;
		}
		else
		{
			pstMp4RInfo->usStreamType = AVMIX;
		}
		uiRet = AUDIO;
	}
	pstMp4RInfo->usStreamNum++;
	return uiRet;

}

void  MP4R_LoadMvhd(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiCacheLen)
{
	char acCache[1024];
	fread(&acCache[HEAD_SIZE_8BYTE], sizeof(char), (uiCacheLen - HEAD_SIZE_8BYTE), pstMp4RInfo->pFile);
	pstMp4RInfo->uiCurFilePos += (uiCacheLen - HEAD_SIZE_8BYTE);
	STR_MVHD * pMvhd = (STR_MVHD *)acCache;
	pstMp4RInfo->uiDura = Swap32(pMvhd->uiDura);

	return;
}

void  MP4R_LoadMdhd(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiCacheLen)
{
	char acCache[1024];
	fread(&acCache[HEAD_SIZE_8BYTE], sizeof(char), (uiCacheLen - HEAD_SIZE_8BYTE), pstMp4RInfo->pFile);
	pstMp4RInfo->uiCurFilePos += (uiCacheLen - HEAD_SIZE_8BYTE);
	STR_MDHD * pMdhd = (STR_MDHD *)acCache;
	
	if(pMdhd->uiTimeScale == Swap32(SCALE_ONE_MSEC_44K1) || pMdhd->uiTimeScale == Swap32(SCALE_ONE_MSEC_48K))
	{
		pstMp4RInfo->uiSamplerate = Swap32(pMdhd->uiTimeScale);
	}
	else
	{
		pstMp4RInfo->uiTimeScale = Swap32(pMdhd->uiTimeScale);
	}

	return;
}

void MP4R_LoadStts(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiType, UINT32 uiLoadLen)
{
	if(uiLoadLen <= MP4_READ_LEN_PER_TIME_BYTE)
	{
		fread(&pstMp4RInfo->stStts[uiType].aucDat, sizeof(char), uiLoadLen, pstMp4RInfo->pFile);
		pstMp4RInfo->stStts[uiType].uiTotalPerTime = uiLoadLen/8;
		pstMp4RInfo->stStts[uiType].uiNumPerTime = 0;
	}
	else
	{
		fread(&pstMp4RInfo->stStts[uiType].aucDat, sizeof(char), MP4_READ_LEN_PER_TIME_BYTE, pstMp4RInfo->pFile);
		pstMp4RInfo->stStts[uiType].uiTotalPerTime = MP4_READ_LEN_PER_TIME_BYTE/8;
		pstMp4RInfo->stStts[uiType].uiNumPerTime = 0;
		fseek(pstMp4RInfo->pFile, (uiLoadLen - MP4_READ_LEN_PER_TIME_BYTE), SEEK_CUR);
	}
	pstMp4RInfo->uiCurFilePos += uiLoadLen;
	pstMp4RInfo->stStts[uiType].uiLeftPerBlock = 
		Swap32(pstMp4RInfo->stStts[uiType].aucDat[pstMp4RInfo->stStts[uiType].uiNumPerTime * 2]);

	return;

}


void MP4R_LoadStss(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiType, UINT32  uiLoadLen, UINT32 uiOffset)
{
	fseek(pstMp4RInfo->pFile, pstMp4RInfo->stStss[uiType].uiStartPosInFile + uiOffset*sizeof(UINT32), SEEK_SET);
				//pstMp4RInfo->stStss[uiType].aucDat[uiOffset]), SEEK_SET);

	if(uiLoadLen <= MP4_READ_LEN_PER_TIME_BYTE)
	{
		fread(&pstMp4RInfo->stStss[uiType].aucDat[0], sizeof(char), uiLoadLen, pstMp4RInfo->pFile);
		pstMp4RInfo->stStss[uiType].uiTotalPerTime = uiLoadLen/4;
		pstMp4RInfo->stStss[uiType].uiNumPerTime = 0;
	}
	else
	{
		fread(&pstMp4RInfo->stStss[uiType].aucDat[0], sizeof(char), MP4_READ_LEN_PER_TIME_BYTE, pstMp4RInfo->pFile);
		pstMp4RInfo->stStss[uiType].uiTotalPerTime = MP4_READ_LEN_PER_TIME_UINT32;
		pstMp4RInfo->stStss[uiType].uiNumPerTime = 0;
		//fseek(pstMp4RInfo->pFile, (uiLoadLen - MP4_READ_LEN_PER_TIME_BYTE), SEEK_CUR);
	}
	pstMp4RInfo->uiCurFilePos += uiLoadLen;

	return;

}


void MP4R_LoadStsz(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiType, UINT32 uiLoadLen, UINT32 uiOffset)
{
	fseek(pstMp4RInfo->pFile, pstMp4RInfo->stStsz[uiType].uiStartPosInFile + uiOffset*sizeof(UINT32), SEEK_SET);
		//pstMp4RInfo->stStsz[uiType].aucDat[uiOffset], SEEK_SET);

	if(uiLoadLen <= MP4_READ_LEN_PER_TIME_BYTE)
	{
		fread(&pstMp4RInfo->stStsz[uiType].aucDat[0], sizeof(char), uiLoadLen, pstMp4RInfo->pFile);
		pstMp4RInfo->stStsz[uiType].uiTotalPerTime = uiLoadLen/4;
		pstMp4RInfo->stStsz[uiType].uiNumPerTime = 0;
	}
	else
	{
		fread(&pstMp4RInfo->stStsz[uiType].aucDat[0], sizeof(char), MP4_READ_LEN_PER_TIME_BYTE, pstMp4RInfo->pFile);
		pstMp4RInfo->stStsz[uiType].uiTotalPerTime = MP4_READ_LEN_PER_TIME_UINT32;
		pstMp4RInfo->stStsz[uiType].uiNumPerTime = 0;
		//fseek(pstMp4RInfo->pFile, (uiLoadLen - MP4_READ_LEN_PER_TIME_BYTE), SEEK_CUR);
	}
	pstMp4RInfo->uiCurFilePos += uiLoadLen;

	return;

}

void MP4R_LoadStco(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32 uiType, UINT32 uiLoadLen, UINT32 uiOffset)
{
	fseek(pstMp4RInfo->pFile, (pstMp4RInfo->stStco[uiType].uiStartPosInFile + uiOffset*sizeof(UINT32)), SEEK_SET);

	if(uiLoadLen <= MP4_READ_LEN_PER_TIME_BYTE)
	{
		fread(&pstMp4RInfo->stStco[uiType].aucDat[0], sizeof(char), uiLoadLen, pstMp4RInfo->pFile);
		pstMp4RInfo->stStco[uiType].uiTotalPerTime = uiLoadLen/4;
		pstMp4RInfo->stStco[uiType].uiNumPerTime = 0;
	}
	else
	{
		fread(&pstMp4RInfo->stStco[uiType].aucDat[0], sizeof(char), MP4_READ_LEN_PER_TIME_BYTE, pstMp4RInfo->pFile);
		pstMp4RInfo->stStco[uiType].uiTotalPerTime = MP4_READ_LEN_PER_TIME_UINT32;
		pstMp4RInfo->stStco[uiType].uiNumPerTime = 0;
		//fseek(pstMp4RInfo->pFile, (uiLoadLen - MP4_READ_LEN_PER_TIME_BYTE), SEEK_CUR);
	}
	pstMp4RInfo->uiCurFilePos += uiLoadLen;

	return;

}

UINT32  MP4R_LoadCertainInfo(STR_MP4_RELEASE_INFO* pstMp4RInfo, UINT32* uiStreamType)
{
	char acCache[1024];
	UINT32 uiCacheLen = 0, uiCacheType = 0, uiLoadLen = 0,  uiCount = 0;
	uiCacheLen = MP4R_LoadHeadInfo(pstMp4RInfo->pFile, &uiCacheType);
	if( uiCacheLen == 0 || uiCacheLen < HEAD_SIZE_8BYTE)
	{
		printf("MP4R_LoadCertainInfo, head len is %d\n", uiCacheLen);
		return FALSE;
	}
	pstMp4RInfo->uiCurFilePos += HEAD_SIZE_8BYTE;
	
	if(uiCacheType == FTYP || uiCacheType == FREE || uiCacheType == MDAT ||uiCacheType == STSD
		|| uiCacheType == HDLR|| uiCacheType == VMHD ||uiCacheType == STSC
		|| uiCacheType == SMHD || uiCacheType == DINF || uiCacheType == DREF 
		|| uiCacheType == URL_ || uiCacheType == AVC1 || uiCacheType == AVCC)
	{
		fseek(pstMp4RInfo->pFile, (uiCacheLen - HEAD_SIZE_8BYTE), SEEK_CUR);
		pstMp4RInfo->uiCurFilePos += (uiCacheLen - HEAD_SIZE_8BYTE);
		if(pstMp4RInfo->uiCurFilePos == pstMp4RInfo->uiFilelen)
		{
			printf("MP4R_LoadCertainInfo, end type is %x\n", uiCacheType);
			MP4R_FreshStreamStruct(pstMp4RInfo, 0);
			return TRUE;
		}
		return uiCacheType;
	}

	if(uiCacheType == MDHD)
	{
		MP4R_LoadMdhd(pstMp4RInfo, uiCacheLen);
		//printf("MP4R_LoadCertainInfo, case mdhd\n");
		return uiCacheType;
	}

	if(uiCacheType == MOOV || uiCacheType == TRAK || uiCacheType == MDIA ||uiCacheType == MINF
		||uiCacheType == STBL)
	{
		//fseek(pstMp4RInfo->pFile, HEAD_SIZE_8BYTE, SEEK_CUR);
		//pstMp4RInfo->uiCurFilePos += HEAD_SIZE_8BYTE;
		return uiCacheType;
	}

	if(uiCacheType == MVHD )
	{
		MP4R_LoadMvhd(pstMp4RInfo, uiCacheLen);
		return uiCacheType;
	}
	
	if(uiCacheType == TKHD)
	{
		*uiStreamType = MP4R_LoadTkhd(pstMp4RInfo, uiCacheLen);
		return uiCacheType;
	}

	if(uiCacheType == STTS)
	{		
		fread(&acCache[HEAD_SIZE_8BYTE], sizeof(char), (sizeof(STR_STTS) - HEAD_SIZE_8BYTE), pstMp4RInfo->pFile);
		pstMp4RInfo->uiCurFilePos += (sizeof(STR_STTS) - HEAD_SIZE_8BYTE);
		pstMp4RInfo->stStts[*uiStreamType].uiStartPosInFile = pstMp4RInfo->uiCurFilePos;
		pstMp4RInfo->stStts[*uiStreamType].uiCurPosInFile = pstMp4RInfo->uiCurFilePos;
		STR_STTS* pStts = (STR_STTS *)acCache;
		uiCount = Swap32(pStts->uiEntryCnt);
		uiLoadLen = uiCount * sizeof(UINT32) * 2;
		pstMp4RInfo->stStts[*uiStreamType].uiTotal = uiCount;
		pstMp4RInfo->stStts[*uiStreamType].uiNum = 0;
		MP4R_LoadStts(pstMp4RInfo, *uiStreamType, uiLoadLen);
		return uiCacheType;
	}

	if(uiCacheType == STSS)
	{
		fread(&acCache[HEAD_SIZE_8BYTE], sizeof(char), (sizeof(STR_STSS) - HEAD_SIZE_8BYTE), pstMp4RInfo->pFile);
		pstMp4RInfo->uiCurFilePos += (sizeof(STR_STSS) - HEAD_SIZE_8BYTE);
		pstMp4RInfo->stStss[*uiStreamType].uiStartPosInFile = pstMp4RInfo->uiCurFilePos;
		pstMp4RInfo->stStss[*uiStreamType].uiCurPosInFile = pstMp4RInfo->uiCurFilePos;
		STR_STSS* pStss = (STR_STSS *)acCache;
		uiCount = Swap32(pStss->uiEntryCnt);
		uiLoadLen = uiCount * sizeof(UINT32);
		pstMp4RInfo->stStss[*uiStreamType].uiTotal = uiCount;
		pstMp4RInfo->stStss[*uiStreamType].uiNum = 0;
		MP4R_LoadStss(pstMp4RInfo, *uiStreamType, uiLoadLen, 0);
		if(uiLoadLen > MP4_READ_LEN_PER_TIME_BYTE)
		{
			fseek(pstMp4RInfo->pFile, (uiLoadLen - MP4_READ_LEN_PER_TIME_BYTE), SEEK_CUR);
		}
		return uiCacheType;
	}

	if(uiCacheType == STSZ)
	{
		fread(&acCache[HEAD_SIZE_8BYTE], sizeof(char), (sizeof(STR_STSZ) - HEAD_SIZE_8BYTE), pstMp4RInfo->pFile);
		pstMp4RInfo->uiCurFilePos += (sizeof(STR_STSZ) - HEAD_SIZE_8BYTE);
		pstMp4RInfo->stStsz[*uiStreamType].uiStartPosInFile = pstMp4RInfo->uiCurFilePos;
		pstMp4RInfo->stStsz[*uiStreamType].uiCurPosInFile = pstMp4RInfo->uiCurFilePos;
		STR_STSZ* pStsz = (STR_STSZ*)acCache;
		uiCount = Swap32(pStsz->uiSampleCnt);
		uiLoadLen = uiCount * sizeof(UINT32);
		pstMp4RInfo->stStsz[*uiStreamType].uiTotal = uiCount;
		pstMp4RInfo->stStsz[*uiStreamType].uiNum = 0;
		MP4R_LoadStsz(pstMp4RInfo, *uiStreamType, uiLoadLen, 0);
		if(uiLoadLen > MP4_READ_LEN_PER_TIME_BYTE)
		{
			fseek(pstMp4RInfo->pFile, (uiLoadLen - MP4_READ_LEN_PER_TIME_BYTE), SEEK_CUR);
		}	
		return uiCacheType;
	}

	if(uiCacheType == STCO)
	{
		fread(&acCache[HEAD_SIZE_8BYTE], sizeof(char), (sizeof(STR_STCO) - HEAD_SIZE_8BYTE), pstMp4RInfo->pFile);
		pstMp4RInfo->uiCurFilePos += (sizeof(STR_STCO) - HEAD_SIZE_8BYTE);
		pstMp4RInfo->stStco[*uiStreamType].uiStartPosInFile = pstMp4RInfo->uiCurFilePos;
		pstMp4RInfo->stStco[*uiStreamType].uiCurPosInFile = pstMp4RInfo->uiCurFilePos;
		STR_STCO* pStco = (STR_STCO*)acCache;
		uiCount = Swap32(pStco->uiEntryCnt);
		uiLoadLen = uiCount * sizeof(UINT32);
		pstMp4RInfo->stStco[*uiStreamType].uiTotal = uiCount;
		pstMp4RInfo->stStco[*uiStreamType].uiNum = 0;
		MP4R_LoadStco(pstMp4RInfo, *uiStreamType, uiLoadLen, 0);
		if(uiLoadLen > MP4_READ_LEN_PER_TIME_BYTE)
		{
			fseek(pstMp4RInfo->pFile, (uiLoadLen - MP4_READ_LEN_PER_TIME_BYTE), SEEK_CUR);
		}	
		if(pstMp4RInfo->uiCurFilePos == pstMp4RInfo->uiFilelen)
		{
			printf("MP4R_LoadCertainInfo, end type is %x\n", uiCacheType);
			MP4R_FreshStreamStruct(pstMp4RInfo, 0);
			pstMp4RInfo->uiCurFilePos = 0x30;
			return TRUE;
		}
		return uiCacheType;
	}

	printf("MP4R_LoadCertainInfo, head type is %x\n", uiCacheType);
	
	return FALSE;

}


char* MP4R_LoadStart(char* acFile, CALLBACK_MP4READ CallbackMp4read)
{
	UINT32 uiCacheLen = 0, uiCacheType = 0, uiRet = 2, uiStreamType = 0;

	STR_MP4_RELEASE_INFO* pstMp4RInfo = (STR_MP4_RELEASE_INFO*)malloc(sizeof(STR_MP4_RELEASE_INFO) + sizeof(UINT32));
	char*     pBuff = (char*)malloc(2000000);
	memset((char*)pstMp4RInfo, 0, (sizeof(STR_MP4_RELEASE_INFO) + sizeof(UINT32)));

	pstMp4RInfo->pFile = fopen(acFile, "r+b");
	if(!pstMp4RInfo->pFile)
	{
		printf("MP4R_LoadStart, open %s failed\n", acFile);
		free(pstMp4RInfo);
		return NULL;
	}

	fseek(pstMp4RInfo->pFile, 0, SEEK_END);
	pstMp4RInfo->uiFilelen = ftell(pstMp4RInfo->pFile);  

	fseek(pstMp4RInfo->pFile, 0, SEEK_SET);
	
	while(uiRet > 1)
	{
		uiRet = MP4R_LoadCertainInfo(pstMp4RInfo, &uiStreamType);
	}
	
	if(uiRet == 0)
	{
		free(pstMp4RInfo);
		return NULL;
	}

	pstMp4RInfo->uiLoadStartTime = timeGetTime();

	DWORD dwRdmp4;
	HANDLE handleRdmp4;
	STR_MP4_PARAM_PASS* stParam = (STR_MP4_PARAM_PASS*)malloc(sizeof(STR_MP4_PARAM_PASS) + sizeof(UINT32));
	stParam->pstMp4info = pstMp4RInfo;
	stParam->pBuff = pBuff;
	stParam->pCallback = CallbackMp4read;
	pstMp4RInfo->ucCommandState = MP4_COMMAND_START;
	handleRdmp4 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)MP4R_ReadOneFrame, stParam, 0, &dwRdmp4);
	if(handleRdmp4)
		{CloseHandle(handleRdmp4);}

	return (char*)pstMp4RInfo;
	
}




void MP4R_LoadFinish(char* acMp4RInfo)
{
	STR_MP4_RELEASE_INFO* pstMp4RInfo = NULL;
	pstMp4RInfo = (STR_MP4_RELEASE_INFO*)acMp4RInfo;

	fclose(pstMp4RInfo->pFile);
	free(pstMp4RInfo);

	return;
}

UINT32 MP4R_Seek_SearchIFrameNextAudio(STR_MP4_RELEASE_INFO *pstMp4RInfo, UINT32 uiIFramePos, UINT32 uiStreamNo)
{
	UINT32 i = 0, uiCount = 0;
	char tmp[4];
	fseek(pstMp4RInfo->pFile, pstMp4RInfo->stStco[uiStreamNo].uiStartPosInFile, SEEK_SET);
	for(i = 0; i < pstMp4RInfo->stStco[uiStreamNo].uiTotal; i++)
	{
		fread(tmp, sizeof(UINT32), 1 , pstMp4RInfo->pFile);
		if(Swap32(*((UINT32*)tmp)) >= uiIFramePos)
		{
			//MP4R_LoadStco(pstMp4RInfo, 0, pstMp4RInfo->stStco[uiStreamNo].uiTotal - uiCount, uiCount);
			return uiCount;
		}
		uiCount++;
	}
	
	return 0xff000000;

}

UINT32 MP4R_Seek_SearchVideoFrameDura(STR_MP4_RELEASE_INFO *pstMp4RInfo, UINT32 uiIFramePos, UINT32 uiStreamNo)
{
	UINT32 i = 0, j = 0, uiCount = 0, uiDuration = 0;
	char tmp[4];
	
	fseek(pstMp4RInfo->pFile, pstMp4RInfo->stStco[uiStreamNo].uiStartPosInFile, SEEK_SET);
	for(i = 0; i < pstMp4RInfo->stStco[uiStreamNo].uiTotal; i++)
	{
		for(j = 0; j < pstMp4RInfo->stStts[0].aucDat[i * 2]; j++)
		{
			fread(tmp, sizeof(UINT32), 1 , pstMp4RInfo->pFile);
			uiDuration += Swap32(pstMp4RInfo->stStts[0].aucDat[i * 2] * (pstMp4RInfo->stStts[0].aucDat[i * 2 + 1] / FRAME_PERIOD));
			if(Swap32(*((UINT32*)tmp)) >= uiIFramePos)
			{
				return uiCount;
			}
			uiCount++;
		}

	}
	
	return 0xff000000;

}

void MP4R_Seek(char* acMp4RInfo, UINT32 uiTime)
{
	UINT32 i = 0, j = 0, uiCount = 0, uiDuration= 0, uiEntryCnt = 0, uiKeyFrameCycle = 0, uiAimKeyFrame = 0;
	UINT32 uiAimFrame = 0, uiTotalFrame = 0, uiBreak = 0;
	STR_MP4_RELEASE_INFO* pstMp4RInfo = NULL;
	pstMp4RInfo = (STR_MP4_RELEASE_INFO*)acMp4RInfo;
	if(uiTime > pstMp4RInfo->uiDura)
	{
		printf("MP4R_Seek, duration is %x, seektime is %x\n", pstMp4RInfo->uiDura, uiTime);
		return;
	}

	uiKeyFrameCycle = pstMp4RInfo->uiDura / pstMp4RInfo->stStss[0].uiTotal;
	uiAimKeyFrame = uiTime / uiKeyFrameCycle;	
	if(uiAimKeyFrame > pstMp4RInfo->stStss[0].uiTotal)
	{
		printf("MP4R_Seek, aim key-frame is %x, total key-frame is %x\n", uiAimKeyFrame, pstMp4RInfo->stStss[0].uiTotal);
		return;
	}
	pstMp4RInfo->stStss[0].uiTotal = pstMp4RInfo->stStss[0].uiTotal - uiAimKeyFrame;
	MP4R_LoadStss(pstMp4RInfo, 0, pstMp4RInfo->stStss[0].uiTotal * sizeof(UINT32), uiAimKeyFrame);
	uiAimFrame = Swap32(pstMp4RInfo->stStss[0].aucDat[0]) - 1;
	pstMp4RInfo->uiFrameCnt = uiAimFrame;               //一个从0开始  一个从1开始
	//uiTotalFrame = pstMp4RInfo->stStsz[0].uiTotal;
	//uiCount = uiAimFrame;
	UINT32 uiTimeSlice = pstMp4RInfo->uiSamplerate;
	if(pstMp4RInfo->usStreamNum == 2)
	{
		for(i = 0; i < pstMp4RInfo->stStts[0].uiTotal; i++)
		{
			UINT32 uiCycle = Swap32(pstMp4RInfo->stStts[0].aucDat[i * 2 + 1]);
			for(j = 0; j < pstMp4RInfo->stStts[0].aucDat[i * 2]; j++)
			{
				//uiDuration += uiCycle / FRAME_PERIOD;
				uiDuration += (Swap32(pstMp4RInfo->stStts[0].aucDat[i * 2]) * uiCycle) * 1000 / uiTimeSlice;

				pstMp4RInfo->stStts[0].uiNum = uiCount;
				pstMp4RInfo->stStsz[0].uiNum = uiCount;
				pstMp4RInfo->stStco[0].uiNum = uiCount;
				if(uiCount == uiAimFrame)
				{
					MP4R_LoadStco(pstMp4RInfo, 0, (pstMp4RInfo->stStco[0].uiTotal - uiCount)*sizeof(UINT32), uiCount);
					MP4R_LoadStsz(pstMp4RInfo, 0, (pstMp4RInfo->stStsz[0].uiTotal - uiCount)*sizeof(UINT32), uiCount);
					pstMp4RInfo->stStts[0].uiNumPerTime = i;
					pstMp4RInfo->stStts[0].uiLeftPerBlock = Swap32(pstMp4RInfo->stStts[0].aucDat[i * 2]) - j;

					//刷新VIDEO/AUDIO/CUR
					uiBreak = 1;
					break;
				}
				uiCount++;
			}
			if(uiBreak)
				{break;}
		}

		UINT32  uiAudioAimFrameCount = 0, uiAudioCount = 0;
		uiBreak = 0;
		uiAudioAimFrameCount = MP4R_Seek_SearchIFrameNextAudio(pstMp4RInfo, Swap32(pstMp4RInfo->stStco[0].aucDat[0 ]), 1);
		if(uiAudioAimFrameCount >= 0xff000000)
		{
			printf("MP4R_Seek, can not find audio frame count, input video pos is %x\n", pstMp4RInfo->stStco[0].aucDat[0 ]);
			return;
		}

		for(i = 0; i < pstMp4RInfo->stStts[1].uiTotal; i++)
		{
			for(j = 0; j < Swap32(pstMp4RInfo->stStts[1].aucDat[i * 2]); j++)
			{
				pstMp4RInfo->stStts[1].uiNum = uiAudioCount;
				pstMp4RInfo->stStsz[1].uiNum = uiAudioCount;
				pstMp4RInfo->stStco[1].uiNum = uiAudioCount;
				if(uiAudioAimFrameCount == uiAudioCount)
				{
					MP4R_LoadStco(pstMp4RInfo, 1, (pstMp4RInfo->stStco[1].uiTotal - uiAudioAimFrameCount)*sizeof(UINT32), uiAudioAimFrameCount);
					MP4R_LoadStsz(pstMp4RInfo, 1, (pstMp4RInfo->stStsz[1].uiTotal - uiAudioAimFrameCount)*sizeof(UINT32), uiAudioAimFrameCount);
					pstMp4RInfo->stStts[1].uiNumPerTime = i;
					pstMp4RInfo->stStts[1].uiLeftPerBlock = Swap32(pstMp4RInfo->stStts[1].aucDat[i * 2]) - j;
					uiBreak = 1;
					break;
				}
				uiAudioCount++;
			}
			if(uiBreak)
				{break;}
		}

		MP4R_FreshStreamStruct(pstMp4RInfo, uiDuration);
		pstMp4RInfo->stAudio.uiTimeStamp = uiDuration;//Swap32(pstMp4RInfo->stStco[AUDIO].aucDat[0]);
		pstMp4RInfo->stVideo.uiTimeStamp = uiDuration;//Swap32(pstMp4RInfo->stStco[VIDEO].aucDat[0]);
		pstMp4RInfo->stCurrent.uiType = VIDEO;

	}
	else
	{
		for(i = 0; i < pstMp4RInfo->stStts[0].uiTotal; i++)
		{
			//uiDuration += Swap32(pstMp4RInfo->stStts[0].aucDat[i * 2] * (pstMp4RInfo->stStts[0].aucDat[i * 2 + 1] / FRAME_PERIOD));
			uiDuration += (Swap32(pstMp4RInfo->stStts[0].aucDat[i * 2]) 
					* Swap32(pstMp4RInfo->stStts[0].aucDat[i * 2 + 1])) * 1000 / uiTimeSlice;
			pstMp4RInfo->stStts[0].uiNum = uiCount;
			pstMp4RInfo->stStsz[0].uiNum = uiCount;
			pstMp4RInfo->stStco[0].uiNum = uiCount;
			if(uiDuration >= uiTime)
			{
				MP4R_LoadStco(pstMp4RInfo, 0, (pstMp4RInfo->stStts[0].uiTotal - uiCount), uiCount);
				MP4R_LoadStsz(pstMp4RInfo, 0, (pstMp4RInfo->stStts[0].uiTotal - uiCount), uiCount);
			}
			uiCount++;
			printf("MP4R_Seek, can not find time point, duration is %x, seektime is %x\n", uiDuration, uiTime);
			return;
		}
	}

	fseek(pstMp4RInfo->pFile, Swap32(pstMp4RInfo->stStco[0].aucDat[0]), SEEK_SET);
	pstMp4RInfo->uiCurFilePos = Swap32(pstMp4RInfo->stStco[0].aucDat[0]);
	pstMp4RInfo->uiSetSeektime = uiDuration;
	return;

}

UINT32 MP4R_IndentIFrame(STR_MP4_RELEASE_INFO* pstMp4RInfo)
{
	UINT32 uiTimeStamp = 0, i = 0;
	for(i = 0; i < MP4_READ_LEN_PER_TIME_UINT32; i++)
	{
		if(pstMp4RInfo->uiFrameCnt == Swap32(pstMp4RInfo->stStss[0].aucDat[i]) - 1)
		{
			return TRUE;
		}
		if(pstMp4RInfo->uiFrameCnt < Swap32(pstMp4RInfo->stStss[0].aucDat[i]) - 1)
		{
			break;
		}
	}
	return FALSE;
}


//command:0  stop     /   1  start      /2   pause       /3   seek
UINT32 MP4R_FuncInterface(UINT8 ucCommand, UINT32 uiTime, char* acFile, UINT32* stMp4info, CALLBACK_MP4READ CallbackMp4read)
{
	char*  pMp4info = NULL;
	if(ucCommand == MP4_COMMAND_START)
	{
		pMp4info = MP4R_LoadStart(acFile, CallbackMp4read);
		(*stMp4info) = (UINT32)pMp4info;
	}
	else if(ucCommand == MP4_COMMAND_PAUSE)
	{
		((STR_MP4_RELEASE_INFO*) (*stMp4info))->uiPausetimeStart = timeGetTime();
		((STR_MP4_RELEASE_INFO*) (*stMp4info))->ucCommandState = MP4_COMMAND_PAUSE;
	}
	else if(ucCommand == MP4_COMMAND_SEEK)
	{
		((STR_MP4_RELEASE_INFO*) (*stMp4info))->uiSetSeektime = uiTime;
		((STR_MP4_RELEASE_INFO*) (*stMp4info))->ucCommandState = MP4_COMMAND_SEEK;
	}
	else if(ucCommand == MP4_COMMAND_GOON)
	{
		((STR_MP4_RELEASE_INFO*) *stMp4info)->ucCommandState = MP4_COMMAND_GOON;
	}
	else
	{
		((STR_MP4_RELEASE_INFO*) (*stMp4info))->ucCommandState = MP4_COMMAND_STOP;
	}

	return ((STR_MP4_RELEASE_INFO*) (*stMp4info))->uiDura;
}
