/**********************************************************
file name :common.cpp
note       :this file include the basic low-level function
auther    :kyo      
date       :2011.1
**********************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "time.h"
#include "mp4_common.h"

#include <windows.h> 
#include <conio.h> 
//#include <Record.h>
#include<psapi.h> //for mem info
//#include "ipsan.h"
//#include "telnet.h"
#pragma comment(lib,"winmm.lib")
#pragma comment(lib, "Psapi.lib")
//#pragma comment(lib, "Record.lib")


//extern STR_SYS_INFO g_strSysInfo;
//extern CRITICAL_SECTION g_protect;
//extern char* g_RecvDatBuf[MAX_BROAD_NUM][TCP_ARRAY];
//extern STR_TNRC_PARAM g_stTnrcInfo;
//extern STR_MP4INFO g_stSys;
#define __DEBUG

//mp4 add
UINT32 Swap32(UINT32 uiNum)
{
	UINT32  uiRet = 0;
	uiRet = (((uiNum & 0xff000000) >> 24) | ((uiNum & 0xff0000) >> 8) 
		| ((uiNum & 0xff00) << 8) | ((uiNum & 0xff) << 24));
	return uiRet;
}


UINT16 Swap16(UINT16 usNum)
{
	UINT16  usRet = 0;
	UINT16  usRet1 = 0, usRet2 = 0;
	usRet1 = (usNum & 0xff00);
	usRet2 = (usNum & 0x00ff);
	usRet =((usRet1 >> 8) | (usRet2 << 8));
	return usRet;
}


UINT64 Swap64(UINT64 ulNum)
{
	UINT64  ulRet = 0;
	UINT32  uiRet1 = 0, uiRet2 = 0;
	uiRet1 = ((ulNum & 0xffffffff00000000) >> 32);
	uiRet1 = Swap32(uiRet1);
	uiRet2 = (ulNum & 0xffffffff);
	uiRet2 = Swap32(uiRet2);
	
	ulRet = (uiRet1 | (uiRet2 << 32));
	
	return ulRet;
}

UINT16 NumberToAscii(UINT32 uiValue)
{
	UINT8 ucTen = 0, ucSin = 0;
	UINT16 usRet = 0;
	if(uiValue > 99)
	{
		return 0;
	}
	ucTen = uiValue/10;
	ucSin = uiValue%10;

	usRet = ((ucTen + 0x30) << 8) + (ucSin + 0x30);
	return usRet;

}

INT8 CreatTmpFile(char* pacDir, UINT32 uiType, UINT32 uiId, STR_MP4INFO* stSys)
{
	char  acDir[MAX_DIRLEN], cAdd = 0;
	UINT32 uiLen = 0;
	UINT16 usNumber;
	uiLen = strlen(pacDir);
	if(uiLen >= MAX_INDIR)
	{
		printf("CreatTmpFile, input dir len = %d\r\n", uiLen);
		return FALSE;
	}
	if(uiId > 99)
		{return FALSE;}
	
	memset(acDir, 0, MAX_DIRLEN);	
	strcpy(acDir, pacDir);
	if(uiId> 0)
	{
		usNumber = NumberToAscii(uiId);
		usNumber = Swap16(usNumber);
		memcpy(&acDir[uiLen], (char*)&usNumber, 2);
		uiLen += 2;
	}

	if(uiType == STSS)
	{
		strcpy(&acDir[uiLen], DIR_STSS);
		stSys->astStream[uiId].pStss = fopen(acDir, "w+b");
		if(stSys->astStream[uiId].pStss == NULL)
		{
			printf("CreatTmpFile, STSS creat failed\r\n");
			return FALSE;
		}
	}
	else if(uiType == STSC)
	{
		strcpy(&acDir[uiLen], DIR_STSC);
		stSys->astStream[uiId].pStsc = fopen(acDir, "w+b");
		if(stSys->astStream[uiId].pStsc == NULL)
		{
			printf("CreatTmpFile, STSC creat failed\r\n");
			return FALSE;
		}
	}
	else if(uiType == STSZ)
	{
		strcpy(&acDir[uiLen], DIR_STSZ);
		stSys->astStream[uiId].pStsz = fopen(acDir, "w+b");
		if(stSys->astStream[uiId].pStsz == NULL)
		{
			printf("CreatTmpFile, STSZ creat failed\r\n");
			return FALSE;
		}
	}
	else if(uiType == STCO)
	{
		strcpy(&acDir[uiLen], DIR_STCO);
		stSys->astStream[uiId].pStco = fopen(acDir, "w+b");
		if(stSys->astStream[uiId].pStco == NULL)
		{
			printf("CreatTmpFile, STCO creat failed\r\n");
			return FALSE;
		}
	}
	else if(uiType == STTS)
	{
		strcpy(&acDir[uiLen], DIR_STTS);
		stSys->astStream[uiId].pStts = fopen(acDir, "w+b");
		if(stSys->astStream[uiId].pStts == NULL)
		{
			printf("CreatTmpFile, STTS creat failed\r\n");
			return FALSE;
		}
	}
	else if(uiType == MDAT)
	{
		strcpy(&acDir[uiLen], DIR_MDAT);
		stSys->pMpg = fopen(acDir, "w+b");
		if(stSys->pMpg == NULL)
		{
			printf("CreatTmpFile, MDAT creat failed\r\n");
			return FALSE;
		}
	}
	else
	{
		printf("CreatTmpFile, type = %d\r\n", uiType);
		return FALSE;
	}

	return TRUE;
}

INT8 DelTmpFile(char* pacDir, UINT32 uiType, UINT32 uiId, STR_MP4INFO* stSys)
{
	char  acDir[MAX_DIRLEN], cAdd = 0;
	UINT32 uiLen = 0;
	UINT16 usNumber;
	uiLen = strlen(pacDir);
	if(uiLen >= MAX_INDIR)
	{
		printf("CreatTmpFile, input dir len = %d\r\n", uiLen);
		return FALSE;
	}
	if(uiId > 99)
		{return FALSE;}
	
	memset(acDir, 0, MAX_DIRLEN);	
	strcpy(acDir, pacDir);
	if(uiId> 0)
	{
		usNumber = NumberToAscii(uiId);
		usNumber = Swap16(usNumber);
		memcpy(&acDir[uiLen], (char*)&usNumber, 2);
		uiLen += 2;
	}

	if(uiType == STSS)
	{
		strcpy(&acDir[uiLen], DIR_STSS);
		if(stSys->astStream[uiId].pStss != NULL)
		{
			fclose(stSys->astStream[uiId].pStss);
		}
		if(0 == DeleteFile(acDir))
		{
			int err = GetLastError();
			printf("DelTmpFile,err = %d\n",err);
		}
	}
	else if(uiType == STSC)
	{
		strcpy(&acDir[uiLen], DIR_STSC);
		if(stSys->astStream[uiId].pStsc != NULL)
		{
			fclose(stSys->astStream[uiId].pStsc);
		}
		DeleteFile(acDir);
	}
	else if(uiType == STSZ)
	{
		strcpy(&acDir[uiLen], DIR_STSZ);
		if(stSys->astStream[uiId].pStsz != NULL)
		{
			fclose(stSys->astStream[uiId].pStsz);
		}
		DeleteFile(acDir);
	}
	else if(uiType == STCO)
	{
		strcpy(&acDir[uiLen], DIR_STCO);
		if(stSys->astStream[uiId].pStco != NULL)
		{
			fclose(stSys->astStream[uiId].pStco);
		}
		DeleteFile(acDir);
	}
	else if(uiType == STTS)
	{
		strcpy(&acDir[uiLen], DIR_STTS);
		if(stSys->astStream[uiId].pStts != NULL)
		{
			fclose(stSys->astStream[uiId].pStts);
		}
		DeleteFile(acDir);
	}
	else if(uiType == MDAT)
	{
		strcpy(&acDir[uiLen], DIR_MDAT);
		if(stSys->pMpg != NULL)
		{
			fclose(stSys->pMpg);
		}
		DeleteFile(acDir);
	}
	else
	{
		printf("CreatTmpFile, type = %d\r\n", uiType);
		return FALSE;
	}

	return TRUE;
}



INT8 WtFile(INT8* pacSrc, UINT32 uiLen, FILE* pFile)
{
	UINT32 uiWtLen = 0;
	uiWtLen = fwrite(pacSrc, sizeof(INT8), uiLen, pFile);
	if(uiWtLen!= uiLen)
	{
		//_PrintInfo_ERROR("WtFile failed\r\n");
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

INT8  ConvertSliceLen(char* pSlice, UINT32 uiSliceLen)
{
	UINT32 uiLen = Swap32(uiSliceLen);
	*((UINT32*) pSlice) = uiLen;
	return TRUE;
}

UINT32 CalcSliceLen(char* pFrame, UINT32 uiFrameLen, UINT32* pauiSliceLen)
{
	UINT32 i = 0, uiNum = 0, uiBeforeTotal = 0, auiPos[5];

	for(i = 0; i < MAX_FRAMEHEAD_READ, i < (uiFrameLen - 4); i++)
	{
		if(pFrame[i] == 0 && pFrame[i + 1] == 0 && pFrame[i + 2] == 0 && pFrame[i + 3] == 1)
		{				
			auiPos[uiNum] = i;
			if(uiNum != 0)
			{
				*(pauiSliceLen + uiNum - 1) = auiPos[uiNum] - auiPos[uiNum - 1] - 4;
				ConvertSliceLen(pFrame + auiPos[uiNum - 1], *(pauiSliceLen + uiNum - 1));
			}
			uiNum++;
		}
	}
	
	if(uiNum == 1)						   //fst slice,also the only slice
	{
		*(pauiSliceLen +0) = uiFrameLen - 4;
		ConvertSliceLen(pFrame, *(pauiSliceLen + 0));
	}
	else										   //last slice
	{
		for(i = 0; i < uiNum - 1; i++)
		{
			uiBeforeTotal += *(pauiSliceLen + i);
		}
		*(pauiSliceLen + uiNum - 1) = uiFrameLen - uiBeforeTotal - 4 * 3;                   //del 4 + other 2 * 4
		ConvertSliceLen(pFrame + auiPos[uiNum - 1], *(pauiSliceLen + uiNum - 1));
	}
	//printf("CalcSliceLen,slice num = %d, len =%d\n",uiNum, *pauiSliceLen);
	 
	return uiNum;

}

