/*****************************************************************
*  中控协议实现
*
******************************************************************/

//add by zhangmin
#ifdef GDM_RTSP_SERVER

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include "rtsp_authentication.h"

#include "mid_platform.h"
#include "rtsp_server.h"
#include "app_rtsp_center.h"


/*******************static function*********************************************/
static int app_center_start_feedback(char *buff, int *len);
static int app_center_stop_feedback(char *buff, int *len);
static int app_center_write_data(HANDLE  m_hCom, unsigned char *buf, int dwlen);
static int app_center_read_data(HANDLE  m_hcom, unsigned char *buf, int dwsize);
static unsigned __stdcall  app_center_proc(void *pParams);

/*******************static parameter*******************************************/
static int g_start_flag = APP_CENTER_STOP; //0 stop 1 starting

// cl4000 center start feedback
static int app_center_start_feedback(char *buff, int *len)
{
	char passwd[64] = "0000000000";
	char user[64] = {0};
	char localip[64] = {0};
	//get local ip
	rtsp_get_local_ip(localip, sizeof(localip));
	app_authentication_get_passwd(passwd);
	app_authentication_get_user(user);
	//start feedback
	sprintf(buff, "[STREAMING!][URL:http://%s][USERNAME:%s][PASSWORD:%s]", localip, user, passwd);
	*len = strlen(buff);
	return 0;
}

//cl4000 center stop feedback
static int app_center_stop_feedback(char *buff, int *len)
{
	sprintf(buff, "%s", "[STREAMSTOPPED!]");
	*len = strlen(buff);
	return 0;
}

#define DEFAULT_WEB_CONTROL "D:\\Data\\rtspuser\\app_web_center.ini"
//value == 2 表示web控制start
//value == 3表示已经处理
//value == 4 表示web 控制stop
static int app_center_web_get_control()
{
	int value = 0;
	value = GetPrivateProfileInt("web", "control", 1, DEFAULT_WEB_CONTROL);
	//PRINTF("app_center_web_start = %d\n",value);
	return value;
}
static int app_center_web_set_control(char *value)
{

	WritePrivateProfileString("web", "control", value, DEFAULT_WEB_CONTROL);
	return 0;

}
static unsigned __stdcall  app_center_proc(void *pParams)
{
	unsigned char buff[1024] = {0};
	int i = 0;
	int rl = 0;
	int wl = 0;
	int ret = 0;
	int num = 0;

	HANDLE m_hCom = (HANDLE)(pParams);
	PRINTF("app center proc is begin,the m_hCom = %p\n", m_hCom);

	while(1) {
		mid_plat_sleep(500);
		memset(buff, 0, sizeof(buff));
		rl = app_center_read_data(m_hCom, buff, 24);

		//PRINTF("app_center_proc.%d\n",rl);
		//	PRINTF("buff = #%s#,flag = %d\n",buff,app_center_get_stream_flag() );
		if(rl != 0) {
			PRINTF("buff = #%s#,flag = %d\n", buff, app_center_get_stream_flag());

			//num ++;
			if(strcmp((char *)buff, "[STARTSTREAM]") == 0 && (app_center_get_stream_flag() == APP_CENTER_STOP)) {
				PRINTF("I rececv serial control begin the rtsp stream\n");
				app_authentication_start();
				memset(buff, 0, sizeof(buff));
				app_center_start_feedback((char *)buff, &wl);

				ret =  app_center_write_data(m_hCom, buff, wl);
				app_center_set_stream_flag(APP_CENTER_START);
				PRINTF("i will write the serial control  start feedback wl = %d,ret = %d\n", wl, ret);
			} else if(strcmp((char *)buff, "[STREAMSTOP]") == 0)
				//&& (app_center_get_stream_flag()  ==APP_CENTER_START))
			{
				PRINTF("I rececv serial control stop the rtsp stream\n");
				app_authentication_stop();
				memset(buff, 0, sizeof(buff));
				app_center_stop_feedback((char *)buff, &wl);
				ret =  app_center_write_data(m_hCom, buff, wl);
				app_center_set_stream_flag(APP_CENTER_STOP);
				//close the rtsp client
				rtsp_close_all_client();
				PRINTF("i will write the serial control stop feedback  wl = %d,ret = %d\n", wl, ret);
			}

		} else {
			num ++;

			if(num > 10000) {
				num = 0;
			}
		}

		if(num % 2) { // 1s //web center
			if(app_center_web_get_control() == 2) {
				if(app_center_get_stream_flag() == APP_CENTER_STOP) {
					PRINTF("I rececv web control  begin the rtsp stream\n");
					app_authentication_start();
					app_center_set_stream_flag(APP_CENTER_START);
					PRINTF("i will write the web control  start feedback wl = %d,ret = %d\n", wl, ret);
				}

				app_center_web_set_control("3");
			} else if(app_center_web_get_control() == 4) {
				PRINTF("I rececv web control stop the rtsp stream\n");
				app_authentication_stop();
				app_center_set_stream_flag(APP_CENTER_STOP);
				//close the rtsp client
				rtsp_close_all_client();
				PRINTF("i will write the web control stop feedback  wl = %d,ret = %d\n", wl, ret);
				app_center_web_set_control("3");
			}
		}

	}

	PRINTF("ERROR!,the app center is exit\n");
	return 0;
}


