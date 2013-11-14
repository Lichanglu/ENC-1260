#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>

//#include "netcom.h"
#include "common.h"
#include "sysparam.h"
#include "tcpcom.h"
#include "demo.h"
#include "log_common.h"
/*
#############LCD order  start#################
*/
#define FLAG_CLR			0x01
#define FLAG_RESET			0x02
#define FLAG_WAYLEFT		0x05
#define FLAG_WAYRIGHT		0x06
#define FLAG_HIDECURSOR		0x0C
#define FLAG_SHOWCURSOR		0x0F
#define FLAG_MOVELEFT		0x10
#define FLAG_MOVERIGHT		0x14
#define FLAG_SCREENLEFT		0x18
#define FLAG_SCREENRIGHT	0x1C
#define FLAG_SETCURSOR		0x80
#define FLAG_LIGHTSHOW		0x28
#define FLAG_LIGHTHIDE		0x2C

#define FLAG_UP				0xB8
#define FLAG_DOWN			0xE8
#define FLAG_ENTER			0xD8
#define FLAG_ESCAPE			0x78

#define FLAG_NULL			0x1
#define FLAG_CENTER			0x2
#define FLAG_LEFT			0x4
#define FLAG_RIGHT			0x8
#define FLAG_NEWLINE		0x10

#define ONE_LINE_POS		0x00
#define TWO_LINE_POS		0x40

#define MAX_DISPLAY_CHAR	16

#define DSP1 PORT_ONE
/*
#############LCD order  end#################
*/

/*system param table goboal*/
extern SavePTable  gSysParaT;
extern DSPCliParam gDSPCliPara[PORT_NUM];

int dwShowFlag = 0;
int gLcdFD = -1;

/*send ttyS order*/
static int SendOrder(int fd, int nFlag, int nParam)
{
	unsigned char data[4];
	int length = 0, ret = -1;

	memset(data, 0, sizeof(data));

	data[0] = 0xF8;
	data[1] = nFlag;
	length = 2;

	if(nFlag == FLAG_SETCURSOR) {
		data[2] = nParam;
		length++;
	}

	ret = SendDataToCom(fd, data, length);

	if(ret == -1) {
		DEBUG(DL_ERROR, "SendOrder()::SendDataToCom() send error !!!\n");
		return ret;
	}

	return (ret);

}

/*LCD display char*/
static int __SendData(int fd, unsigned char *pdata, int nLen, int nFlag)
{
	int nLength = nLen + 4;
	unsigned char send[300];
	int ret;

	if((nFlag & FLAG_CENTER) == FLAG_CENTER) {
		if(MAX_DISPLAY_CHAR > nLen) {
			SendOrder(fd, FLAG_SETCURSOR, (MAX_DISPLAY_CHAR - nLen) / 2 + (((nFlag & FLAG_NEWLINE) == FLAG_NEWLINE) ? TWO_LINE_POS : ONE_LINE_POS));
		} else {
			SendOrder(fd, FLAG_SETCURSOR, (((nFlag & FLAG_NEWLINE) == FLAG_NEWLINE) ? TWO_LINE_POS : ONE_LINE_POS));
		}
	} else if((nFlag & FLAG_LEFT) == FLAG_LEFT) {
		SendOrder(fd, FLAG_SETCURSOR, (((nFlag & FLAG_NEWLINE) == FLAG_NEWLINE) ? TWO_LINE_POS : ONE_LINE_POS));
	} else if((nFlag & FLAG_RIGHT) == FLAG_RIGHT) {
		if(16 > nLen) {
			SendOrder(fd, FLAG_SETCURSOR, MAX_DISPLAY_CHAR - nLen + (((nFlag & FLAG_NEWLINE) == FLAG_NEWLINE) ? TWO_LINE_POS : ONE_LINE_POS));
		} else {
			SendOrder(fd, FLAG_SETCURSOR, (((nFlag & FLAG_NEWLINE) == FLAG_NEWLINE) ? TWO_LINE_POS : ONE_LINE_POS));
		}
	}

	memset(send, 0, nLength);
	send[0] = 0xF8;
	send[1] = 0x03;
	memcpy(send + 2, pdata, nLen);
	send[nLen + 2] = 0xA0;

	ret = SendDataToCom(fd, send, nLength);

	if(ret == -1) {
		DEBUG(DL_ERROR, "SendData()::SendDataToCom() send error!!!\n");
		return (ret);
	}

	return (ret);
}

static int SendData(int fd, char *pdata, int uflag)
{
	return (__SendData(fd, (unsigned char *)pdata, strlen(pdata), uflag));
}

/*ReadName */
static void ReadName(char *name)
{
	strcpy(name, gSysParaT.sysPara.strName);
}

/*ReadVerNo*/
static void ReadVerNo(char *ver)
{
	strcpy(ver, gSysParaT.sysPara.strVer);
}

