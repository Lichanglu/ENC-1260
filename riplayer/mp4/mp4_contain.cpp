//#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "mp4_common.h"
#include "mp4_contain.h"
#include <windows.h> 
#include <conio.h> 
//#include <Record.h>

//STR_MP4INFO g_stSys;
//#define DURATION_TEST   1

void SetParam720576(STR_MP4INFO* stSys)
{
	UINT32 i = 0;
	
	stSys->stMpgParam.st720576.uiNum = 0x23;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x61;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x76;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x63;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x43;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x01;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x42;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x80;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x28;
	stSys->stMpgParam.st720576.aucPara[i++] = 0xff;
	stSys->stMpgParam.st720576.aucPara[i++] = 0xe1;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x00;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x0c;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x67;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x42;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x80;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x28;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x40;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x5c;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x09;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x2c;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x4e;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x80;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x01;
	stSys->stMpgParam.st720576.aucPara[i++] = 0;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x04;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x68;
	stSys->stMpgParam.st720576.aucPara[i++] = 0xde;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x3c;
	stSys->stMpgParam.st720576.aucPara[i++] = 0x80;
	
	return;

}

void SetParam640480(STR_MP4INFO* stSys)
{
	UINT32 i = 0;
	
	stSys->stMpgParam.st640480.uiNum = 0x21;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x61;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x76;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x63;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x43;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x01;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x42;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x80;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x28;
	stSys->stMpgParam.st640480.aucPara[i++] = 0xff;
	stSys->stMpgParam.st640480.aucPara[i++] = 0xe1;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x00;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x0a;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x67;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x42;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x80;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x28;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x40;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x50;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x1e;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x88;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x01;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x00;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x04;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x68;
	stSys->stMpgParam.st640480.aucPara[i++] = 0xde;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x3c;
	stSys->stMpgParam.st640480.aucPara[i++] = 0x80;
	
	return;

}

void SetParam704576( STR_MP4INFO* stSys)
{
	UINT32 i = 0;
	stSys->stMpgParam.st704576.uiNum = 0x21;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x61;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x76;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x63;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x43;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x01;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x42;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x80;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x28;
	stSys->stMpgParam.st704576.aucPara[i++] = 0xff;
	stSys->stMpgParam.st704576.aucPara[i++] = 0xe1;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x00;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x0a;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x67;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x42;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x80;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x28;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x8c;

	stSys->stMpgParam.st704576.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x40;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x58;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x09;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x22;

	stSys->stMpgParam.st704576.aucPara[i++] = 0x01;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x00;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x04;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x68;
	stSys->stMpgParam.st704576.aucPara[i++] = 0xde;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x3c;
	stSys->stMpgParam.st704576.aucPara[i++] = 0x80;
	
	return;
}

void SetParam720480( STR_MP4INFO* stSys)
{
	UINT32 i = 0;
	stSys->stMpgParam.st720480.uiNum = 0x22;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x61;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x76;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x63;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x43;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x01;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x42;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x80;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x28;
	stSys->stMpgParam.st720480.aucPara[i++] = 0xff;
	stSys->stMpgParam.st720480.aucPara[i++] = 0xe1;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x00;
	
	stSys->stMpgParam.st720480.aucPara[i++] = 0x0b;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x67;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x42;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x80;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x28;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x40;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x5c;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x1e;
	stSys->stMpgParam.st720480.aucPara[i++] = 0xb1;	
	stSys->stMpgParam.st720480.aucPara[i++] = 0x3a;
	
	stSys->stMpgParam.st720480.aucPara[i++] = 0x01;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x00;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x04;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x68;
	stSys->stMpgParam.st720480.aucPara[i++] = 0xde;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x3c;
	stSys->stMpgParam.st720480.aucPara[i++] = 0x80;
	
	return;
}

void SetParam800600( STR_MP4INFO* stSys)
{
	UINT32 i = 0;
	stSys->stMpgParam.st800600.uiNum = 0x22;//0x21;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x61;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x76;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x63;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x43;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x01;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x42;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x80;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x28;
	stSys->stMpgParam.st800600.aucPara[i++] = 0xff;
	stSys->stMpgParam.st800600.aucPara[i++] = 0xe1;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x00;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x0b;//0x0a;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x67;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x42;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x80;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x28;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x8c;
	
	stSys->stMpgParam.st800600.aucPara[i++] = 0x8c;//0x64;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x40;//0x09;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x64;//0xaf;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x09;//0x2a;
	stSys->stMpgParam.st800600.aucPara[i++] = 0xaf;
	
	stSys->stMpgParam.st800600.aucPara[i++] = 0x2a;//0x88;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x01;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x00;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x04;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x68;
	stSys->stMpgParam.st800600.aucPara[i++] = 0xde;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x3c;
	stSys->stMpgParam.st800600.aucPara[i++] = 0x80;
	
	return;
}

void SetParam1024576( STR_MP4INFO* stSys)
{
	UINT32 i = 0;
	stSys->stMpgParam.st1024576.uiNum = 0x22;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x61;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x76;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x63;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x43;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x01;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x42;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x80;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x28;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0xff;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0xe1;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x00;
	
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x0b;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x67;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x42;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x80;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x28;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x40;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x20;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x02;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x48;	
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x80;
	
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x01;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x00;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x04;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x68;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0xde;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x3c;
	stSys->stMpgParam.st1024576.aucPara[i++] = 0x80;
	
	return;
}

void SetParam1024768( STR_MP4INFO* stSys)
{
	UINT32 i = 0;
	stSys->stMpgParam.st1024768.uiNum = 0x22;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x61;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x76;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x63;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x43;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x01;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x42;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x80;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x28;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0xff;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0xe1;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x00;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x0b;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x67;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x42;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x80;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x28;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x8c;
	
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x40;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x20;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x03;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x08;
	
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x80;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x01;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x00;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x04;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x68;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0xde;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x3c;
	stSys->stMpgParam.st1024768.aucPara[i++] = 0x80;
	
	return;
}

void SetParam1280720( STR_MP4INFO* stSys)
{
	UINT32 i = 0;
	stSys->stMpgParam.st1280720.uiNum = 0x22;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x61;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x76;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x63;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x43;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x01;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x42;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x80;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x28;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0xff;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0xe1;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x00;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x0b;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x67;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x42;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x80;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x28;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x8c;
	
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x40;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x28;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x02;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0xd8;
	
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x80;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x01;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x00;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x04;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x68;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0xde;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x3c;
	stSys->stMpgParam.st1280720.aucPara[i++] = 0x80;
	
	return;
}

void SetParam1280960( STR_MP4INFO* stSys)
{
	UINT32 i = 0;
	stSys->stMpgParam.st1280960.uiNum = 0x22;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x61;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x76;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x63;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x43;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x01;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x42;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x80;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x28;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0xff;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0xe1;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x00;
	
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x0b;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x67;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x42;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x80;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x28;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x40;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x28;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x03;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0xc8;	
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x80;
	
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x01;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x00;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x04;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x68;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0xde;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x3c;
	stSys->stMpgParam.st1280960.aucPara[i++] = 0x80;
	
	return;
}

void SetParam1366768( STR_MP4INFO* stSys)
{
	UINT32 i = 0;
	stSys->stMpgParam.st1366768.uiNum = 0x23;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x61;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x76;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x63;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x43;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x01;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x42;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x80;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x28;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0xff;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0xe1;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x00;
	
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x0c;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x67;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x42;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x80;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x28;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x8c;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x40;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x2b;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x03;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x0b;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x36;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x80;
	
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x01;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x00;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x04;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x68;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0xde;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x3c;
	stSys->stMpgParam.st1366768.aucPara[i++] = 0x80;
	
	return;
}

void SetParam14001050( STR_MP4INFO* stSys)
{
	UINT32 i = 0;
	stSys->stMpgParam.st14001050.uiNum = 0x22;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x61;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x76;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x63;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x43;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x01;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x42;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x80;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x28;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0xff;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0xe1;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x00;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x0a;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x67;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x42;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x80;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x28;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x8c;
	
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x2c;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x01;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x0a;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0xcb;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x22;
	
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x88;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x01;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x00;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x04;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x68;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0xde;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x3c;
	stSys->stMpgParam.st14001050.aucPara[i++] = 0x80;
	
	return;
}


void SetParam19201088( STR_MP4INFO* stSys)
{
	UINT32 i = 0;
	stSys->stMpgParam.st19201088.uiNum = 0x22;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x61;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x76;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x63;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x43;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x01;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x42;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x80;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x28;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0xff;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0xe1;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x00;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x0b;//0x0a;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x67;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x42;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x80;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x28;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x8c;

	stSys->stMpgParam.st19201088.aucPara[i++] = 0x8c;//add
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x40;//add
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x3c;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x01;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x12;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x20;
	
	//stSys->stMpgParam.st19201088.aucPara[i++] = 0x88;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x01;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x00;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x04;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x68;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0xde;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x3c;
	stSys->stMpgParam.st19201088.aucPara[i++] = 0x80;
	
	return;
}

