#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <asm/types.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>

#include "log_common.h"


#define BUFSIZE 8192
struct route_info {
	u_int dstAddr;
	u_int srcAddr;
	u_int gateWay;
	char ifName[IF_NAMESIZE];
};
int readNlSock(int sockFd, char *bufPtr, int seqNum, int pId)
{
	struct nlmsghdr *nlHdr;
	int readLen = 0, msgLen = 0;

	do {
		/* Recieve response from the kernel */
		if((readLen = recv(sockFd, bufPtr, BUFSIZE - msgLen, 0)) < 0) {
			perror("SOCK READ: ");
			return -1;
		}

		nlHdr = (struct nlmsghdr *)bufPtr;

		/* Check if the header is valid */
		if((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR)) {
			perror("Error in recieved packet");
			return -1;
		}

		/* Check if the its the last message */
		if(nlHdr->nlmsg_type == NLMSG_DONE) {
			break;
		} else {
			/* Else move the pointer to buffer appropriately */
			bufPtr += readLen;
			msgLen += readLen;
		}

		/* Check if its a multi part message */
		if((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0) {
			/* return if its not */
			break;
		}
	} while((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));

	return msgLen;
}
/* For printing the routes. */
void printRoute(struct route_info *rtInfo)
{
	char tempBuf[512];

	/* Print Destination address */
	if(rtInfo->dstAddr != 0) {
		strcpy(tempBuf, (char *)inet_ntoa(rtInfo->dstAddr));
	} else {
		sprintf(tempBuf, "*.*.*.*\t");
	}

	fprintf(stdout, "%s\t", tempBuf);

	/* Print Gateway address */
	if(rtInfo->gateWay != 0) {
		strcpy(tempBuf, (char *)inet_ntoa(rtInfo->gateWay));
	} else {
		sprintf(tempBuf, "*.*.*.*\t");
	}

	fprintf(stdout, "%s\t", tempBuf);

	/* Print Interface Name*/
	fprintf(stdout, "%s\t", rtInfo->ifName);

	/* Print Source address */
	if(rtInfo->srcAddr != 0) {
		strcpy(tempBuf, (char *)inet_ntoa(rtInfo->srcAddr));
	} else {
		sprintf(tempBuf, "*.*.*.*\t");
	}

	fprintf(stdout, "%s\n", tempBuf);
}


/* For parsing the route info returned */
void parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo, char *gateway)
{
	struct rtmsg *rtMsg;
	struct rtattr *rtAttr;
	int rtLen;
	char *tempBuf = NULL;

	tempBuf = (char *)malloc(100);
	rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);

	/* If the route is not for AF_INET or does not belong to main routing table
	then return. */
	if((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN)) {
		return;
	}


	/* get the rtattr field */
	rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
	rtLen = RTM_PAYLOAD(nlHdr);

	for(; RTA_OK(rtAttr, rtLen); rtAttr = RTA_NEXT(rtAttr, rtLen)) {
		switch(rtAttr->rta_type) {
			case RTA_OIF:
				if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);
				break;

			case RTA_GATEWAY:
				rtInfo->gateWay = *(u_int *)RTA_DATA(rtAttr);
				break;

			case RTA_PREFSRC:
				rtInfo->srcAddr = *(u_int *)RTA_DATA(rtAttr);
				break;

			case RTA_DST:
				rtInfo->dstAddr = *(u_int *)RTA_DATA(rtAttr);
				break;
		}
	}

	if(strstr((char *)inet_ntoa(rtInfo->dstAddr), "0.0.0.0")) {
		sprintf(gateway, (char *)inet_ntoa(rtInfo->gateWay));
	}

	//printRoute(rtInfo);
	free(tempBuf);
	return;
}