/*Read IP addr*/
static void ReadIPMaskaddr(char *ipaddr, int flag)
{
	struct in_addr	addr ;

	if(flag == 0) {   //IP addr
		memcpy(&addr, &gSysParaT.sysPara.dwAddr, 4);
		strcpy(ipaddr, inet_ntoa(addr));
	} else if(flag == 1) {  //Netmask
		memcpy(&addr, &gSysParaT.sysPara.dwNetMark, 4);
		strcpy(ipaddr, inet_ntoa(addr));
	} else {  		//GateWay
		memcpy(&addr, &gSysParaT.sysPara.dwGateWay, 4);
		strcpy(ipaddr, inet_ntoa(addr));
	}
}

/*read video infomation*/
static void ReadVideoinfo(char *str, int num, int line)
{
	switch(num) {
		case 0:
			switch(line) {   //VIDEO width and height
				case 1:
					sprintf(str, "Width:%d", gSysParaT.videoPara[DSP1].nWidth);
					break;

				case 2:
					sprintf(str, "Height:%d", gSysParaT.videoPara[DSP1].nHight);
					break;
			}

			break;

		case 1:     //Encode param
			switch(line) {
				case 1:
					if(gSysParaT.videoPara[DSP1].sCbr == 0) { //VBR
						strcpy(str, "VBR");
					} else {
						strcpy(str, "CBR");
					}

					break;

				case 2:
					if(gSysParaT.videoPara[DSP1].sCbr == 0) {
						sprintf(str, "Quality:%d", gSysParaT.videoPara[DSP1].nQuality);
					} else {
						sprintf(str, "BitRate:%dK", gSysParaT.videoPara[DSP1].sBitrate);
					}

					break;
			}

			break;

		case 2:		//Audio param
			switch(line) {
				case 1:
					sprintf(str, "SampleRate:%dK", (ChangeSampleIndex(gSysParaT.audioPara[DSP1].SampleRate)) / 1000);
					break;

				case 2:
					sprintf(str, "BitRate:%dK", (gSysParaT.audioPara[DSP1].BitRate) / 1000);
					break;
			}

			break;
	}
}

/*client number*/
void ReadClientNum(char *str)
{
	int i, cli_num = 0 ;

	for(i = 0; i < MAX_CLIENT; i++) {
		if(ISUSED(DSP1, i) && ISLOGIN(DSP1, i))	 {
			cli_num++;
		}
	}

	sprintf(str, "Clients:%d", cli_num);
}

/*LCD show infomation*/
void LCDShowInfo(int fd)
{
	char strTemp[200];

	switch(dwShowFlag) {
		case 0:			//encode Name
			SendOrder(fd, FLAG_CLR, 0);
			memset(strTemp, 0, sizeof(strTemp));
			ReadName(strTemp);
			SendData(fd, strTemp, FLAG_CENTER);
			ReadVerNo(strTemp);
			SendData(fd, strTemp, FLAG_CENTER | FLAG_NEWLINE);
			break;

		case 1:
			SendOrder(fd, FLAG_CLR, 0);
			memset(strTemp, 0, sizeof(strTemp));
			GetDeviceType(strTemp);
			SendData(fd, strTemp, FLAG_CENTER);
			ReadIPMaskaddr(strTemp, 0);  //IP
			SendData(fd, strTemp, FLAG_CENTER | FLAG_NEWLINE);
			break;

		case 2:
			SendOrder(fd, FLAG_CLR, 0);
			memset(strTemp, 0, sizeof(strTemp));
			ReadIPMaskaddr(strTemp, 1);  //NW
			SendData(fd, strTemp, FLAG_CENTER);
			ReadIPMaskaddr(strTemp, 2);  //GW
			SendData(fd, strTemp, FLAG_CENTER | FLAG_NEWLINE);
			break;

		case 3:
			SendOrder(fd, FLAG_CLR, 0);
			ReadVideoinfo(strTemp, 0, 1); // width
			SendData(fd, strTemp, FLAG_CENTER);
			ReadVideoinfo(strTemp, 0, 2); // height
			SendData(fd, strTemp, FLAG_CENTER | FLAG_NEWLINE);
			break;

		case 4:
			SendOrder(fd, FLAG_CLR, 0);
			ReadVideoinfo(strTemp, 1, 1);  //VBR or CBR
			SendData(fd, strTemp, FLAG_CENTER);
			ReadVideoinfo(strTemp, 1, 2); //quality or bitrate
			SendData(fd, strTemp, FLAG_CENTER | FLAG_NEWLINE);
			break;

		case 5:
			SendOrder(fd, FLAG_CLR, 0);
			ReadVideoinfo(strTemp, 2, 1);  //Audio samplerate
			SendData(fd, strTemp, FLAG_CENTER);
			ReadVideoinfo(strTemp, 2, 2); //Audio bitrate
			SendData(fd, strTemp, FLAG_CENTER | FLAG_NEWLINE);
			break;
	}

	dwShowFlag++;

	if(dwShowFlag > 5) {	//display
		dwShowFlag = 0;
	}
}