void SetParmAudio( STR_MP4INFO* stSys)
{
	UINT32 j = 0;
	//stSys->stMpgParam.aucStsdAudio = 0x23;
#ifdef _AUDIO_FORMAT_AACLC
	stSys->stMpgParam.aucStsdAudio[j++] = 0x65;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x73;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x64;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x73;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x03;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x19;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x01;//0x02;                                           //   1
	stSys->stMpgParam.aucStsdAudio[j++] = 0x1f;//0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x04;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x11;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x40;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x15;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x10;//0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x02;//0x01;//0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x1a;//0xe7;//0xfa;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x54;//0x57;//0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x01;//0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x72;//0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x5c;//0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x05;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x02;
	if(stSys->astStream[0].usSampleRate == 44100)
	{
		stSys->stMpgParam.aucStsdAudio[j++] = 0x12;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x10;
	}
	else
	{
		stSys->stMpgParam.aucStsdAudio[j++] = 0x11;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x90;
	}
	stSys->stMpgParam.aucStsdAudio[j++] = 0x06;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x01;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x02;
#endif

	stSys->stMpgParam.aucStsdAudio[j++] = 0x65;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x73;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x64;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x73;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x03;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x19;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x01;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x1f;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x04;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x11;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x40;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x15;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x10;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x02;
	if(stSys->astStream[0].usSampleRate == 44100)
	{
		stSys->stMpgParam.aucStsdAudio[j++] = 0x1a;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x54;
		stSys->stMpgParam.aucStsdAudio[j++] = 0;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x01;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x72;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x5c;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x05;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x02;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x13;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x90;
	}
	else
	{
		stSys->stMpgParam.aucStsdAudio[j++] = 0xcd;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x09;
		stSys->stMpgParam.aucStsdAudio[j++] = 0;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x01;
		stSys->stMpgParam.aucStsdAudio[j++] = 0xee;
		stSys->stMpgParam.aucStsdAudio[j++] = 0xe0;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x05;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x02;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x13;
		stSys->stMpgParam.aucStsdAudio[j++] = 0x10;
	}
	stSys->stMpgParam.aucStsdAudio[j++] = 0x06;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x01;
	stSys->stMpgParam.aucStsdAudio[j++] = 0x02;

	return;

}

void SetStsdParam(UINT32 uiId, STR_MP4INFO* stSys)                   
{
	UINT32 i = 0, j = 0;
	SetParam1024576(stSys);
	SetParam1024768(stSys);
	SetParam1280720(stSys);
	SetParam640480(stSys);
	SetParam704576(stSys);
	SetParam720480(stSys);
	SetParam800600(stSys);
	SetParam1280960(stSys);
	SetParam1366768(stSys);
	SetParam14001050(stSys);
	SetParam19201088(stSys);
	SetParam720576(stSys);
	SetParmAudio(stSys);

	return ;

}

UINT32  FillFtyp(STR_MPEG* pstMpg, UINT32  uiType)
{
	UINT32   uiSize = 0;
	memset((char*)(&pstMpg->stFtyp), 0, sizeof(STR_FTYP));
	
	pstMpg->stFtyp.uiType = FTYP;	
	if(uiType == VIDEO)
	{
		pstMpg->stFtyp.auiString[0] = ISOM;
		pstMpg->stFtyp.auiString[1] = 0x20000;
		pstMpg->stFtyp.auiString[2] = ISOM;
		pstMpg->stFtyp.auiString[3] = ISO2;
		pstMpg->stFtyp.auiString[4] = AVC1;
		pstMpg->stFtyp.auiString[5] = MP41;
		uiSize = sizeof(STR_FTYP);
		pstMpg->stFtyp.uiSize = Swap32(uiSize);
	}
	else if(uiType == AUDIO)
	{
		pstMpg->stFtyp.auiString[0] = ISOM;
		pstMpg->stFtyp.auiString[1] = 0x20000;
		pstMpg->stFtyp.auiString[2] = ISOM;
		pstMpg->stFtyp.auiString[3] = ISO2;
		pstMpg->stFtyp.auiString[4] = MP41;
		uiSize = sizeof(STR_FTYP) -sizeof(UINT32);
		pstMpg->stFtyp.uiSize = Swap32(uiSize);
	}
	else
	{

	}

	return uiSize;
}


INT8 FillMdatStart(STR_MPEG* pstMpg)
{
	pstMpg->stMdat.uiSize = HEAD_SIZE_8BYTE;
	pstMpg->stMdat.uiType = MDAT;

	return TRUE;
}

INT8 FillMdatEnd(STR_MPEG* pstMpg, UINT32 uiId, UINT32 uiType, STR_MP4INFO* stSys)
{
	UINT32 uiSize = 0;
	uiSize = Swap32(pstMpg->stMdat.uiSize);
	if(uiType == VIDEO)
	{
		fseek(stSys->pMpg, (sizeof(STR_FTYP) + sizeof(STR_FREE)), SEEK_SET);
	}
	else if(uiType == AUDIO)
	{
		fseek(stSys->pMpg, (sizeof(STR_FTYP) - sizeof(UINT32) + sizeof(STR_FREE)), SEEK_SET);
	}
	WtFile((INT8*)(&uiSize), sizeof(UINT32), stSys->pMpg);
	fseek(stSys->pMpg, 0, SEEK_END);
	//printf("FillMdatEnd,size=%d\n", pstMpg->stMdat.uiSize);

	return TRUE;
}

UINT32  FillFree(STR_MPEG* pstMpg)
{
	UINT32 uiSize = 0;
	memset((char*)(&pstMpg->stFree), 0, sizeof(STR_FREE));
	pstMpg->stFree.uiType = FREE;
	uiSize = HEAD_SIZE_8BYTE;
	pstMpg->stFree.uiSize = Swap32(uiSize);
	return uiSize;
}

UINT32 FillMoov(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	UINT32 uiSize = 0 , i = 0;
	pstMpg->stMoov.uiType = MOOV;
	uiSize += FillMvhd(pstMpg, uiId, stSys);
	//printf("FillMoov,mvhd=%d\n",uiSize);
	for(i = 0; i < stSys->uiStreamNum; i++)
	{
		uiSize += FillTrak(pstMpg, i, stSys);
		//printf("FillMoov,track[%d]=%d\n",i,uiSize);
	}
	//uiSize += FillUdta();
	uiSize += HEAD_SIZE_8BYTE;
	//printf("FillMoov,moov=%d\n",uiSize);
	pstMpg->stMoov.uiSize = Swap32(uiSize);

	return uiSize;
}

UINT32  FillMvhd(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_MVHD * pMvhd = NULL;
	UINT32  uiTime = 0, uiSize = 0;
	pMvhd= &(pstMpg->stMoov.stMvhd);
	memset((char*)pMvhd, 0, sizeof(STR_MVHD));
	
	pMvhd->uiType = MVHD;
	pMvhd->ucVer = 0;
	pMvhd->uiCreatTime = CTIME;
	pMvhd->uiModTime = CTIME;
	pMvhd->uiTimeScale = Swap32(SCALE_ONE_MSEC);
	uiTime = stSys->astStream[0].uiEndtime;//timeGetTime();
	#ifdef DURATION_TEST
	pMvhd->uiDura = Swap32((stSys->astStream[uiId].uiFrameCnt  * SCALE_ONE_MSEC)/ FRAME_RATE);
	#else
	pMvhd->uiDura = Swap32(uiTime - stSys->astStream[uiId].uiStarttime);
	#endif
	pMvhd->uiRate = Swap32(RATE_NORMAL);
	pMvhd->usVolume = Swap16(VOLUME_MAX);
	pMvhd->auiMatrix[0] = 0x100;
	pMvhd->auiMatrix[1] = 0;
	pMvhd->auiMatrix[2] = 0;
	pMvhd->auiMatrix[3] = 0;
	pMvhd->auiMatrix[4] = 0x100;
	pMvhd->auiMatrix[5] = 0;
	pMvhd->auiMatrix[6] = 0;
	pMvhd->auiMatrix[7] = 0;
	pMvhd->auiMatrix[8] = 0x40;
	pMvhd->auiPredef[0] = 0;
	pMvhd->auiPredef[1] = 0;
	pMvhd->auiPredef[2] = 0;
	pMvhd->auiPredef[3] = 0;
	pMvhd->auiPredef[4] = 0;
	pMvhd->auiPredef[5] = 0;
	pMvhd->uiNextTrack = Swap32(stSys->uiStreamNum + 1);//uiNext;                                            //PARAM!!
	uiSize = sizeof(STR_MVHD);
	pMvhd->uiSize = Swap32(uiSize);     //0x6c

	return uiSize;
}


UINT32  FillTrak(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_TRAK* pTrak = NULL;
	UINT32  uiTime = 0, uiSize = 0;
	pTrak= &(pstMpg->stMoov.stTrak[uiId]);
	memset((char*)pTrak, 0, sizeof(STR_TRAK));
	pTrak->uiType = TRAK;

	uiSize += FillTkhd(pstMpg, uiId, stSys);
	uiSize += FillMdia(pstMpg, uiId, stSys);
	uiSize += HEAD_SIZE_8BYTE;
	pTrak->uiSize = Swap32(uiSize);

	return uiSize;
}


UINT32  FillTkhd(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_TKHD* pTkhd = NULL;
	pTkhd= &(pstMpg->stMoov.stTrak[uiId].stTkhd);
	UINT32  uiTime = 0, uiSize = 0;
	memset((char*)pTkhd, 0, sizeof(STR_TKHD));

	pTkhd->uiType = TKHD;
	pTkhd->ucVer = VER_0;
	pTkhd->aucFlag[0] = 0;
	pTkhd->aucFlag[1] = 0;
	pTkhd->aucFlag[2] = TRACK_FM_VALUE;
	pTkhd->uiCreatTime = CTIME;
	pTkhd->uiModTime = CTIME;
	pTkhd->uiTrack = Swap32(uiId + 1);
	uiTime = stSys->astStream[0].uiEndtime;//timeGetTime();
#ifdef DURATION_TEST
	pTkhd->uiDura = Swap32((stSys->astStream[uiId].uiFrameCnt  * SCALE_ONE_MSEC) / FRAME_RATE);
#else
	pTkhd->uiDura = Swap32(uiTime - stSys->astStream[uiId].uiStarttime);
#endif

	pTkhd->usLay = Swap32(LAYER0);
	pTkhd->usAlte = Swap32(TK_NO_RELATION);
	if(stSys->astStream[uiId].uiType == AUDIO)
	{
		pTkhd->usVolume = Swap16(VOLUME_MAX);
	}
	else
	{
		pTkhd->usVolume = 0;
	}
	pTkhd->auiMatrix[0] = 0x100;
	pTkhd->auiMatrix[1] = 0;
	pTkhd->auiMatrix[2] = 0;
	pTkhd->auiMatrix[3] = 0;
	pTkhd->auiMatrix[4] = 0x100;
	pTkhd->auiMatrix[5] = 0;
	pTkhd->auiMatrix[6] = 0;
	pTkhd->auiMatrix[7] = 0;
	pTkhd->auiMatrix[8] = 0x40;
	if(stSys->astStream[uiId].uiType == VIDEO)
	{
		pTkhd->uiWidth = Swap32(stSys->astStream[uiId].uiWidth << 16);
		pTkhd->uiHeight = Swap32(stSys->astStream[uiId].uiHeight << 16);
	}
	else
	{
		pTkhd->uiWidth = 0;
		pTkhd->uiHeight = 0;
	}
	uiSize = sizeof(STR_TKHD);

	pTkhd->uiSize = Swap32(uiSize);    //0x5c

	return uiSize;

}


