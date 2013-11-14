/*********************************************************************

Co.Name	:	   Reach Software
FileName:		   remotectrl2.c
Creater：     yangshh                    Date: 2010-08-18
Function：	  Remote Control Module
Other Description:	Control Camera	for com port

**********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <time.h>

#include "remotectrl.h"
#include "sysparam.h"
#include "common.h"
#include "track.h"
/*remote config File */
int gRemoteFD = -1;
RemoteConfig gRemote;
char FarCtrlList[7][20] = {{"EVI-D70"},	//protocol for remote
	{"EVI-D100"},
	{"PELCO-D"},
	{"PELCO-P"},
	{"VCC-H80PI"},
	{"AW-HE50"},
	{""}
};


extern int SetTrackStatus(int fd, int val);

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

void ClosePort(int fd)
{
	if(fd != -1) {
		close(fd);
	}
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
static unsigned long writen(int fd, const void *vptr, size_t n)
{
	unsigned long nleft;
	unsigned long nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;

	while(nleft > 0) {
		if((nwritten = write(fd, ptr, nleft)) <= 0) {
			if(nwritten < 0 && errno == EINTR) {
				nwritten = 0;    /* and call write() again */
			} else {
				return(-1);    /* error */
			}
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}

	return(n);
}
/* end writen */
/*send data to tty com*/
int SendDataToCom(int fd, unsigned char *data, int len)
{
	unsigned long real_len = 0 ;

	if((real_len = writen(fd, data, len)) != len) {
		DEBUG(DL_ERROR, "SendDataToCom() write tty error\n");
		return -1;
	}

	usleep(20000);
	return (real_len);
}

/*initial camera*/
int InitCamera(int fd)
{
	unsigned char comm[16];
	int length = 0;
	int ret = -1;

	if(fd == -1) {
		goto INITCEXIT;
	}

	memset(comm, 0, sizeof(comm));
	comm[0] = 0x88;
	comm[1] = 0x01;
	comm[2] = 0x00;
	comm[3] = 0x01;
	comm[4] = 0xFF;
	length = 5;
	ret = SendDataToCom(fd, comm, length);

	if(ret == -1) {
		goto INITCEXIT;
	}

	usleep(200);
	memset(comm, 0, sizeof(comm));
	comm[0] = 0x88;
	comm[1] = 0x30;
	comm[2] = 0x01;
	comm[3] = 0xff;
	length = 4;
	ret = SendDataToCom(fd, comm, length);

	if(ret == -1) {
		goto INITCEXIT;
	}

	usleep(200);

	memset(comm, 0, sizeof(comm));
	comm[0] = 0x81;
	comm[1] = 0x09;
	comm[2] = 0x00;
	comm[3] = 0x02;
	comm[4] = 0xFF;
	length = 5;
	ret = SendDataToCom(fd, comm, length);

	if(ret == -1) {
		goto INITCEXIT;
	}

	usleep(200);

	DEBUG(DL_DEBUG, "InitCamera() Succeed\n");
	return 0;
INITCEXIT:
	DEBUG(DL_ERROR, "InitCamera() Error\n");
	return -1;
}


/*control command Analysis*/
int CCtrlCodeAnalysis(unsigned char *pCode, char *strCode,
                      unsigned char Address, unsigned char hSpeed,
                      unsigned char vSpeed)
{
	char pstr[16];
	int nIndex = 0;
	char *ptmp = 0;
	char *ptmp1 = 0;

	memset(pstr, 0, 16);

	ptmp1 = strCode;
	ptmp = strstr(strCode, ",");

	while(ptmp != NULL) {
		strncpy(pstr, ptmp1, ptmp - ptmp1);

		if(pstr[0] == 0x5e) {
			pCode[nIndex] = Address;
		} else if(pstr[0] == 'H' && pstr[1] == '(') {
			pCode[nIndex] = hSpeed;
		} else if(pstr[0] == 'V' && pstr[1] == '(') {
			pCode[nIndex] = vSpeed;
		} else {
			pCode[nIndex] = strtol(pstr, 0, 16);
		}

		nIndex++;
		ptmp1 = ptmp + 1;
		ptmp = strstr(ptmp + 1, ",");
		memset(pstr, 0, 16);
	}

	strcpy(pstr, ptmp1);

	if(pstr[0] == '=') {
		if(pstr[1] == '7') {
			pCode[nIndex] = pCode[0] ^ pCode[1] ^ pCode[2] ^ pCode[3] ^ pCode[4] ^ pCode[5] ^ pCode[6];
		}
	} else if(pstr[0] == '+') {
		if(pstr[1] == '5') {
			pCode[nIndex] = (pCode[1] + pCode[2] + pCode[3] + pCode[4] + pCode[5]) % 256;
		}
	} else {
		pCode[nIndex] = strtol(pstr, 0, 16);
	}

	return nIndex + 1;
}



/*HE50 摄像头速度索引表*/
int HE50CameraIndex(int speed, int way)
{
	int index = 0;

	switch(way) {
		case UPSTART:
			switch(speed) {
				case 1:
					index = 6;
					break;

				case 5:
					index = 7;
					break;

				case 10:
					index = 9;
					break;

				default:
					index = 7;
					break;
			}

			break;

		case DOWNSTART:
			switch(speed) {
				case 1:
					index = 4;
					break;

				case 5:
					index = 2;
					break;

				case 10:
					index = 1;
					break;

				default:
					index = 2;
					break;
			}

			break;

		case LEFTSTART:
			switch(speed) {
				case 1:
					index = 4;
					break;

				case 5:
					index = 2;
					break;

				case 10:
					index = 1;
					break;

				default:
					index = 2;
					break;
			}

			break;

		case RIGHTSTART:
			switch(speed) {
				case 1:
					index = 6;
					break;

				case 5:
					index = 7;
					break;

				case 10:
					index = 9;
					break;

				default:
					index = 7;
					break;
			}

			break;

		case DDECSTART:
			switch(speed) {
				case 1:
					index = 4;
					break;

				case 5:
					index = 2;
					break;

				case 10:
					index = 1;
					break;

				default:
					index = 2;
					break;
			}

			break;

		case DADDSTART:
			switch(speed) {
				case 1:
					index = 6;
					break;

				case 5:
					index = 7;
					break;

				case 10:
					index = 9;
					break;

				default:
					index = 7;
					break;
			}

			break;

		default:
			index = 7;
			DEBUG(DL_DEBUG, "[HE50CameraIndex] Error way = %d \n", way);
			break;
	}

	return (index);
}

/*
##	Camera control up start
##  fd ------handle operation
##  addr-----Camera operation address
##	speed----Camera operation speed
##
*/
int CCtrlUpStart(int fd, int addr, int speed)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, vSpeed, hSpeed, size, ret = -1;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[UPSTART].comm);

	if(strcmp((char *)gRemote.ptzName, "/etc/camera/AW-HE50.ini") == 0) {
		speed = HE50CameraIndex(speed, UPSTART);
		/*vSpeed=5    5=8*/
		vSpeed = gRemote.VSSlid.speed[speed];
		/*vSpeed=5    5=8*/
		hSpeed = gRemote.HSSlid.speed[speed];
		/*Address1='#'*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, hSpeed, vSpeed);
		DEBUG(DL_DEBUG, "speed=%d Vspeed=%x  hSpeed = %x addr = %d nAddr = %x\n", speed,
		      vSpeed, hSpeed, addr, nAddr);
	} else {
		/*vSpeed=5    5=8*/
		vSpeed = gRemote.VSSlid.speed[speed];
		/*Address1=81*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, vSpeed, vSpeed);
	}

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlUpStart() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlUpStart()->SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlUpStart() End  size = %d\n\n", size);
	return 0;

}

/*
##	Camera control UP stop
##  fd ------handle operation
##  addr-----Camera operation address
##
*/
int CCtrlUpStop(int fd, int addr)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, size, ret = -1;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[UPSTOP].comm);

	if(strcmp((char *)gRemote.ptzName, "/etc/camera/AW-HE50.ini") == 0) {
		/*Address1=23*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0x35, 0x30);
	} else {
		/*Address1=81*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0, 0);
	}

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlUpStop() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlUpStop() SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlUpStop() End\n");
	return 0;

}


/*
##	Remote control down start
##  fd ------handle operation
##  addr-----Camera address operation
##	speed----Camera trans speed
##
*/
int CCtrlDownStart(int fd, int addr, int speed)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, vSpeed, hSpeed, size, ret = -1;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[DOWNSTART].comm);

	if(strcmp((char *)gRemote.ptzName, "/etc/camera/AW-HE50.ini") == 0) {
		speed = HE50CameraIndex(speed, DOWNSTART);
		/*vSpeed=5    5=8*/
		vSpeed = gRemote.VSSlid.speed[speed];
		/*vSpeed=5    5=8*/
		hSpeed = gRemote.HSSlid.speed[speed];
		/*Address1='#'*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, hSpeed, vSpeed);
	} else {
		/*vSpeed=5*/
		vSpeed = gRemote.VSSlid.speed[speed];
		/*Address1=81*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0, vSpeed);
	}

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlDownStart() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlDownStart()->SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlDownStart() End\n");
	return 0;

}

/*
##	Remote control down Stop
##  fd ------handle operation
##  addr-----Camera address operation
##
*/
int CCtrlDownStop(int fd, int addr)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, size, ret = -1;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[DOWNSTOP].comm);

	if(strcmp((char *)gRemote.ptzName, "/etc/camera/AW-HE50.ini") == 0) {
		/*Address1=23*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0x35, 0x30);
	} else {
		/*Address1=81*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0, 0);
	}

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlUpStop() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlUpStop() SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlDownStop() End\n");
	return 0;

}


/*
##
##	Remote control Left Start
##  fd ------handle operation
##  addr-----Camera address operation
##	speed----Camera trans speed
##
##
*/
int CCtrlLeftStart(int fd, int addr, int speed)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, hSpeed, vSpeed, size, ret = -1;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[LEFTSTART].comm);

	if(strcmp((char *)gRemote.ptzName, "/etc/camera/AW-HE50.ini") == 0) {
		speed = HE50CameraIndex(speed, LEFTSTART);
		/*vSpeed=5    5=8*/
		vSpeed = gRemote.VSSlid.speed[speed];
		/*vSpeed=5    5=8*/
		hSpeed = gRemote.HSSlid.speed[speed];
		/*Address1='#'*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, hSpeed, vSpeed);
	} else {
		/*hSpeed=5*/
		hSpeed = gRemote.HSSlid.speed[speed];
		/*Address1=81*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, hSpeed, 0);
	}

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlLeftStart() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlLeftStart()->SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlLeftStart() End\n");
	return 0;

}

/*
##
##	Remote control Left Stop
##  fd ------handle operation
##  addr-----Camera address operation
##
*/
int CCtrlLeftStop(int fd, int addr)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, size, ret = -1;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[LEFTSTOP].comm);

	if(strcmp((char *)gRemote.ptzName, "/etc/camera/AW-HE50.ini") == 0) {
		/*Address1=23*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0x35, 0x30);
	} else {
		/*Address1=81*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0, 0);
	}

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlLeftStop() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlLeftStop() SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlLeftStop() End\n");
	return 0;

}

/*
##
##	Remote control right Start
##  fd ------handle operation
##  addr-----Camera address operation
##	speed----Camera trans speed
##
*/
int CCtrlRightStart(int fd, int addr, int speed)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, hSpeed, vSpeed, size, ret = -1;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[RIGHTSTART].comm);

	if(strcmp((char *)gRemote.ptzName, "/etc/camera/AW-HE50.ini") == 0) {
		speed = HE50CameraIndex(speed, RIGHTSTART);
		/*vSpeed=5    5=8*/
		vSpeed = gRemote.VSSlid.speed[speed];
		/*vSpeed=5    5=8*/
		hSpeed = gRemote.HSSlid.speed[speed];
		/*Address1='#'*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, hSpeed, vSpeed);
	} else {
		/*hSpeed=5*/
		hSpeed = gRemote.HSSlid.speed[speed];
		/*Address1=81*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, hSpeed, 0);
	}

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlRightStart() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlRightStart()->SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlRightStart() End\n");
	return 0;

}

/*
##
##	Remote control Right Stop
##  fd ------handle operation
##  addr-----Camera address operation
##
*/
int CCtrlRightStop(int fd, int addr)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, size, ret = -1;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[RIGHTSTOP].comm);

	if(strcmp((char *)gRemote.ptzName, "/etc/camera/AW-HE50.ini") == 0) {
		/*Address1=23*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0x35, 0x30);
	} else {
		/*Address1=81*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0, 0);
	}

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlRightStop() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlRightStop() SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlRightStop() End\n");
	return 0;

}

/*
##
##	Remote control Zoom out  start
##  fd ------handle operation
##  addr-----Camera address operation
##
*/
int CCtrlZoomAddStart(int fd, int addr)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, size, ret = -1;
	int speed = 5, vSpeed, hSpeed;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[DADDSTART].comm);

	if(strcmp((char *)gRemote.ptzName, "/etc/camera/AW-HE50.ini") == 0) {
		speed = HE50CameraIndex(speed, DADDSTART);
		/*vSpeed=5    5=8*/
		vSpeed = gRemote.VSSlid.speed[speed];
		/*vSpeed=5    5=8*/
		hSpeed = gRemote.HSSlid.speed[speed];
		/*Address1='#'*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, hSpeed, vSpeed);
	} else {
		/*Address1=81*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0, 0);
	}

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlZoomAddStart() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlZoomAddStart() SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlZoomAddStart() End\n");
	return 0;

}


/*
##	Remote control Zoom out Stop
##  fd ------handle operation
##  addr-----Camera address operation
##
*/
int CCtrlZoomAddStop(int fd, int addr)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, size, ret = -1;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[DADDSTOP].comm);

	if(strcmp((char *)gRemote.ptzName, "/etc/camera/AW-HE50.ini") == 0) {
		/*Address1=23*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0x35, 0x30);
	} else {
		/*Address1=81*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0, 0);
	}

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlZoomAddStart() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlZoomAddStart() SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlZoomAddStart() End\n");
	return 0;

}

/*
##
##	Remote control Zoom In start
##  fd ------handle operation
##  addr-----Camera address operation
##
*/
int CCtrlZoomDecStart(int fd, int addr)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, size, ret = -1;
	int vSpeed, hSpeed, speed = 5;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[DDECSTART].comm);

	if(strcmp((char *)gRemote.ptzName, "/etc/camera/AW-HE50.ini") == 0) {
		speed = HE50CameraIndex(speed, DDECSTART);
		/*vSpeed=5    5=8*/
		vSpeed = gRemote.VSSlid.speed[speed];
		/*vSpeed=5    5=8*/
		hSpeed = gRemote.HSSlid.speed[speed];
		/*Address1='#'*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, hSpeed, vSpeed);
	} else {
		/*Address1=81*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0, 0);
	}

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlZoomAddStart() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlZoomAddStart() SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlZoomAddStart() End\n");
	return 0;

}


/*
##
##	Remote control Zoom In Stop
##  fd ------handle operation
##  addr-----Camera address operation
##
*/
int CCtrlZoomDecStop(int fd, int addr)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, size, ret = -1;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[DDECSTOP].comm);

	if(strcmp((char *)gRemote.ptzName, "/etc/camera/AW-HE50.ini") == 0) {
		/*Address1=23*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0x35, 0x30);
	} else {
		/*Address1=81*/
		nAddr  = gRemote.Addr.addr[addr];
		size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0, 0);
	}

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlZoomAddStart() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlZoomAddStart() SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlZoomAddStart() End\n");
	return 0;

}