int app_center_init(char *pCom)
{
	HANDLE	 m_hCom = NULL;
	unsigned int 	 dwError;
	m_hCom = CreateFile(("COM1:"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if(m_hCom != INVALID_HANDLE_VALUE) {
		DCB dcb;

		if(GetCommState(m_hCom, &dcb)) {
			//			DWORD BaudRate : 波特率 (默认 = 9600)
			//			BYTE StopBits : 0,1,2 = 1, 1.5, 2 (默认 = 0)
			//			BYTE Parity : 0-4= 无, 奇, 偶, 标志, 空格 (默认 = 0)
			//			BYTE ByteSize : 数据位, 4-8 (默认 = 8)

			dcb.BaudRate = GetPrivateProfileInt("Comm", "Baud", 9600, APP_CENTER_FILE);	//设置速率
			//dcb.StopBits = 1;//GetPrivateProfileInt("Comm","StopBit",1,m_chProtocolPath);
			dcb.Parity = GetPrivateProfileInt("Comm", "Parity", 0, APP_CENTER_FILE);
			dcb.ByteSize = GetPrivateProfileInt("Comm", "DataBit", 8, APP_CENTER_FILE);	//数据位

			PRINTF("init ,the baudrate =%d,the parity=%d,the bytesize=%d\n", dcb.BaudRate ,
			       dcb.Parity, dcb.ByteSize);

			if(SetCommState(m_hCom, &dcb)) {

				// COMMTIMEOUTS对象 -> 设置超时
				COMMTIMEOUTS comTimeOut;
				// 接收时，两字符间最大的时延
				comTimeOut.ReadIntervalTimeout = 60;
				// 读取每字节的超时
				comTimeOut.ReadTotalTimeoutMultiplier = 30;
				// 读串口数据的固定超时
				// 总超时 = ReadTotalTimeoutMultiplier * 字节数 + ReadTotalTimeoutConstant
				comTimeOut.ReadTotalTimeoutConstant = 2;
				// 写每字节的超时
				comTimeOut.WriteTotalTimeoutMultiplier = 3;
				// 写串口数据的固定超时
				comTimeOut.WriteTotalTimeoutConstant = 2;

				// 将超时参数写入设备控制
				if(SetCommTimeouts(m_hCom, &comTimeOut)) {
					_beginthreadex(NULL, 0, app_center_proc, (void *)m_hCom, 0, NULL);
					return 1;
				} else {
					//dwError = GetLastError();
					if(CloseHandle(m_hCom)) {
						m_hCom = INVALID_HANDLE_VALUE;
					}

					return 0;
				}
			}

			dwError = GetLastError();

			if(CloseHandle(m_hCom)) {
				m_hCom = INVALID_HANDLE_VALUE;
			}

			return 0;
		}

		dwError = GetLastError();

		if(CloseHandle(m_hCom)) {
			m_hCom = INVALID_HANDLE_VALUE;
		}

		return 0;
	} else {
		PRINTF("WARNNING,THE serial init error\n");
	}

	return 0;
}



static int mid_serial_write(HANDLE	 m_hCom, unsigned char *outputData,
                            const unsigned int  sizeBuffer,
                            int *length)
{
	//PRINTF("mid_write :m_hCom = %p\n",m_hCom);
	if(m_hCom != INVALID_HANDLE_VALUE && sizeBuffer > 0) {
		return WriteFile(m_hCom,				// handle to file to write to
		                 outputData,			// pointer to data to write to file
		                 sizeBuffer,			// number of bytes to write
		                 (unsigned long *)length, NULL);     // pointer to number of bytes written
	}

	return -1 ;
}

static int mid_serial_read(HANDLE	 m_hCom, unsigned char *inputData,
                           const unsigned int sizeBuffer,
                           int  *length)
{
	if(m_hCom != INVALID_HANDLE_VALUE && sizeBuffer > 0) {
		int bResult = ReadFile(m_hCom,					// handle of file to read
		                       inputData,				// handle of file to read
		                       sizeBuffer,				// number of bytes to read
		                       (unsigned long *)	length,				// pointer to number of bytes read
		                       NULL);					// pointer to structure for data
		//PRINTF("sizeBUffer = %d,length = %d,bResult = %d\n",sizeBuffer,*length,bResult);
		//	printf("Send Data Failed 0x%x \r\n",GetLastError() );
		/*		if (!bResult)
				{
					DWORD dwError = GetLastError();
					CString str;
					str.Format("Error Code: %d\n",dwError);
					OutputDebugString(str);
				}
		*/
		return bResult;
	}

	return -1;
}


static int app_center_read_data(HANDLE	 m_hcom, unsigned char *buf, int dwsize)
{

	int rd = 0;
	mid_serial_read(m_hcom, buf, dwsize, &rd);

	return rd;
}

static int app_center_write_data(HANDLE	 m_hCom, unsigned char *buf, int dwlen)
{
	int wl = 0;
	int ln = 0;

	for(int i = 1; i < 6; ++ i) {
		wl = mid_serial_write(m_hCom, buf, dwlen, &ln);

		//PRINTF("wl = %d,ln= %d,dwlen = %d\n",wl,ln,dwlen);
		if(wl == -1) {
			//	printf("Send Data Failed 0x%x \r\n",GetLastError() );
			ln = 0;
		}

		if(ln == 0) {
			Sleep(i * 80);
		} else {
			break;
		}
	}

	return ln;
}

int app_center_get_stream_flag(void)
{
	return g_start_flag;
}

int app_center_set_stream_flag(int flag)
{
	if(flag < APP_CENTER_MAX) {
		g_start_flag = flag;
		return 0;
	}

	return -1;

}

#endif