UINT32  FillMdia(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_MDIA* pMdia = NULL;
	UINT32 uiSize = 0;
	pMdia = &(pstMpg->stMoov.stTrak[uiId].stMdia);
	pMdia->uiType = MDIA;

	uiSize += FillMdhd(pstMpg, uiId, stSys);
	uiSize += FillHdlr(pstMpg, uiId, stSys);
	uiSize += FillMinf(pstMpg, uiId, stSys);
	uiSize += HEAD_SIZE_8BYTE;
	pMdia->uiSize = Swap32(uiSize);

	return uiSize;
}

UINT32  FillMdhd(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_MDHD * pMdhd = NULL;
	UINT32  uiTime = 0, uiSize = 0;
	pMdhd= &(pstMpg->stMoov.stTrak[uiId].stMdia.stMdhd);
	memset((char*)pMdhd, 0, sizeof(STR_MDHD));

	pMdhd->uiType = MDHD;
	pMdhd->ucVer = VER_0;
	pMdhd->aucFlag[0] = 0; 
	pMdhd->aucFlag[1] = 0; 
	pMdhd->aucFlag[2] = 0; 
	pMdhd->uiCreatTime = CTIME;
	pMdhd->uiModTime = CTIME;
	if(stSys->astStream[uiId].uiType == VIDEO)
	{
		pMdhd->uiTimeScale = Swap32(MP4_TIME_PRECISION * 1000);                //25
	}
	else if(stSys->astStream[uiId].uiType == AUDIO)
	{
		if(stSys->astStream[0].usSampleRate == 44100)
		{
			pMdhd->uiTimeScale = Swap32(SCALE_ONE_MSEC_44K1);
		}
		else
		{
			pMdhd->uiTimeScale = Swap32(SCALE_ONE_MSEC_48K);
		}
	}
	uiTime = stSys->astStream[0].uiEndtime;//timeGetTime();
	
#ifdef DURATION_TEST
	if(stSys->astStream[uiId].uiType == VIDEO)
	{
		pMdhd->uiDura = Swap32((stSys->astStream[uiId].uiFrameCnt * SCALE_ONE_MSEC) / FRAME_RATE );
	}
	else
	{
		pMdhd->uiDura = Swap32((stSys.->astStream[uiId].uiFrameCnt * SCALE_ONE_MSEC_44K1) / SCALE_44100_FRAMERATE) ;
	}
#else
	if(stSys->astStream[uiId].uiType == VIDEO)
	{
		pMdhd->uiDura = Swap32((uiTime - stSys->astStream[uiId].uiStarttime)*MP4_TIME_PRECISION);//((g_stSys.astStream[uiId].uiFrameCnt * 120)/30);//
	}
	else
	{
		if(stSys->astStream[0].usSampleRate == 44100)
		{
			pMdhd->uiDura = Swap32((uiTime - stSys->astStream[uiId].uiStarttime)*22.05);
		}
		else
		{
			pMdhd->uiDura = Swap32((uiTime - stSys->astStream[uiId].uiStarttime)*24);
		}
	}
#endif
	//printf("FillMdhd, duration is %x\n", ((uiTime - stSys->astStream[uiId].uiStarttime)));
	pMdhd->usLang = LAN_DEFAULT;                           //01  sound
	pMdhd->usPreDef = 0;
	uiSize = sizeof(STR_MDHD);

	pMdhd->uiSize = Swap32(uiSize);                   //0x20

	return uiSize;

}

UINT32  FillHdlr(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_HDLR * pHdlr = NULL;
	UINT32  uiSize = 0;
	pHdlr = &(pstMpg->stMoov.stTrak[uiId].stMdia.stHdlr);
	memset((char*)pHdlr, 0, sizeof(STR_HDLR));

	pHdlr->uiType = HDLR;
	pHdlr->ucVer = VER_0;
	pHdlr->aucFlag[0] = 0;
	pHdlr->aucFlag[1] = 0;
	pHdlr->aucFlag[2] = 0;
	pHdlr->uiPreDef = 0;
	if(stSys->astStream[uiId].uiType == VIDEO)
	{
		pHdlr->uiHdType = VIDE;
		strcpy(pHdlr->acName, V_HAND);
	}
	else if(stSys->astStream[uiId].uiType == AUDIO)
	{
		pHdlr->uiHdType = SOUN;
		strcpy(pHdlr->acName, A_HAND);
	}
	else
	{
	}

	uiSize = sizeof(STR_HDLR) + 1;                 // i 是以0结尾的字串
	pHdlr->uiSize = Swap32(uiSize);              //0x2d

	return uiSize;

}



UINT32  FillMinf(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_MINF* pMinf = NULL;
	UINT32 uiSize = 0;
	pMinf = &(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf);
	pMinf->uiType = MINF;

	uiSize += FillVshd(pstMpg, uiId, stSys);
	uiSize += FillDinf(pstMpg, uiId);
	uiSize += FillStbl(pstMpg, uiId, stSys);
	uiSize += HEAD_SIZE_8BYTE;

	pMinf->uiSize = Swap32(uiSize);

	return uiSize;
}

UINT32  FillVshd(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_VMHD* pVmhd = NULL;
	STR_SMHD* pSmhd = NULL;
	UINT32  uiSize = 0;
	if(stSys->astStream[uiId].uiType == VIDEO)
	{
		pVmhd = (STR_VMHD*)&(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf.unMfhd);
		memset((char*)pVmhd, 0, sizeof(STR_VMHD));

		pVmhd->uiType = VMHD;
		pVmhd->ucVer = VER_0;
		pVmhd->aucFlag[0] = 0;
		pVmhd->aucFlag[1] = 0;
		pVmhd->aucFlag[2] = FLAG_1;
		pVmhd->uiGrapMode = VMHD_GRAPMODE;
		pVmhd->ausOpcolor[0] = VMHD_OPCOLOR;
		pVmhd->ausOpcolor[1] = VMHD_OPCOLOR;
		pVmhd->ausOpcolor[2] = VMHD_OPCOLOR;		
		uiSize = sizeof(STR_VMHD);

		pVmhd->uiSize = Swap32(uiSize);
		return uiSize;
	}
	else
	{
		pSmhd = (STR_SMHD*)&(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf.unMfhd);
		memset((char*)pSmhd, 0, sizeof(STR_SMHD));

		pSmhd->uiType = SMHD;
		pSmhd->ucVer = VER_0;
		pSmhd->aucFlag[0] = 0;
		pSmhd->aucFlag[1] = 0;
		pSmhd->aucFlag[2] = FLAG_0;
		pSmhd->usBalan = VMHD_BALANCE;
		pSmhd->usResv = 0;
		uiSize = sizeof(STR_SMHD);

		pSmhd->uiSize = Swap32(uiSize);
		return uiSize;
	}

}


UINT32  FillDinf(STR_MPEG* pstMpg, UINT32 uiId)
{
	STR_DINF* pDinf = NULL;
	UINT32  uiSize = 0;
	pDinf = &(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf.stDinf);

	pDinf->uiType = DINF;
	uiSize += FillDref(pstMpg, uiId);
	uiSize += HEAD_SIZE_8BYTE;
	pDinf->uiSize = Swap32(uiSize);

	return uiSize;

}


UINT32  FillDref(STR_MPEG* pstMpg, UINT32 uiId)
{
	STR_DREF* pDref = NULL;
	UINT32  uiSize = 0;
	pDref = &(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf.stDinf.stDref);
	memset((char*)pDref, 0, sizeof(STR_DREF));

	pDref->uiType = DREF;
	pDref->ucVer = VER_0;
	pDref->aucFlag[0] = 0;
	pDref->aucFlag[1] = 0;
	pDref->aucFlag[2] = 0;
	pDref->uiCnt = Swap32(DREF_COUNT);
	uiSize += FillUrl(pstMpg, uiId, URL_NONE);
	uiSize += 16;

	pDref->uiSize = Swap32(uiSize);

	return uiSize;

}

//注意:结构和其他不一样
UINT32  FillUrl(STR_MPEG* pstMpg, UINT32 uiId, UINT32 uiType)
{
	STR_URL* pUrl = NULL;
	UINT32  uiSize = 0;
	pUrl = &(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf.stDinf.stDref.stUrl);
	memset((char*)pUrl, 0, sizeof(STR_URL));

	pUrl->uiType = URL_;
	if(uiType == URL_NONE)
	{
		uiSize = 0xc; 
		pUrl->uiSize = Swap32(uiSize);
		pUrl->acUrl[0] = 0;
		pUrl->acUrl[1] = 0;
		pUrl->acUrl[2] = 0;
		pUrl->acUrl[3] = 1;
		return uiSize;
	}

	strcpy(pUrl->acUrl, URL_STRING);
	uiSize = strlen(pUrl->acUrl);
	uiSize += HEAD_SIZE_8BYTE;
	pUrl->uiSize = Swap32(uiSize);

	return uiSize;
}

UINT32  FillStbl(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_STBL* pStbl = NULL;
	UINT32  uiSize = 0;
	pStbl = &(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf.stStbl);
	memset((char*)pStbl, 0, sizeof(STR_STBL));

	pStbl->uiType = STBL;

	uiSize += FillStsd(pstMpg, uiId, stSys);
	uiSize += FillStts(pstMpg, uiId, stSys);
	uiSize += FillStss(pstMpg, uiId, stSys);                            //can be pass?
	uiSize += FillStsc(pstMpg, uiId);
	uiSize += FillStsz(pstMpg, uiId, stSys);
	uiSize += FillStco(pstMpg, uiId, stSys);
	if(stSys->astStream[uiId].uiType == VIDEO)
		{uiSize += HEAD_SIZE_8BYTE + 16;}
	else if(stSys->astStream[uiId].uiType == AUDIO)
		{uiSize += HEAD_SIZE_8BYTE;}

	pStbl->uiSize = Swap32(uiSize);
	
	return uiSize;
}