int CCtrlPositionPreset(int fd, int addr, int position)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, size, ret = -1;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[POSITIONPRESET].comm);
	/*Address1=81*/
	nAddr  = gRemote.Addr.addr[addr];

	size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0, 0);

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlPositionPreset() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	pCode[5] = position; //第5位表示预置位
	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlPositionPreset() SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlPositionPreset() End\n");
	return 0;

}
int CCtrlSetPositionPreset(int fd, int addr, int position)
{
	unsigned char pCode[16];
	char strCode[200];
	char strKey[64];
	char strtmp[8];
	int nAddr, size, ret = -1;

	memset(pCode, 0, 16);
	memset(strCode, 0, 200);
	memset(strKey, 0, 64);
	memset(strtmp, 0, 8);

	/*Up_Start=*/
	strcpy(strCode, gRemote.CCode[POSITIONPRESETSET].comm);
	/*Address1=81*/
	nAddr  = gRemote.Addr.addr[addr];
	size = CCtrlCodeAnalysis(pCode, strCode, nAddr, 0, 0);

	if(size <= 0 || fd == -1) {
		DEBUG(DL_ERROR, "CCtrlPositionPreset() CCtrlCodeAnalysis Error\n");
		return -1;
	}

	pCode[5] = position; //第5位表示预置位
	ret = SendDataToCom(fd, pCode, size);

	if(ret == -1) {
		DEBUG(DL_ERROR, "CCtrlPositionPreset() SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "CCtrlPositionPreset() End\n");
	return 0;

}
/*Trans parity*/
int TransParity(int parity)
{
	int ch = 'N';

	switch(parity)	{
		case 0:
			ch = 'N';			//none
			break;

		case 1:
			ch = 'O';    		//odd
			break;

		case 2:
			ch = 'E';			//even
			break;

		default:
			DEBUG(DL_ERROR, "TransParity() parity:%d  Error\n", parity);
			break;
	}

	return ch;
}


/*Initial remote config file*/
int InitRemoteStruct(int index)
{
	int cnt, ret;
	char filename[50], buf[60], buf1[20];

	if((index < 0) && (index > PROTNUM - 1)) {
		index = 0;
	}

	memset(&gRemote, 0, sizeof(gRemote));
	/*Camera No.*/
	strcpy(filename, "/etc/camera/");
	strcat(filename, FarCtrlList[index]);
	strcat(filename, ".ini");

	strcpy((char *)gRemote.ptzName, filename);

	DEBUG(DL_DEBUG, "Config File: %s\n", filename);

	/*Speed control*/
	for(cnt = 1; cnt < SLID_MAX_NUM; cnt++) {
		memset(buf, 0, sizeof(buf));
		memset(buf1, 0, sizeof(buf1));
		sprintf(buf1, "%d", cnt);
		ret = ConfigGetKey(filename, "HSeepSlid", buf1, buf);

		if(ret == -10) {    //file no exist
			DEBUG(DL_ERROR, "file no exist %s\n", gRemote.ptzName);
			return -1;
		}

		if(ret != 0) {
			continue;
		}

		gRemote.HSSlid.speed[cnt] = strtol(buf, 0, 16);
		memset(buf, 0, sizeof(buf));
		memset(buf1, 0, sizeof(buf1));
		sprintf(buf1, "%d", cnt);
		ret = ConfigGetKey(filename, "VSeepSlid", buf1, buf);

		if(ret != 0) {
			continue;
		}

		gRemote.VSSlid.speed[cnt] = strtol(buf, 0, 16);

	}

	/*operation addr*/
	for(cnt = 1; cnt < ADDR_MAX_NUM ; cnt++)  {
		memset(buf, 0, sizeof(buf));
		memset(buf1, 0, sizeof(buf1));
		sprintf(buf1, "Address%d", cnt);
		ret = ConfigGetKey(filename, "Address", buf1, buf);

		if(ret != 0) {
			continue;
		}

		gRemote.Addr.addr[cnt] = strtol(buf, 0, 16);;
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "Up_Start", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[UPSTART].comm, buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "Up_Stop", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[UPSTOP].comm, buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "Down_Start", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[DOWNSTART].comm, buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "Down_Stop", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[DOWNSTOP].comm, buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "Left_Start", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[LEFTSTART].comm, buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "Left_Stop", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[LEFTSTOP].comm, buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "Right_Start", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[RIGHTSTART].comm, buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "Right_Stop", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[RIGHTSTOP].comm, buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "DADD_Start", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[DADDSTART].comm, buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "DADD_Stop", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[DADDSTOP].comm, buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "DDEC_Start", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[DDECSTART].comm, buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "DDEC_Stop", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[DDECSTOP].comm, buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "PositionPreset", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[POSITIONPRESET].comm, buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "CtrlCode", "PositionPresetSet", buf);

	if(ret == 0) {
		strcpy(gRemote.CCode[POSITIONPRESETSET].comm, buf);
	}

	DEBUG(DL_DEBUG, "[InitRemoteStruct] ################ PositionPresetSet [%s]\n", buf);
	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "Comm", "StopBit", buf);

	if(ret == 0) {
		gRemote.comm.stopbits = atoi(buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "Comm", "DataBit", buf);

	if(ret == 0) {
		gRemote.comm.databits = atoi(buf);
	}

	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "Comm", "Baud", buf);

	if(ret == 0) {
		gRemote.comm.baud = atoi(buf);
	}


	memset(buf, 0, sizeof(buf));
	ret = ConfigGetKey(filename, "Comm", "Parity", buf);

	if(ret == 0) {
		gRemote.comm.parity = TransParity(atoi(buf));
	}

	return 0;

}

/*Camera control initial*/
int CameraCtrlInit(int com)
{
	int ret = -1;
	int fd;
	int baudrate = 9600, databits = 8;
	int stopbits = 1, parity = 'N';

	baudrate = gRemote.comm.baud;
	databits = gRemote.comm.databits;
	parity = gRemote.comm.parity;
	stopbits = gRemote.comm.stopbits;

	fd = InitPort(com, baudrate, databits, stopbits, parity);

	if(fd <= 0) {
		return -1;
	}

	ret = InitCamera(fd);

	if(ret != 0) {
		return -1;
	}

	return fd;
}


void Print(BYTE *data, int len)
{
	int i;

	DEBUG(DL_DEBUG, "\n");

	for(i = 0; i < len; i++) {
		DEBUG(DL_DEBUG, "%2x  ", data[i]);
	}

	DEBUG(DL_DEBUG, "\n");
}
/*
** Remote
**Send MSG_FARCTRL Message
**MSG_FARCTRL message header
**4BYTES    for  Type
**4BYTES    fpr  Speed
Type
2            UP Start
16          UP Stop
3            DOWN Start
17          DOWN Stop
0            LEFT Start
14          LEFT Stop
1            RIGHT Start
15          RIGHT Stop
8            FOCUS ADD Start
22          FOCUS ADD Stop
9            FOCUS DEC Start
23          FOCUS DEC Stop
Speed
1            LOW
5            MID
10          HIGH

 */
void FarCtrlCamera(int dsp, unsigned char *data, int len)
{
	int type, speed;
	int fd;
	int caddr = 1;

	Print(data, len);
	type = (int)(data[0] | data[1] << 8 | data[2] << 16 | data[3] << 24);
	speed = (int)(data[4] | data[5] << 8 | data[6] << 16 | data[7] << 24);

	if(len < 12) {
		caddr = 1;
	} else {
		caddr = (int)(data[8] | data[9] << 8 | data[10] << 16 | data[11] << 24);
	}

	if(caddr < 1 || caddr > 100) {
		caddr = 1;
	}

	DEBUG(DL_DEBUG, "type:%d    len:%d\n", type, len);
	DEBUG(DL_DEBUG, "speed:%d\n", speed);
	PRINTF("the camera addr = %d\n", caddr);

	if(gRemoteFD <= 0) {
		DEBUG(DL_ERROR, "Remote Port Open Failed\n");
	}

	fd = gRemoteFD;

	switch(type) {
		case 2:		//UP Start
			CCtrlUpStart(fd, caddr, speed);
			break;

		case 16:	//UP stop
			CCtrlUpStop(fd, caddr);
			break;

		case 3:		//DOWN start
			CCtrlDownStart(fd, caddr, speed);
			break;

		case 17:	//DOWN stop
			CCtrlDownStop(fd, caddr);
			break;

		case 0:		// LEFT Start
			CCtrlLeftStart(fd, caddr, speed);
			break;

		case 14:	//LEFT Stop
			CCtrlLeftStop(fd, caddr);
			break;

		case 1:		//RIGHT Start
			CCtrlRightStart(fd, caddr, speed);
			break;

		case 15:    //RIGHT Stop
			CCtrlRightStop(fd, caddr);
			break;

		case 8:		//FOCUS ADD Start
			CCtrlZoomAddStart(fd, caddr);
			break;

		case 22:	//FOCUS ADD Stop
			CCtrlZoomAddStop(fd, caddr);
			break;

		case 9:		//FOCUS DEC Start
			CCtrlZoomDecStart(fd, caddr);
			break;

		case 23:	//FOCUS DEC Stop
			CCtrlZoomDecStop(fd, caddr);
			break;

		case 10:	//预置位，speed表示预置位号
			CCtrlPositionPreset(fd, caddr, speed);
			break;

		case 11:	//设置预置位，speed表示预置位号
			CCtrlSetPositionPreset(fd, caddr, speed);
			break;

		case 12://设置自动跟踪模块的手动和自动模式0自动1手动
			SetTrackStatus(fd, speed);
			break;

		default:
			DEBUG(DL_ERROR, "FarCtrlCamera() command Error\n");
			break;
	}

}


/*###########################################*/
#if 0
/*摄像头的控制*/
int CameraControl()
{
	int ret = -1;
	int fd;
	int baudrate = 9600, databits = 8;
	int stopbits = 1, parity = 'N';

	baudrate = gRemote.comm.baud;
	databits = gRemote.comm.databits;
	parity = gRemote.comm.parity;
	stopbits = gRemote.comm.stopbits;


	Printf("Initial succeed\n");
	fd = InitPort(PORT_COM2, baudrate, databits, stopbits, parity);
	ret = InitCamera(fd);

	if(ret != 0) {
		return -1;
	}

	/*	sleep(1);
		CCtrlUpStart(fd,1,10);
		sleep(10);
		CCtrlUpStop(fd,1);
		usleep(100);

		CCtrlDownStart(fd,1,10);
		sleep(10);
		CCtrlDownStop(fd,1);
		usleep(100);
	*/
	CCtrlLeftStart(fd, 1, 10);
	sleep(10);
	CCtrlLeftStop(fd, 1);
	/*	usleep(100);
		CCtrlRightStart(fd,1,10);
		sleep(20);
		CCtrlRightStop(fd,1);
		usleep(100);
		CCtrlZoomAddStart(fd,1);
		sleep(10);
		CCtrlZoomAddStop(fd,1);
		usleep(100);
		CCtrlZoomDecStart(fd,1);
		sleep(20);
		CCtrlZoomDecStop(fd,1);
		usleep(100);*/
	return 0;
}

#endif