/*Thread LCD*/
void LCDModuleTask(void *param)
{
	PRINTF("get pid= %d\n", getpid());

	//signal_set();
	while(!gblGetQuit()) {  /*LCD Module*/
		sleep(3);
		LCDShowInfo(gLcdFD);
	}
}

#if 0
void SetSpeed(int fd, int speed)
{
	int i;
	int status;
	struct termios Opt;
	int speed_arr[] = { B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	                    B38400, B19200, B9600, B4800, B2400, B1200, B300,
	                  };
	int name_arr[] = {115200, 38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,
	                  19200,  9600, 4800, 2400, 1200,  300,
	                 };

	tcgetattr(fd, &Opt);

	for(i = 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
		if(speed == name_arr[i]) {
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);

			if(status != 0) {
				DEBUG(DL_ERROR, "tcsetattr fd\n");
				return;
			}

			tcflush(fd, TCIOFLUSH);
		}
	}
}

int SetParity(int fd, int databits, int stopbits, int parity)
{
	struct termios options;

	if(tcgetattr(fd, &options)  !=  0) {
		DEBUG(DL_ERROR, "SetupSerial 1");
		return(FALSE);
	}

	options.c_cflag &= ~CSIZE;
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	options.c_oflag  &= ~OPOST;   			/*Output*/
	options.c_iflag   &= ~IXON;  			//0x11
	options.c_iflag   &= ~ICRNL;  			//0x0d
	//	options.c_cflag|=CLOCAL;
	//	options.c_cflag|=CREAD;
	//	options.c_cflag&=~CRTSCTS;

	switch(databits) {
		case 7:
			options.c_cflag |= CS7;
			break;

		case 8:
			options.c_cflag |= CS8;
			break;

		default:
			DEBUG(DL_ERROR, "Unsupported data size\n");
			return (FALSE);
	}

	switch(parity) {
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;  	/* Clear parity enable */
			options.c_iflag &= ~INPCK;	 	/* Enable parity checking */
			break;

		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;    	/* Disnable parity checking */
			break;

		case 'e':
		case 'E':
			options.c_cflag |= PARENB;     	/* Enable parity */
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;       /* Disnable parity checking */
			break;

		case 'S':
		case 's':  							/*as no parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;

		default:
			DEBUG(DL_ERROR, "Unsupported parity\n");
			return (FALSE);
	}

	switch(stopbits) {
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;

		case 2:
			options.c_cflag |= CSTOPB;
			break;

		default:
			DEBUG(DL_ERROR, "Unsupported stop bits\n");
			return (FALSE);
	}

	/* Set input parity option */
	if(parity != 'n') {
		options.c_iflag |= INPCK;
	}

	tcflush(fd, TCIFLUSH);
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 1; 				/* define the minimum bytes data to be readed*/

	if(tcsetattr(fd, TCSANOW, &options) != 0) {
		DEBUG(DL_ERROR, "SetupSerial 3");
		return (FALSE);
	}

	return (TRUE);
}


void ClosePort(int fd)
{
	if(fd != -1) {
		close(fd);
	}
}


int OpenPort(int port_num)
{
	char port[4][20] = {{"/dev/ttyS0"}, {"/dev/ttyS1"}, {"/dev/ttyS2"}, {"/dev/ttyS3"}};
	int fd;

	if(port_num > 3 || port_num < 0) {
		DEBUG(DL_ERROR, "port num error:%d\n", port_num);
		return -1;
	}

	if((fd = open(port[port_num], O_RDWR)) < 0) {
		DEBUG(DL_ERROR, "ERROR: failed to open %s, errno=%d\n", port[port_num], errno);
		return -1;
	} else {
		DEBUG(DL_ERROR, "Open %s success!\n", port[port_num]);
	}

	return fd;
}

int InitPort(int port_num, int baudrate,
             int databits, int stopbits, int parity)
{
	int fd;

	fd = OpenPort(port_num);

	if(fd == -1) {
		return -1;
	}

	SetSpeed(fd, baudrate);

	if(SetParity(fd, databits, stopbits, parity) == FALSE) {
		DEBUG(DL_ERROR, "Set Parity Error!\n");
		ClosePort(fd);
		return -1;
	}

	return fd;
}
#endif
int LCDCtrlComInit(int com)
{
	int baudrate = 9600, databits = 8;
	int stopbits = 1, parity = 'N';
	int fd;

	fd = InitPort(com, baudrate, databits, stopbits, parity);

	if(fd <= 0) {
		return -1;
	}

	return fd;
}

/*open lcd module communication ttyS*/
void OpenLCDCom()
{
	gLcdFD = LCDCtrlComInit(PORT_COM2);

	if(gLcdFD < 0) {
		DEBUG(DL_ERROR, "OpenLCDCom:LCDCtrlComInit() failed fd = %d errno = %d!!\n", gLcdFD, errno);
	}
}