UINT32  FillStsd(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_STSDA* pStsdA = NULL;
	STR_STSDV* pStsdV = NULL;
	UINT32  uiSize = 0;
	UINT16  usSize = 0;
	
	if(stSys->astStream[uiId].uiType == VIDEO)
	{
		pStsdV = (STR_STSDV*)&(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf.stStbl.stStsd);
		memset((char*)pStsdV, 0, sizeof(STR_STSDV));
		pStsdV->uiType = STSD;
		pStsdV->usIndex = Swap16(STSD_INDEX);

		pStsdV->stVideo.uiType = AVC1;
		pStsdV->stVideo.usIndex = Swap16(STSD_INDEX);
		pStsdV->stVideo.usPredef = 0;
		pStsdV->stVideo.auiPredef[0] = 0;
		pStsdV->stVideo.auiPredef[1] = 0;
		pStsdV->stVideo.auiPredef[2] = 0;
		usSize = (stSys->astStream[uiId].uiWidth & 0xffff);
		pStsdV->stVideo.usWidth = Swap16(usSize);           
		usSize = (stSys->astStream[uiId].uiHeight & 0xffff);
		pStsdV->stVideo.usHeight = Swap16(usSize);
		pStsdV->stVideo.uiHoriz = Swap32(STSD_HORIZ);
		pStsdV->stVideo.uiVert = Swap32(STSD_VERT);
		pStsdV->stVideo.usFrameCnt = Swap16(STSD_FRAMECNT);
		pStsdV->stVideo.usDepth = Swap16(STSD_DEPTH);
		pStsdV->stVideo.sPredef = Swap16(STSD_PREDEF);
		/*if(g_stSys.astStream[uiId].uiWidth == 1400)
		{
			pStsdV->stVideo.uiExtLen = Swap32(STSD_DESCR31 + sizeof(UINT32));
			memcpy(&pStsdV->stVideo.acExt[0], &stSys->stMpgParam.aucStsdVedio[0], STSD_DESCR31);
		}
		else
		{
			pStsdV->stVideo.uiExtLen = Swap32(STSD_DESCR30 + sizeof(UINT32));
			memcpy(&pStsdV->stVideo.acExt[0], &stSys->stMpgParam.aucStsdVedio[0], STSD_DESCR30);
		}*/
		if(stSys->astStream[uiId].uiWidth == 640 && stSys->astStream[uiId].uiHeight == 480)
		{
			pStsdV->stVideo.uiExtLen = Swap32(stSys->stMpgParam.st640480.uiNum);
			memcpy(pStsdV->stVideo.acExt, stSys->stMpgParam.st640480.aucPara, stSys->stMpgParam.st640480.uiNum);
			uiSize = sizeof(STR_STSDVIDEO) -STSD_EXTLEN + stSys->stMpgParam.st640480.uiNum - 6;
		}
		else if(stSys->astStream[uiId].uiWidth == 800 && stSys->astStream[uiId].uiHeight == 600)
		{
			pStsdV->stVideo.uiExtLen = Swap32(stSys->stMpgParam.st800600.uiNum);
			memcpy(pStsdV->stVideo.acExt, stSys->stMpgParam.st800600.aucPara, stSys->stMpgParam.st800600.uiNum);
			uiSize = sizeof(STR_STSDVIDEO) -STSD_EXTLEN + stSys->stMpgParam.st800600.uiNum - 6;
		}
		else if(stSys->astStream[uiId].uiWidth == 1024 && stSys->astStream[uiId].uiHeight == 576)
		{
			pStsdV->stVideo.uiExtLen = Swap32(stSys->stMpgParam.st1024576.uiNum);
			memcpy(pStsdV->stVideo.acExt, stSys->stMpgParam.st1024576.aucPara, stSys->stMpgParam.st1024576.uiNum);
			uiSize = sizeof(STR_STSDVIDEO) -STSD_EXTLEN + stSys->stMpgParam.st1024576.uiNum - 6;
		}
		else if(stSys->astStream[uiId].uiWidth == 720 && stSys->astStream[uiId].uiHeight == 480)
		{
			pStsdV->stVideo.uiExtLen = Swap32(stSys->stMpgParam.st720480.uiNum);
			memcpy(pStsdV->stVideo.acExt, stSys->stMpgParam.st720480.aucPara, stSys->stMpgParam.st720480.uiNum);
			uiSize = sizeof(STR_STSDVIDEO) -STSD_EXTLEN + stSys->stMpgParam.st720480.uiNum - 6;
		}
		else if(stSys->astStream[uiId].uiWidth == 704 && stSys->astStream[uiId].uiHeight == 576)
		{
			pStsdV->stVideo.uiExtLen = Swap32(stSys->stMpgParam.st704576.uiNum);
			memcpy(pStsdV->stVideo.acExt, stSys->stMpgParam.st704576.aucPara, stSys->stMpgParam.st704576.uiNum);
			uiSize = sizeof(STR_STSDVIDEO) -STSD_EXTLEN + stSys->stMpgParam.st704576.uiNum - 6;
		}
		else if(stSys->astStream[uiId].uiWidth == 1024 && stSys->astStream[uiId].uiHeight == 768)
		{
			pStsdV->stVideo.uiExtLen = Swap32(stSys->stMpgParam.st1024768.uiNum);
			memcpy(pStsdV->stVideo.acExt, stSys->stMpgParam.st1024768.aucPara, stSys->stMpgParam.st1024768.uiNum);
			uiSize = sizeof(STR_STSDVIDEO) -STSD_EXTLEN + stSys->stMpgParam.st1024768.uiNum - 6;
		}
		else if(stSys->astStream[uiId].uiWidth == 1280 && stSys->astStream[uiId].uiHeight == 720)
		{
			pStsdV->stVideo.uiExtLen = Swap32(stSys->stMpgParam.st1280720.uiNum);
			memcpy(pStsdV->stVideo.acExt, stSys->stMpgParam.st1280720.aucPara, stSys->stMpgParam.st1280720.uiNum);
			uiSize = sizeof(STR_STSDVIDEO) -STSD_EXTLEN + stSys->stMpgParam.st1280720.uiNum - 6;
		}
		else if(stSys->astStream[uiId].uiWidth == 1366 && stSys->astStream[uiId].uiHeight == 768)
		{
			pStsdV->stVideo.uiExtLen = Swap32(stSys->stMpgParam.st1366768.uiNum);
			memcpy(pStsdV->stVideo.acExt, stSys->stMpgParam.st1366768.aucPara, stSys->stMpgParam.st1366768.uiNum);
			uiSize = sizeof(STR_STSDVIDEO) -STSD_EXTLEN + stSys->stMpgParam.st1366768.uiNum - 6;
		}
		else if(stSys->astStream[uiId].uiWidth == 1400 && stSys->astStream[uiId].uiHeight == 1050)
		{
			pStsdV->stVideo.uiExtLen = Swap32(stSys->stMpgParam.st14001050.uiNum);
			memcpy(pStsdV->stVideo.acExt, stSys->stMpgParam.st14001050.aucPara, stSys->stMpgParam.st14001050.uiNum);
			uiSize = sizeof(STR_STSDVIDEO) -STSD_EXTLEN + stSys->stMpgParam.st14001050.uiNum - 6;
		}
		else if(stSys->astStream[uiId].uiWidth == 1920 && (stSys->astStream[uiId].uiHeight == 1088 || stSys->astStream[uiId].uiHeight == 1080))
		{
			pStsdV->stVideo.uiExtLen = Swap32(stSys->stMpgParam.st19201088.uiNum);
			memcpy(pStsdV->stVideo.acExt, stSys->stMpgParam.st19201088.aucPara, stSys->stMpgParam.st19201088.uiNum);
			uiSize = sizeof(STR_STSDVIDEO) -STSD_EXTLEN + stSys->stMpgParam.st19201088.uiNum - 6;
		}
		else if(stSys->astStream[uiId].uiWidth == 1280 && stSys->astStream[uiId].uiHeight == 960)
		{
			pStsdV->stVideo.uiExtLen = Swap32(stSys->stMpgParam.st1280960.uiNum);
			memcpy(pStsdV->stVideo.acExt, stSys->stMpgParam.st1280960.aucPara, stSys->stMpgParam.st1280960.uiNum);
			uiSize = sizeof(STR_STSDVIDEO) -STSD_EXTLEN + stSys->stMpgParam.st1280960.uiNum - 6;
		}
		else
		{
			pStsdV->stVideo.uiExtLen = Swap32(stSys->stMpgParam.st720576.uiNum);
			memcpy(pStsdV->stVideo.acExt, stSys->stMpgParam.st720576.aucPara, stSys->stMpgParam.st720576.uiNum);
			uiSize = sizeof(STR_STSDVIDEO) -STSD_EXTLEN + stSys->stMpgParam.st720576.uiNum - 6;
		}

		pStsdV->stVideo.uiSize = Swap32(uiSize);
		pStsdV->uiSize = Swap32(uiSize + 0x10);
		return uiSize;
	}
	else
	{
		pStsdA= (STR_STSDA*)&(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf.stStbl.stStsd);
		memset((char*)pStsdA, 0, sizeof(STR_STSDA));
		pStsdA->uiType = STSD;
		pStsdA->usIndex = Swap16(STSD_INDEX);

		pStsdA->stAudio.uiSize = sizeof(STR_STSDAUDIO);
		pStsdA->stAudio.uiType = MP4A;
		pStsdA->stAudio.usIndex = Swap16(STSD_INDEX);
		pStsdA->stAudio.usChannel = Swap16(STSD_CHANNEL);
		pStsdA->stAudio.usSampleSize = Swap16(STSD_SAMPLE_SIZE);
		pStsdA->stAudio.usPredef = 0;
		pStsdA->stAudio.uiSampleRate = Swap32(stSys->astStream[0].usSampleRate << 16);
		pStsdA->stAudio.uiExtLen = Swap32(STSD_EXTLEN_A);
		memcpy(pStsdA->stAudio.acExt, stSys->stMpgParam.aucStsdAudio, STSD_EXTLEN_A);

		uiSize = sizeof(STR_STSDAUDIO) -STSD_EXTLEN + STSD_EXTLEN_A - 4;
		pStsdA->stAudio.uiSize = Swap32(uiSize);
		pStsdA->uiSize = Swap32(uiSize + 0x10);
		return uiSize;

	}

	return 0;

}