//获取默认网关
int get_gateway(char *gateway)
{
	struct nlmsghdr *nlMsg;
	struct rtmsg *rtMsg;
	struct route_info *rtInfo;
	char msgBuf[BUFSIZE];
	struct in_addr	addr ;
	unsigned int 	value;
	unsigned int 	dwAddr, dwNetmask, dwGateWay;

	int sock, len, msgSeq = 0;
	char buff[1024];


	/* Create Socket */
	if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) <= 0) {
		perror("Socket Creation: ");
	}

	/* Initialize the buffer */
	memset(msgBuf, 0, BUFSIZE);

	/* point the header and the msg structure pointers into the buffer */
	nlMsg = (struct nlmsghdr *)msgBuf;
	rtMsg = (struct rtmsg *)NLMSG_DATA(nlMsg);

	/* Fill in the nlmsg header*/
	nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.
	nlMsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .

	nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
	nlMsg->nlmsg_seq = msgSeq++; // Sequence of the message packet.
	nlMsg->nlmsg_pid = getpid(); // PID of process sending the request.

	/* Send the request */
	if(send(sock, nlMsg, nlMsg->nlmsg_len, 0) <= 0) {
		PRINTF("Write To Socket Failed...\n");
		return -1;
	}

	/* Read the response */
	if((len = readNlSock(sock, msgBuf, msgSeq, getpid())) <= 0) {
		PRINTF("Read From Socket Failed...\n");
		return -1;
	}

	/* Parse and print the response */
	rtInfo = (struct route_info *)malloc(sizeof(struct route_info));

	for(; NLMSG_OK(nlMsg, len); nlMsg = NLMSG_NEXT(nlMsg, len)) {
		memset(rtInfo, 0, sizeof(struct route_info));
		parseRoutes(nlMsg, rtInfo, gateway);
	}

	inet_aton(gateway, &addr);
	memcpy(&value, &addr, 4);
	dwGateWay = value;

	free(rtInfo);
	close(sock);
	return dwGateWay;
}

//获取本机ip地址
unsigned int GetIPaddr(char *interface_name)
{
	int s;
	unsigned int ip;

	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket");
		return -1;
	}

	struct ifreq ifr;

	strcpy(ifr.ifr_name, interface_name);

	if(ioctl(s, SIOCGIFADDR, &ifr) < 0) {
		perror("ioctl");
		return -1;
	}

	struct sockaddr_in *ptr;

	ptr = (struct sockaddr_in *) &ifr.ifr_ifru.ifru_addr;

	PRINTF("        IP:%s\n", inet_ntoa(ptr->sin_addr));

	memcpy(&ip, &ptr->sin_addr, 4);

	return ip;
}

//获取本机子网掩码
unsigned int GetNetmask(char *interface_name)
{
	int s;
	unsigned int ip;

	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket");
		return -1;
	}

	struct ifreq ifr;

	strcpy(ifr.ifr_name, interface_name);

	if(ioctl(s, SIOCGIFNETMASK, &ifr) < 0) {
		perror("ioctl");
		return -1;
	}

	struct sockaddr_in *ptr;

	ptr = (struct sockaddr_in *) &ifr.ifr_ifru.ifru_netmask;

	PRINTF("        Netmask:%s\n", inet_ntoa(ptr->sin_addr));

	memcpy(&ip, &ptr->sin_addr, 4);

	return ip;
}

//获取组播ip
unsigned int GetBroadcast(char *interface_name)
{
	int s;
	unsigned int ip;

	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket");
		return -1;
	}

	struct ifreq ifr;

	strcpy(ifr.ifr_name, interface_name);

	if(ioctl(s, SIOCGIFBRDADDR, &ifr) < 0) {
		perror("ioctl");
		return -1;
	}

	struct sockaddr_in *ptr;

	ptr = (struct sockaddr_in *) &ifr.ifr_ifru.ifru_broadaddr;

	PRINTF("        Broadcast:%s\n", inet_ntoa(ptr->sin_addr));

	memcpy(&ip, &ptr->sin_addr, 4);

	return ip;

}


#if 0
int main()
{
	char gateway[255] = {0};
	get_gateway(gateway);
	printf("Gateway:%s\n", gateway);
}
#endif