UINT32  FillStts(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_STTS* pStts = NULL;
	UINT32  uiSize = 0, uiCount = 0;
	pStts = &(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf.stStbl.stStts);
	memset((char*)pStts, 0, sizeof(STR_STTS));
	pStts->uiType = STTS;
	pStts->uiVer = VER_0;

	uiCount = stSys->astStream[0].uiFrameCnt % FRAME_PERIOD;
	if(uiCount != 0)
	{
		uiCount = stSys->astStream[0].uiFrameCnt / FRAME_PERIOD + 1;
	}
	else
	{
		uiCount = stSys->astStream[0].uiFrameCnt / FRAME_PERIOD;
	}

	pStts->uiEntryCnt = Swap32(uiCount);

	uiSize = sizeof(STR_STTS) + uiCount*4*2;

	pStts->uiSize = Swap32(uiSize);

	return uiSize;
}

//每一个帧的TIMETICK
//FILE *pDebug =NULL;
void  Stts_EFrame(UINT32 uiId, UINT32 uiTimetick, UINT32 uiCount, STR_MP4INFO* stSys)
{
	UINT32  uiTmp = 0, uiCount1 = 0,uiCount2 = 0;
	double dFramerate = 0, dTime = 0;
	if(stSys->astStream[uiId].pStts)
	{
		//uiTmp = uiTimetick - g_stSys.astStream[uiId].uiLasttime;
		if(stSys->astStream[uiId].uiType == AUDIO)
		{
			uiCount1 = stSys->astStream[uiId].uiFrameCnt - stSys->uiLastNum;
			if(uiCount1 == 0)
				{return;}
			uiCount2 = Swap32(uiCount1);
			stSys->uiLastNum = stSys->astStream[uiId].uiFrameCnt;
			fwrite(&uiCount2, sizeof(UINT32), 1, stSys->astStream[uiId].pStts);
		}
		else
		{
			uiCount2 = Swap32(uiCount);
			fwrite(&uiCount2, sizeof(UINT32), 1, stSys->astStream[uiId].pStts);
		}

	
		if(stSys->astStream[uiId].uiType == VIDEO)
		{
			double dTimePeroid = ((uiTimetick - stSys->astStream[uiId].uiLasttime) > 1 ? (uiTimetick - stSys->astStream[uiId].uiLasttime) : 1);
			double  dTmp = (dTimePeroid*MP4_TIME_PRECISION)/uiCount;
			UINT32 m = (dTimePeroid*MP4_TIME_PRECISION)/uiCount;
			double n = (dTmp - m)*uiCount/MP4_TIME_PRECISION; //dTimePeroid - (m * uiCount)/MP4_TIME_PRECISION;
			//uiTmp = dTmp;//timeGetTime() - g_stSys.astStream[uiId].uiLasttime;
			stSys->astStream[uiId].uiLasttime = uiTimetick - n;//timeGetTime();
			//m = m > MP4_TIME_PRECISION ? m : MP4_TIME_PRECISION;
			uiTmp = Swap32(m);		
			fwrite(&uiTmp, sizeof(UINT32), 1, stSys->astStream[uiId].pStts);
			//printf("Stts_EFrame, video time=%x,lasttime=%x,dura=%x,count=%x\n",uiTimetick,stSys->astStream[uiId].uiLasttime,Swap32(uiTmp),uiCount);
		}
		else
		{
			dTime = ((uiTimetick - stSys->astStream[uiId].uiLasttime) > 1 ? (uiTimetick - stSys->astStream[uiId].uiLasttime) : 1);
			if(stSys->astStream[0].usSampleRate == 44100)
			{
				dFramerate = 43.06640625/2;
			}
			else
			{
				dFramerate = 46.875/2;
			}
			//uiCount2 = uiCount1;
			int flag = 0;
			double dRate1 = 0 , dRate2 = 0, dCount = 0 ;
			dCount = uiCount1;
			/*if(dCount > (dFramerate * dTime)/1000 )
			{
				dRate1 = STTS_AUDIO_SAMPLE * (dCount - (dFramerate * dTime) / 1000)/((dFramerate * dTime) / 1000);
				UINT32 m =  STTS_AUDIO_SAMPLE * (dCount - (dFramerate * dTime) / 1000)/((dFramerate * dTime) / 1000);
				double n = (((dCount* STTS_AUDIO_SAMPLE)/(dRate1-m+STTS_AUDIO_SAMPLE))*1000)/dFramerate;
				dRate2 = STTS_AUDIO_SAMPLE  - m ;//stSys->astStream[uiId].dGain; //
				if(dRate2 > 0)
				{
					uiTmp = (UINT32)dRate2;
					uiTmp = Swap32(uiTmp);
					stSys->astStream[uiId].uiLasttime = stSys->astStream[uiId].uiLasttime + m;//uiTimetick - n/STTS_AUDIO_SAMPLE;
				}
				else
				{
					uiTmp = Swap32(STTS_AUDIO_SAMPLE);
					stSys->astStream[uiId].uiLasttime = stSys->astStream[uiId].uiLasttime + 1000;
				}
			}
			else if(dCount == (dFramerate * dTime)/1000 )
			{
				uiTmp = Swap32(STTS_AUDIO_SAMPLE);
				stSys->astStream[uiId].uiLasttime = stSys->astStream[uiId].uiLasttime + 1000;//uiTimetick;
			}
			else
			{
				dRate1 = STTS_AUDIO_SAMPLE *((dFramerate * dTime) / 1000 - dCount)/((dFramerate * dTime) / 1000);
				UINT32 m = STTS_AUDIO_SAMPLE *((dFramerate * dTime) / 1000 - dCount)/((dFramerate * dTime) / 1000);
				double n = ((STTS_AUDIO_SAMPLE*dCount/(STTS_AUDIO_SAMPLE -dRate1 + m))*1000)/dFramerate;
				dRate2 = STTS_AUDIO_SAMPLE + m;//stSys->astStream[uiId].dGain;//
				uiTmp = (UINT32)dRate2;
				uiTmp = Swap32(uiTmp);
				stSys->astStream[uiId].uiLasttime = stSys->astStream[uiId].uiLasttime + m;//uiTimetick - n/STTS_AUDIO_SAMPLE;
			}*/
			//if(dCount > (dFramerate * dTime)/1000 )
			{
				dRate1 = STTS_AUDIO_SAMPLE * ((dFramerate * dTime) / 1000) / dCount;
				UINT32 m =  (STTS_AUDIO_SAMPLE * ((dFramerate * dTime) / 1000) / dCount + 0.3);
				//UINT32 m1 = STTS_AUDIO_SAMPLE * ((dFramerate * dTime) / 1000) / dCount;
				double n = (dRate1 - m)*dCount/(STTS_AUDIO_SAMPLE*dFramerate);
				//n = n > 0 ? n : 0;
				//m = m > 128 ? m : 128;
				uiTmp = Swap32(m);
				stSys->astStream[uiId].uiLasttime = uiTimetick - n;
			}
			/*else if(dCount == (dFramerate * dTime)/1000 )
			{
				uiTmp = Swap32(STTS_AUDIO_SAMPLE);
				stSys->astStream[uiId].uiLasttime = uiTimetick;
			}
			else
			{
				dRate1 = STTS_AUDIO_SAMPLE * ((dFramerate * dTime) / 1000) / dCount;
				UINT32 m =  STTS_AUDIO_SAMPLE * ((dFramerate * dTime) / 1000) / dCount;
				double n = (dRate1 - m)*dCount/(STTS_AUDIO_SAMPLE*dFramerate);
				uiTmp = Swap32(m);
				stSys->astStream[uiId].uiLasttime = uiTimetick - n;
			}*/
			

			//================================
			fwrite(&uiTmp, sizeof(UINT32), 1, stSys->astStream[uiId].pStts);
			//printf("Stts_EFrame, audio duration is %f\n", dRate2);
			/*char tmp[100];
			UINT32 clen = 0, uiTime = 0;
			//uiTime = dTime;
			memset(tmp, 0, 100);
			sprintf(tmp, "Stts_EFrame,rate1 = %f,rate2=%f, gain=%f\n",
				dRate1,dRate2,stSys->astStream[uiId].dGain);
			clen = strlen(tmp);
			fwrite(tmp, sizeof(char), clen, pDebug);*/
		}
		//g_stSys.astStream[uiId].uiLasttime = timeGetTime();
	}

	return;	
}


UINT32  FillStss(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_STSS* pStss = NULL;
	UINT32  uiSize = 0;
	pStss = &(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf.stStbl.stStss);
	memset((char*)pStss, 0, sizeof(STR_STSS));

	pStss->uiType = STSS;
	pStss->uiVer = VER_0;
	uiSize = sizeof(STR_STSS) + sizeof(UINT32) * stSys->astStream[uiId].uiKeyFrameCnt;
	pStss->uiEntryCnt = Swap32(stSys->astStream[uiId].uiKeyFrameCnt);
	pStss->uiSize = Swap32(uiSize);

	return uiSize;

}


//每一个关键帧在CHUNK中的位置
void  Stss_EKeyFrame(UINT32 uiId, STR_MP4INFO* stSys)
{
	UINT32  uiTmp = 0;
	stSys->astStream[uiId].uiKeyFrameCnt++;
	if(stSys->astStream[uiId].pStss)
	{
		uiTmp = Swap32(stSys->astStream[uiId].uiFrameCnt);
		fwrite(&uiTmp, sizeof(UINT32), 1, stSys->astStream[uiId].pStss);
	}

	return;	
}


UINT32  FillStsc(STR_MPEG* pstMpg, UINT32 uiId)
{
	STR_STSC* pStsc = NULL;
	UINT32  uiSize = 0;
	pStsc = &(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf.stStbl.stStsc);
	memset((char*)pStsc, 0, sizeof(STR_STSC));

	pStsc->uiType = STSC;
	pStsc->uiVer = VER_0;
	pStsc->uiEntryCnt = Swap32(STSC_CNT);
	pStsc->stChunk.uiFstChunk = Swap32(STSC_FST_CHK);
	pStsc->stChunk.uiSamplePerChunk = Swap32(STSC_SAMPLE_PCHK);
	pStsc->stChunk.uiSampleDescIndex = Swap32(STSC_SAMPLE_INDEX);
	uiSize = sizeof(STR_STSC);
	pStsc->uiSize = Swap32(uiSize);

	return uiSize;

}

//记录帧数和每个帧大小
UINT32  FillStsz(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_STSZ* pStsz = NULL;
	UINT32  uiSize = 0;
	pStsz = &(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf.stStbl.stStsz);
	memset((char*)pStsz, 0, sizeof(STR_STSZ));

	pStsz->uiType = STSZ;
	pStsz->uiVer = VER_0;
	pStsz->uiSampleSize = STSZ_SAMPLE_SIZE;
	uiSize = sizeof(STR_STSZ) + sizeof(UINT32) * stSys->astStream[uiId].uiFrameCnt;
	pStsz->uiSampleCnt = Swap32(stSys->astStream[uiId].uiFrameCnt);
	pStsz->uiSize = Swap32(uiSize);

	return uiSize;

}

//每一个帧的大小
void  Stsz_EFrame(UINT32 uiSize, UINT32 uiId, STR_MP4INFO* stSys)
{
	UINT32  uiTmp = 0, uiRet = 0;
	if(stSys->astStream[uiId].pStsz)
	{
		uiTmp = Swap32(uiSize);
		uiRet =  fwrite((char*)&uiTmp, sizeof(UINT8), 4, stSys->astStream[uiId].pStsz);
		if(sizeof(UINT32) != uiRet)
		{
			printf("Stsz_EFrame, failed, uiRet = %d\n", uiRet);
		}
	}

	return;	
}

//记录偏移地址
UINT32  FillStco(STR_MPEG* pstMpg, UINT32 uiId, STR_MP4INFO* stSys)
{
	STR_STCO* pStco = NULL;
	UINT32  uiSize = 0;
	pStco = &(pstMpg->stMoov.stTrak[uiId].stMdia.stMinf.stStbl.stStco);
	memset((char*)pStco, 0, sizeof(STR_STCO));

	pStco->uiType = STCO;
	pStco->uiVer = VER_0;
	pStco->uiEntryCnt = Swap32(stSys->astStream[uiId].uiFrameCnt);
	uiSize = sizeof(STR_STCO) + sizeof(UINT32) * stSys->astStream[uiId].uiFrameCnt;
	pStco->uiSize = Swap32(uiSize);

	return uiSize;

}

//每一个帧的起始地址偏移
void  Stco_EFrame(UINT32 uiSize, UINT32 uiId, STR_MP4INFO* stSys)
{
	UINT32  uiTmp = 0, uiTmpId = 0;
	//uiTmpId = (uiId == 0 ? 1 : 0);
	if(stSys->astStream[uiId].pStco)
	{
		//printf("Stco_EFrame,id=%d, offset=%d,size=%d\n",uiId,g_stSys.astStream[0].uiOffset,uiSize);
		uiTmp = Swap32(stSys->astStream[0].uiOffset + OFFSET_TO_START);
		stSys->astStream[0].uiOffset += uiSize;
		fwrite(&uiTmp, sizeof(UINT32), 1, stSys->astStream[uiId].pStco);
	}

	return;	
}


char* SysInit(char* pacDir, UINT32 uiStreamNum)
{
	INT8 i = 0, cErr = 0;
	UINT16 usLen = 0;
	FILE* pfile = NULL;
	char* pRet = NULL;
	
	usLen = strlen(pacDir);
	if(usLen > 128)
	{
		printf("SysInit, input file name len err\n");
		return NULL;
	}
	pRet = (char*)malloc(4096);
	STR_MP4INFO* stSys = (STR_MP4INFO*)pRet;
	memset(pRet, 0, 4096);
	stSys->pucDatbuf = (char*)malloc(2000000);

	stSys->uiStreamNum = uiStreamNum;
	for(i = 0; i < stSys->uiStreamNum; i++)
	{
		if(!CreatTmpFile(pacDir, STSS, i, stSys))
		{
			cErr = 1;
		}
		if(!CreatTmpFile(pacDir, STSC, i, stSys))
		{
			cErr = 1;
		}
		if(!CreatTmpFile(pacDir, STCO, i, stSys))
		{
			cErr = 1;
		}
		if(!CreatTmpFile(pacDir, STSZ, i, stSys))
		{
			cErr = 1;
		}
		if(!CreatTmpFile(pacDir, STTS, i, stSys))
		{
			cErr = 1;
		}
	}
	if(cErr)
	{
		printf("SysInit, creat tmp file failed\n");
		for(i = 0; i < stSys->uiStreamNum; i++)
		{
			DelTmpFile(pacDir, STSS, i, stSys);
			DelTmpFile(pacDir, STSC, i, stSys);
			DelTmpFile(pacDir, STSZ, i, stSys);
			DelTmpFile(pacDir, STCO, i, stSys);
			DelTmpFile(pacDir, STTS, i, stSys);
		}
		free(stSys->pucDatbuf);
		free(pRet);
		return NULL;
	}

	stSys->pMpg = fopen(pacDir, "w+b");
	strcpy(stSys->acFileName, pacDir);
	if(stSys->pMpg == NULL)
	{
		printf("SysInit, creat mpeg4 failed\r\n");
		free(stSys->pucDatbuf);
		free(pRet);
		return NULL;
	}

	return pRet;
}


INT8 AddStream(UINT32 uiType, UINT32 uiWidth, UINT32 uiHeight, UINT32 uiId, UINT32 uiSampleRate,UINT32 uiStartTime, char* acSys)
{
	STR_MP4INFO* stSys = (STR_MP4INFO*)acSys;
	if(uiId >= MAX_STREAM)
	{
		return FALSE;
	}

	stSys->astStream[uiId].uiWidth = uiWidth;
	stSys->astStream[uiId].uiHeight = uiHeight;
	stSys->astStream[uiId].uiType = uiType;
	stSys->astStream[0].usSampleRate = uiSampleRate;
	stSys->astStream[uiId].uiStarttime = uiStartTime;//timeGetTime();
	stSys->astStream[uiId].uiLasttime = uiStartTime;//timeGetTime();

	return TRUE;
}

INT8 AddToFile(FILE* pSrc, FILE* pDst)
{
	UINT32 uiLen = 0;
	char* pBuff = NULL;
	fseek(pDst, 0, SEEK_END);
	uiLen = ftell(pDst);  
	//printf("AddToFile, len = %d\n",uiLen);
	if(uiLen > 20000000)
	{
		printf("AddToFile, input file size=%d\n", uiLen);
		return 0;
	}
	pBuff = (char*)malloc(uiLen);
	fseek(pDst, 0, SEEK_SET);
	fread(pBuff, sizeof(char), uiLen, pDst);
	if(uiLen != fwrite(pBuff, sizeof(INT8), uiLen, pSrc))
	{
		Sleep(100);
	}
	free(pBuff);

	return 1;
}


INT8 CreatMpgContainer(UINT32 uiType, char* acSys)
{
	UINT32 uiSize = 0;
	STR_MP4INFO* stSys = (STR_MP4INFO*)acSys;
	if(stSys->uiStreamNum == 1)
	{
		if(uiType == VIDEO)
		{
			uiSize = FillFtyp(&(stSys->astMpg), VIDEO);
			WtFile((INT8*)(&(stSys->astMpg.stFtyp)), uiSize, stSys->pMpg);
			
			uiSize = FillFree(&(stSys->astMpg));
			WtFile((INT8*)(&(stSys->astMpg.stFree)), uiSize, stSys->pMpg);
			
			FillMdatStart(&(stSys->astMpg));
			WtFile((INT8*)(&(stSys->astMpg.stMdat)), HEAD_SIZE_8BYTE, stSys->pMpg);	

			//Stco_EFrame(OFFSET_TO_START, 0);
		}
		else if(uiType == AUDIO)
		{
			uiSize = FillFtyp(&(stSys->astMpg), AUDIO);
			WtFile((INT8*)(&(stSys->astMpg.stFtyp)), uiSize, stSys->pMpg);
			
			uiSize = FillFree(&(stSys->astMpg));
			WtFile((INT8*)(&(stSys->astMpg.stFree)), uiSize, stSys->pMpg);
			
			FillMdatStart(&(stSys->astMpg));
			WtFile((INT8*)(&(stSys->astMpg.stMdat)), HEAD_SIZE_8BYTE, stSys->pMpg);	

			//Stco_EFrame(OFFSET_TO_START_AUDIO, 0);
		}
	}
	else
	{
			uiSize = FillFtyp(&(stSys->astMpg), VIDEO);
			WtFile((INT8*)(&(stSys->astMpg.stFtyp)), uiSize, stSys->pMpg);
			
			uiSize = FillFree(&(stSys->astMpg));
			WtFile((INT8*)(&(stSys->astMpg.stFree)), uiSize, stSys->pMpg);
			
			FillMdatStart(&(stSys->astMpg));
			WtFile((INT8*)(&(stSys->astMpg.stMdat)), HEAD_SIZE_8BYTE, stSys->pMpg);	

			//Stco_EFrame(OFFSET_TO_START, 1);
	}

	//pDebug = fopen("d:\\debug.txt", "a+b");

	return TRUE;
}


INT8 FinishMpgContainer(char* pacDir, UINT32 uiEndtime, char* acSys)
{
	UINT32  uiSize = 0, i = 0, uiLen = 0;
	STR_MP4INFO* stSys = (STR_MP4INFO*)acSys;
	if(stSys->uiStreamNum == 1)
	{
		stSys->astStream[0].uiEndtime = uiEndtime;
		if(stSys->astStream[0].uiType == VIDEO)
		{
			UINT32 uiLeft = stSys->astStream[0].uiFrameCnt % FRAME_PERIOD;
			if(uiLeft != 0)
			{
				Stts_EFrame(0,uiEndtime,uiLeft, stSys);
			}
			SetStsdParam(0, stSys);
			
			FillMdatEnd(&(stSys->astMpg), 0, VIDEO, stSys);

			FillMoov(&(stSys->astMpg), 0, stSys);
			
			fseek( stSys->pMpg, 0, SEEK_END);
			WtFile((INT8*)(&(stSys->astMpg.stMoov)), HEAD_SIZE_8BYTE, stSys->pMpg);	      //moov
			WtFile((INT8*)(&(stSys->astMpg.stMoov.stMvhd)), sizeof(STR_MVHD), stSys->pMpg);// mvhd
			for(i = 0; i < stSys->uiStreamNum; i++)
			{
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i])), HEAD_SIZE_8BYTE, stSys->pMpg);             //trak
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stTkhd)), sizeof(STR_TKHD), stSys->pMpg);    //tkhd
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia)), HEAD_SIZE_8BYTE, stSys->pMpg);   //mdia
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMdhd)), 
					sizeof(STR_MDHD), stSys->pMpg);                                                                                //mdhd
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stHdlr)), 
					(sizeof(STR_HDLR) + ONE_BYTE_ZERO_TAIL), stSys->pMpg);                                          //hdlr
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf)), 
					HEAD_SIZE_8BYTE, stSys->pMpg);                                                                               //minf
				if(stSys->astStream[i].uiType == VIDEO)
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.unMfhd)), 
						sizeof(STR_VMHD), stSys->pMpg);                                                                         //vmhd
				}
				else
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.unMfhd)), 
						sizeof(STR_SMHD), stSys->pMpg);                                                                         //smhd
				}
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf)), 
					HEAD_SIZE_8BYTE, stSys->pMpg);                                                                               //dinf
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf.stDref)), 
					(sizeof(STR_DREF) - sizeof(STR_URL)), stSys->pMpg);                                                                                //dref
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf.stDref.stUrl)), 
					Swap32(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf.stDref.stUrl.uiSize), stSys->pMpg);  //url
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl)), 
					HEAD_SIZE_8BYTE, stSys->pMpg);                                                                               //stbl
				if(stSys->astStream[i].uiType == VIDEO)                                                                             //stsd
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd)), 
						sizeof(STR_STSDV) - STSD_EXTLEN - 6, stSys->pMpg);      
					uiLen = Swap32(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd.stStsdv.stVideo.uiExtLen);
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd.stStsdv.stVideo.uiExtLen)), 
						uiLen, stSys->pMpg); 
				}
				else
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd)), 
						sizeof(STR_STSDA), stSys->pMpg);  
				}
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStts)), 
					sizeof(STR_STTS), stSys->pMpg);                                                                                //stts
				AddToFile(stSys->pMpg, stSys->astStream[i].pStts);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStss)), 
					sizeof(STR_STSS), stSys->pMpg);                                                                                //stss
				AddToFile(stSys->pMpg, stSys->astStream[i].pStss);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsc)), 
					sizeof(STR_STSC), stSys->pMpg);                                                                                //stsc
				AddToFile(stSys->pMpg, stSys->astStream[i].pStsc);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsz)), 
					sizeof(STR_STSZ), stSys->pMpg);                                                                                //stsz
				AddToFile(stSys->pMpg, stSys->astStream[i].pStsz);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStco)), 
					sizeof(STR_STCO), stSys->pMpg);                                                                                //stco
				AddToFile(stSys->pMpg, stSys->astStream[i].pStco);
			}
		}
		else if(stSys->astStream[0].uiType == AUDIO)
		{
			SetStsdParam(0, stSys);
			
			FillMdatEnd(&(stSys->astMpg), 0, AUDIO, stSys);

			FillMoov(&(stSys->astMpg), 0, stSys);
			
			fseek( stSys->pMpg, 0, SEEK_END);
			WtFile((INT8*)(&(stSys->astMpg.stMoov)), HEAD_SIZE_8BYTE, stSys->pMpg);	      //moov
			WtFile((INT8*)(&(stSys->astMpg.stMoov.stMvhd)), sizeof(STR_MVHD), stSys->pMpg);// mvhd
			for(i = 0; i < stSys->uiStreamNum; i++)
			{
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i])), HEAD_SIZE_8BYTE, stSys->pMpg);             //trak
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stTkhd)), sizeof(STR_TKHD), stSys->pMpg);    //tkhd
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia)), HEAD_SIZE_8BYTE, stSys->pMpg);   //mdia
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMdhd)), 
					sizeof(STR_MDHD), stSys->pMpg);                                                                                //mdhd
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stHdlr)), 
					(sizeof(STR_HDLR) + ONE_BYTE_ZERO_TAIL), stSys->pMpg);                                          //hdlr
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf)), 
					HEAD_SIZE_8BYTE, stSys->pMpg);                                                                               //minf
				if(stSys->astStream[i].uiType == VIDEO)
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.unMfhd)), 
						sizeof(STR_VMHD), stSys->pMpg);                                                                         //vmhd
				}
				else
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.unMfhd)), 
						sizeof(STR_SMHD), stSys->pMpg);                                                                         //smhd
				}
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf)), 
					HEAD_SIZE_8BYTE, stSys->pMpg);                                                                               //dinf
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf.stDref)), 
					(sizeof(STR_DREF) - sizeof(STR_URL)), stSys->pMpg);                                                                                //dref
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf.stDref.stUrl)), 
					Swap32((stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf.stDref.stUrl.uiSize)), stSys->pMpg);  //url
				WtFile((INT8*)(&stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl), 
					HEAD_SIZE_8BYTE, stSys->pMpg);                                                                               //stbl
				if(stSys->astStream[i].uiType == VIDEO)                                                                             //stsd
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd)), 
						sizeof(STR_STSDV) - STSD_EXTLEN - 6, stSys->pMpg);      
					uiLen = Swap32(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd.stStsdv.stVideo.uiExtLen);
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd.stStsdv.stVideo.uiExtLen)), 
						uiLen, stSys->pMpg); 
				}
				else
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd)), 
						sizeof(STR_STSDA) - STSD_EXTLEN - 4, stSys->pMpg);      
					uiLen = Swap32(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd.stStsdA.stAudio.uiExtLen);
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd.stStsdA.stAudio.uiExtLen)), 
						uiLen, stSys->pMpg);
				}
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStts)), 
					sizeof(STR_STTS), stSys->pMpg);   
				AddToFile(stSys->pMpg, stSys->astStream[i].pStts);//stts
				//WtFile((INT8*)(&stSys.astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStss), 
				//	sizeof(STR_STSS), stSys.pMpg);                                                                                //stss
				//AddToFile(stSys->pMpg, stSys->astStream[i].pStss);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsc)), 
					sizeof(STR_STSC), stSys->pMpg);                                                                                //stsc
				AddToFile(stSys->pMpg, stSys->astStream[i].pStsc);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsz)), 
					sizeof(STR_STSZ), stSys->pMpg);                                                                                //stsz
				AddToFile(stSys->pMpg, stSys->astStream[i].pStsz);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStco)), 
					sizeof(STR_STCO), stSys->pMpg);                                                                                //stco
				AddToFile(stSys->pMpg, stSys->astStream[i].pStco);
			}
		}
	}
	else
	{
		UINT32 uiLeft = stSys->astStream[0].uiFrameCnt % FRAME_PERIOD;
		stSys->astStream[0].uiEndtime = uiEndtime;
		stSys->astStream[1].uiEndtime = uiEndtime;
		if(uiLeft != 0)
		{
			//printf("FinishMpgContainer,frame=%d,left=%d,endtime=%d\n",stSys->astStream[0].uiFrameCnt,uiLeft,uiEndtime);
			Stts_EFrame(0,uiEndtime,uiLeft,stSys);			
		}
		Stts_EFrame(1,uiEndtime,uiLeft,stSys);

		SetStsdParam(0,stSys);
		
		FillMdatEnd(&(stSys->astMpg), 0, VIDEO, stSys);

		FillMoov(&(stSys->astMpg), 0, stSys);
		
		fseek( stSys->pMpg, 0, SEEK_END);
		WtFile((INT8*)(&(stSys->astMpg.stMoov)), HEAD_SIZE_8BYTE, stSys->pMpg); 	  //moov
		WtFile((INT8*)(&(stSys->astMpg.stMoov.stMvhd)), sizeof(STR_MVHD), stSys->pMpg);// mvhd
		for(i = 0; i < stSys->uiStreamNum; i++)
		{
			if(stSys->astStream[i].uiType == VIDEO)
			{
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i])), HEAD_SIZE_8BYTE, stSys->pMpg);			  //trak
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stTkhd)), sizeof(STR_TKHD), stSys->pMpg);	 //tkhd
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia)), HEAD_SIZE_8BYTE, stSys->pMpg);   //mdia
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMdhd)), 
					sizeof(STR_MDHD), stSys->pMpg);																				//mdhd
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stHdlr)), 
					(sizeof(STR_HDLR) + ONE_BYTE_ZERO_TAIL), stSys->pMpg); 										 //hdlr
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf)), 
					HEAD_SIZE_8BYTE, stSys->pMpg); 																			  //minf
				if(stSys->astStream[i].uiType == VIDEO)
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.unMfhd)), 
						sizeof(STR_VMHD), stSys->pMpg);																		 //vmhd
				}
				else
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.unMfhd)), 
						sizeof(STR_SMHD), stSys->pMpg);																		 //smhd
				}
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf)), 
					HEAD_SIZE_8BYTE, stSys->pMpg); 																			  //dinf
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf.stDref)), 
					(sizeof(STR_DREF) - sizeof(STR_URL)), stSys->pMpg);																				//dref
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf.stDref.stUrl)), 
					Swap32(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf.stDref.stUrl.uiSize), stSys->pMpg);  //url
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl)), 
					HEAD_SIZE_8BYTE, stSys->pMpg); 																			  //stbl
				if(stSys->astStream[i].uiType == VIDEO)																			 //stsd
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd)), 
						sizeof(STR_STSDV) - STSD_EXTLEN - 6, stSys->pMpg); 	 
					uiLen = Swap32(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd.stStsdv.stVideo.uiExtLen);
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd.stStsdv.stVideo.uiExtLen)), 
						uiLen, stSys->pMpg); 
				}
				else
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd)), 
						sizeof(STR_STSDA), stSys->pMpg);  
				}
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStts)), 
					sizeof(STR_STTS), stSys->pMpg);																				//stts
				AddToFile(stSys->pMpg, stSys->astStream[i].pStts);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStss)), 
					sizeof(STR_STSS), stSys->pMpg);																				//stss
				AddToFile(stSys->pMpg, stSys->astStream[i].pStss);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsc)), 
					sizeof(STR_STSC), stSys->pMpg);																				//stsc
				AddToFile(stSys->pMpg, stSys->astStream[i].pStsc);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsz)), 
					sizeof(STR_STSZ), stSys->pMpg);																				//stsz
				AddToFile(stSys->pMpg, stSys->astStream[i].pStsz);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStco)), 
					sizeof(STR_STCO), stSys->pMpg);																				//stco
				AddToFile(stSys->pMpg, stSys->astStream[i].pStco);
			}
			else
			{
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i])), HEAD_SIZE_8BYTE, stSys->pMpg);			  //trak
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stTkhd)), sizeof(STR_TKHD), stSys->pMpg);	 //tkhd
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia)), HEAD_SIZE_8BYTE, stSys->pMpg);   //mdia
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMdhd)), 
					sizeof(STR_MDHD), stSys->pMpg);																				//mdhd
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stHdlr)), 
					(sizeof(STR_HDLR) + ONE_BYTE_ZERO_TAIL), stSys->pMpg); 										 //hdlr
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf)), 
					HEAD_SIZE_8BYTE, stSys->pMpg); 																			  //minf
				if(stSys->astStream[i].uiType == VIDEO)
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.unMfhd)), 
						sizeof(STR_VMHD), stSys->pMpg);																		 //vmhd
				}
				else
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.unMfhd)), 
						sizeof(STR_SMHD), stSys->pMpg);																		 //smhd
				}
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf)), 
					HEAD_SIZE_8BYTE, stSys->pMpg); 																			  //dinf
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf.stDref)), 
					(sizeof(STR_DREF) - sizeof(STR_URL)), stSys->pMpg);																				//dref
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf.stDref.stUrl)), 
					Swap32(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stDinf.stDref.stUrl.uiSize), stSys->pMpg);  //url
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl)), 
					HEAD_SIZE_8BYTE, stSys->pMpg); 																			  //stbl
				if(stSys->astStream[i].uiType == VIDEO)																			 //stsd
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd)), 
						sizeof(STR_STSDV) - STSD_EXTLEN - 6, stSys->pMpg); 	 
					uiLen = Swap32(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd.stStsdv.stVideo.uiExtLen);
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd.stStsdv.stVideo.uiExtLen)), 
						uiLen, stSys->pMpg); 
				}
				else
				{
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd)), 
						sizeof(STR_STSDA) - STSD_EXTLEN - 4, stSys->pMpg); 	 
					uiLen = Swap32(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd.stStsdA.stAudio.uiExtLen);
					WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsd.stStsdA.stAudio.uiExtLen)), 
						uiLen, stSys->pMpg);
				}
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStts)), 
					sizeof(STR_STTS), stSys->pMpg);																				//stts
				AddToFile(stSys->pMpg, stSys->astStream[i].pStts);
				//WtFile((INT8*)(&stSys.astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStss), 
				//	sizeof(STR_STSS), stSys.pMpg);																				//stss
				//AddToFile(stSys->pMpg, stSys->astStream[i].pStss);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsc)), 
					sizeof(STR_STSC), stSys->pMpg);																				//stsc
				AddToFile(stSys->pMpg, stSys->astStream[i].pStsc);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStsz)), 
					sizeof(STR_STSZ), stSys->pMpg);																				//stsz
				AddToFile(stSys->pMpg, stSys->astStream[i].pStsz);
				WtFile((INT8*)(&(stSys->astMpg.stMoov.stTrak[i].stMdia.stMinf.stStbl.stStco)), 
					sizeof(STR_STCO), stSys->pMpg);																				//stco
				AddToFile(stSys->pMpg, stSys->astStream[i].pStco);
			}
		}
	}
	Sleep(1000);
	fclose(stSys->pMpg);
	for(i = 0; i < stSys->uiStreamNum; i++)
	{
		DelTmpFile(pacDir, STSS, i, stSys);
		DelTmpFile(pacDir, STSC, i, stSys);
		DelTmpFile(pacDir, STSZ, i, stSys);
		DelTmpFile(pacDir, STCO, i, stSys);
		DelTmpFile(pacDir, STTS, i, stSys);
	}
	free(stSys->pucDatbuf);
	free(acSys);

	return TRUE;
}


INT8 WtOneFrame(char* pFrame, UINT32 uiFrameLen, UINT32 uiId, UINT32 uiType, UINT32 uiKey, UINT32 uiTimetick, char* acSys)
{
	UINT32 i = 0, j = 0, uiSliceNum = 0, uiBeforeTotal = 0, uiLeft = 0;
	UINT32 auiSliceLen[5];
	STR_MP4INFO* stSys = (STR_MP4INFO*)acSys;
	if(stSys->uiFstFrame == 0)
	{
		for(i = 0; i < stSys->uiStreamNum; i++)
		{
			stSys->astStream[i].uiStarttime = uiTimetick;
			stSys->astStream[i].uiLasttime = uiTimetick;
		}
		stSys->uiFstFrame = 1;
		//printf("WtOneFrame, fst frame\n");
	}

	
	if(stSys->astStream[uiId].uiType == VIDEO)
	{
		if(uiType != VIDEO)
		{
			printf("WtOneFrame, input video frame type err\r\n");
			return 0x10;
		}
		if((pFrame[0] != 0) | (pFrame[1] != 0) | (pFrame[2] != 0) | (pFrame[3] != 1))
		{
			printf("WtOneFrame, input frame is not h264 format\r\n");
			return 0x11;
		}
		if(uiFrameLen > 1000000)
		{
			printf("WtOneFrame, input video len is %d\r\n", uiFrameLen);
			return 0x12;
		}
		
		memcpy(stSys->pucDatbuf, pFrame, uiFrameLen);
		uiSliceNum = CalcSliceLen(stSys->pucDatbuf, uiFrameLen, auiSliceLen);
		WtFile((INT8*)stSys->pucDatbuf, uiFrameLen, stSys->pMpg);
		stSys->astStream[uiId].uiFrameCnt++;
		stSys->astMpg.stMdat.uiSize += uiFrameLen;
		Stsz_EFrame(uiFrameLen, uiId, stSys);
		Stco_EFrame(uiFrameLen, uiId, stSys);
		if(stSys->astStream[uiId].uiFrameCnt % FRAME_PERIOD == 0)
		{
			Stts_EFrame(uiId, uiTimetick, FRAME_PERIOD, stSys);
			stSys->uiSliceflag = 1;
		}
		if(uiKey)
		{
			Stss_EKeyFrame(uiId, stSys);
		}
	}
	else
	{
		if(uiType != AUDIO)
		{
			printf("WtOneFrame, input audio frame type err\r\n");
			return 0x20;
		}
		if((((UINT8)pFrame[0]) != 0xff) | (((UINT8)pFrame[1]) != 0xf1))
		{
			printf("WtOneFrame, input frame is not aac format, %d %d %d %d\r\n", pFrame[0],
				pFrame[1], pFrame[2], pFrame[3]);
			return 0x21;
		}
		if(uiFrameLen > 1000)
		{
			printf("WtOneFrame, input audio len is %d\r\n", uiFrameLen);
			return 0x22;
		}
		
		WtFile((INT8*)(pFrame + ADTS_HEAD), (uiFrameLen - ADTS_HEAD), stSys->pMpg);
		stSys->astStream[uiId].uiFrameCnt++;
		stSys->astMpg.stMdat.uiSize += (uiFrameLen - ADTS_HEAD);
		Stsz_EFrame((uiFrameLen - ADTS_HEAD), uiId, stSys);
		Stco_EFrame((uiFrameLen - ADTS_HEAD), uiId, stSys);
		if(stSys->uiSliceflag == 1)
		{
			Stts_EFrame(uiId, uiTimetick, FRAME_PERIOD, stSys);
			stSys->uiSliceflag = 0;
		}
	}

	if(stSys->astStream[0].uiOffset >= 0x7f000000 || stSys->astStream[1].uiOffset >= 0x7f000000)
	{
		UINT32 uiLeft = stSys->astStream[0].uiFrameCnt % FRAME_PERIOD;
		if(uiLeft != 0)
		{
			if(stSys->uiStreamNum == 0)
			{
				Stts_EFrame(0,uiTimetick,uiLeft, stSys);
			}
			else
			{
				Stts_EFrame(0,uiTimetick,uiLeft, stSys);

			}
		}
		Stts_EFrame(1,uiTimetick,uiLeft, stSys);
		UINT32 uiEnd = timeGetTime();
		FinishMpgContainer(stSys->acFileName, uiEnd, acSys);
	}

	return 1;
}

/*int main()
{
	printf("size=%d\n",sizeof(STR_MP4INFO));
	return 0;
}*/
