       /* cgi.c */
#include "StdAfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>		/* time() */
#include <fcntl.h>
#include <sys/stat.h>
#define LINUX
#ifdef LINUX
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#endif
//

#include "htmllib.h"
#include "cgic.h"
#include "Tools.h"
#include "cgi.h"
#include "weblib.h"
#include "../../middle_control.h"
#include "webTcpCom.h"
#include "app_web_stream_output.h"
#include "stream_output_struct.h"
#include "app_web_stream_output.h"

#define ACTION_START 100
#define ACTION_UPLOADFILE  ACTION_START+10
#define ACTION_LOGIN  ACTION_START+20
#define ACTION_QUERYSYSPARAM  ACTION_START+30
#define ACTION_CHANGEPWD  ACTION_START+40
#define ACTION_NETWORKSET  ACTION_START+50
#define ACTION_UPDATEVIDEOPARAM  ACTION_START+60
#define ACTION_QUERYAVPARAM  ACTION_START+70
#define ACTION_UPDATEAUDIOPARAM  ACTION_START+80
#define ACTION_TEXT_SET  ACTION_START+90
#define ACTION_LOGOUT  ACTION_START+100

//added by Adams begin.
#define PAGE_LEFTMENU_SYSPARAMS_SHOW 301
#define PAGE_LEFTMENU_SYSSET_SHOW 302
#define PAGE_SYSINFO_SHOW 303
#define PAGE_INPUT_SHOW 304
#define PAGE_VIDEO_SHOW 305
#define PAGE_AUDIO_SHOW 306
#define PAGE_OUTPUT_SHOW 307
#define PAGE_CAPTIONLOGO_SHOW 308
#define PAGE_REMOTECTRL_SHOW 309
#define PAGE_NETWORK_SHOW 310
#define PAGE_MODIFYPASSWORD_SHOW 311
#define PAGE_OTHERSET_SHOW 312
#define PAGE_SYSUPGRADE_SHOW 313
#define PAGE_RESETDEFAULT_SHOW 314
#define PAGE_INPUTDETAILS_SHOW 315
#define PAGE_OUTPUTADVANCEDSET_SHOW 316
#define PAGE_OUTPUTRTSPSET_SHOW 317
#define PAGE_VIDEOADVANCEDSET_SHOW 318
#define PAGE_ADDOUTPUT_SHOW 319


#define ACTION_VIDEO_SET 401
#define ACTION_OUTPUT_SET 402
#define ACTION_OUTPUTADD_SET 403
#define ACTION_OUTPUTUPDATE_GET 404
#define ACTION_OUTPUTUPDATE_SET 405
#define ACTION_NETWORK_SET 406
#define ACTION_INPUT_SET 407
#define ACTION_INPUTDETAILS_GET 408
#define ACTION_ADJUSTSCREEN_SET 409
#define ACTION_AUDIO_SET 410
#define ACTION_CAPTIONLOGO_SET 411
#define PAGE_REMOTECTRL_SET 412
#define ACTION_PTZ_CTRL 413
#define ACTION_VIDEOADVANCEDSET_GET 414
#define ACTION_VIDEOADVANCEDSET_SET 415
#define ACTION_OUTPUTRTSPSET_GET 416
#define ACTION_OUTPUTRTSPSET_SET 417
#define ACTION_SETDEFAULT_SET 418
#define ACTION_UPLOADLOGOPIC_SET 419
#define ACTION_DOWNLOAD_SDP 420

#define RESULT_SUCCESS 1
#define RESULT_FAILURE 2
#define RESULT_MUSTMULTIIP 3

//added by Adams end.


#define PAGE_LOGIN_SHOW 201
#define PAGE_TIMEOUT_SHOW 202
#define PAGE_TOP_SHOW 203
#define PAGE_DOWN_SHOW 204
#define PAGE_MAIN_SHOW 205
#define PAGE_SYSTEM_INFO 206
#define PAGE_SYSTEM_SHOW 207
#define PAGE_PARAMETER_SHOW 208
#define SAVE_PLAY_CFG 209
#define TVOD_FILE 210
#define TVOD_OPER 211
#define SAVE_SHOW_MODE 212
#define SAVE_OUTPUT 213

#define RESTART_SHOW 214
#define RESTART_SYS 215
#define SERVER_SHOW 216
#define SERVER_LIST 217
#define SERVER_ADD_SHOW 218
#define SERVER_ADD 219
#define SERVER_MOD_SHOW 220
#define SERVER_MOD 221
#define SERVER_DEL 222
#define SERVER_LIST_SHOW 223
#define ACTION_SYNC 224
#define ACTION_SAVEUPDATE 225
#define UPDATE_MULTI_SET 31
#define SCREEN_CHANGE 152
#define SCREEN_GREEN 151
#define YTKZ_PROTO 226
#define YTKZ 227
#define UPDATE_LANGUAGE 24
#define UPDATE_HIDDEN_VALUE 300
//#define SETPHY	228
//#define GETPHY	229

#define CGI_NAME "encoder.cgi"
#define BUFFER_LEN					1024
#define MAXFILE_NUM					1024
#define FILENAME_MAXLEN				200

#if (defined(DSS_ENC_1200) || defined(DSS_ENC_1260))

#define UPDATEFILEHEAD "7E7E7E7E48454E43"
#else
#define UPDATEFILEHEAD "7E7E7E7E31313030"
#endif
#define BufferLen 500
#define UPFILEHEADLEN 8

#define WEBVERSION "1.0.7"



char sys_password[100];
char sys_webpassword[100];
char sys_timeout[100];
char sys_language[100];

char* trim(char * str)
{
	int len=0;
	int i=0;
	len=strlen(str);
	i=len-1;
	for(;i>0;i--)
	{
		if(str[i]==' '||str[i]==0x0A)
			continue;
		else
			break;
	}
	str[i+1]='\0';
	return str;
}

static void gettexttodisplay(textinfo* info)
{
	strcpy(info->msgtext,"");
	info->xpos=0;
	info->ypos=0;
	info->showtime=1;

}
//0成功
//-1失败
static int loginaction()
{
	char username[100] = {0};
    char password[100] = {0};
	//char strTime[100] = {0};
	cgiFormString("username", username, sizeof(username));
	cgiFormString("password", password, sizeof(password));

	if((!strcmp(username,"admin")&&!strcmp(password,sys_password))||(!strcmp(username,"operator")&&!strcmp(password,sys_webpassword)))
	{
		long now;
		now = (long)time(NULL);
		showPage("./index.html",sys_language);
		fprintf(cgiOut,"<script type='text/javascript'>");
		fprintf(cgiOut, "setLanguageType('%s');", sys_language);
		fprintf(cgiOut,"</script>");
		cgiSetCookieUser(username);
		cgiSetCookienoTime("sessiontime",now);
		return 0;
	}
	else
	{
		char tmp[150]={0};
		getLanguageValue(sys_language,"UserOrPasswordError",tmp);
		forwardPage(CGI_NAME,PAGE_LOGIN_SHOW,tmp);
		return -1;
	}
}
//升级系统
static int updatesystems(void)
{
	FILE *targetFile;
	cgiFilePtr file;
	char name[128];
	//int retCode = 0;
	int got = 0;
	int t;
	char *tmpStr=NULL;
	char fileNameOnServer[64];
	char buffer[1024];
	int rec = 0;
	int flag = 0;


	char languagebuf[150]={0};
	getLanguageValue(sys_language,"uploadFileFail",languagebuf);
	if (cgiFormFileName("file", name, sizeof(name)) !=cgiFormSuccess)
	{
		//forwardPage(CGI_NAME,207,languagebuf);
		fprintf(cgiOut,"<script type='text/javascript'>alert('%s');parent.closeUploading();</script>",languagebuf);
		return -1;
	}

	t=-1;
	//从路径名解析出用户文件名
	while(1){
		tmpStr=strstr(name+t+1,"\\");
		if(NULL==tmpStr)
			tmpStr=strstr(name+t+1,"/");//if "\\" is not path separator, try "/"
		if(NULL!=tmpStr)
			t=(int)(tmpStr-name);
		else
			break;
	}

	if(strstr(name,".bin")==NULL||(name-strstr(name,".bin"))!=(4-strlen(name)))
	{
		memset(languagebuf,0,150);
		getLanguageValue(sys_language,"binfile",languagebuf);
		//forwardPage(CGI_NAME,207,languagebuf);
		fprintf(cgiOut,"<script type='text/javascript'>alert('%s');parent.closeUploading();</script>",languagebuf);
		return -1;
	}
	//根据表单中的值打开文件
	if ((rec=cgiFormFileOpen("file", &file)) != cgiFormSuccess) {
		//memset(languagebuf,0,150);
		//sprintf(languagebuf,"%d",rec);
		//forwardPage(CGI_NAME,207,languagebuf);
		fprintf(cgiOut,"<script type='text/javascript'>alert('%s');parent.closeUploading();</script>",languagebuf);
		return -1;
	}
#ifdef LINUX
	strcpy(fileNameOnServer,"/opt/reach/update.tgz");
#else
	strcpy(fileNameOnServer,"D:\\update\\update.bin");
#endif
	targetFile=fopen(fileNameOnServer,"wb+");
	if(targetFile==NULL){
		//forwardPage(CGI_NAME,207,languagebuf);
		fprintf(cgiOut,"<script type='text/javascript'>alert('%s');parent.closeUploading();</script>",languagebuf);
		return -1;
	}

	while (cgiFormFileRead(file, buffer, BufferLen, &got) ==cgiFormSuccess){
		if(got>0){
			//效验文件合法
			if(flag == 0){
				char tmpsync[20]={0};
				sprintf(tmpsync,"%02X%02X%02X%02X%02X%02X%02X%02X",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);
				if(strcmp(UPDATEFILEHEAD,tmpsync))
				{
					memset(languagebuf,0,150);
					getLanguageValue(sys_language,"uploadFileFail",languagebuf);
					//forwardPage(CGI_NAME,207,languagebuf);
					fprintf(cgiOut,"<script type='text/javascript'>alert('%s');parent.closeUploading();</script>",languagebuf);
					return -1;
				}else{
					fwrite(buffer+8,1,got-8,targetFile);
				}
				flag = 1;
			}else
				fwrite(buffer,1,got,targetFile);
		}
	}
	cgiFormFileClose(file);
	fclose(targetFile);
	memset(languagebuf,0,150);
	getLanguageValue(sys_language,"uploadFileSuccess",languagebuf);
	rec = WebUpdateFile("/opt/reach/update.tgz");
	//forwardPage(CGI_NAME,207,languagebuf);
	fprintf(cgiOut,"<script type='text/javascript'>alert('%s');parent.closeUploading();</script>",languagebuf);

	return 0;
}
#if 0
//上传LOGO图片
static int updateLogoPic(void)
{
	FILE *targetFile;
	cgiFilePtr file;
	char name[128];
	//int retCode = 0;
	int got = 0;
	int t;
	char *tmpStr=NULL;
	char fileNameOnServer[64];
	char buffer[1024];
	int rec = 0;
	int flag = 0;


	char languagebuf[150]={0};
	getLanguageValue(sys_language,"uploadFileFail",languagebuf);
	if (cgiFormFileName("file", name, sizeof(name)) !=cgiFormSuccess)
	{
		//forwardPage(CGI_NAME,207,languagebuf);
		fprintf(cgiOut,"<script type='text/javascript'>alert('%s');</script>",languagebuf);
		return -1;
	}

	t=-1;
	//从路径名解析出用户文件名
	while(1){
		tmpStr=strstr(name+t+1,"\\");
		if(NULL==tmpStr)
			tmpStr=strstr(name+t+1,"/");//if "\\" is not path separator, try "/"
		if(NULL!=tmpStr)
			t=(int)(tmpStr-name);
		else
			break;
	}

	if(strstr(name,".png")==NULL||(name-strstr(name,".png"))!=(4-strlen(name)))
	{
		memset(languagebuf,0,150);
		getLanguageValue(sys_language,"pngfile",languagebuf);
		//forwardPage(CGI_NAME,207,languagebuf);
		fprintf(cgiOut,"<script type='text/javascript'>alert('%s');</script>",languagebuf);
		return -1;
	}
	//根据表单中的值打开文件
	if ((rec=cgiFormFileOpen("file", &file)) != cgiFormSuccess) {
		//forwardPage(CGI_NAME,207,languagebuf);
		fprintf(cgiOut,"<script type='text/javascript'>alert('%s');</script>",languagebuf);
		return -1;
	}
#ifdef LINUX
	strcpy(fileNameOnServer,"/opt/reach/logo.png");
#else
	strcpy(fileNameOnServer,"D:\\update\\logoimg\\logo.png");
#endif
	targetFile=fopen(fileNameOnServer,"wb+");
	if(targetFile==NULL){
		//forwardPage(CGI_NAME,207,languagebuf);
		fprintf(cgiOut,"<script type='text/javascript'>alert('%s');</script>",languagebuf);
		return -1;
	}

	while (cgiFormFileRead(file, buffer, BufferLen, &got) ==cgiFormSuccess){
		if(got>0){
			if(flag == 0){
			//	char tmpsync[20]={0};
				/*sprintf(tmpsync,"%02X%02X%02X%02X%02X%02X%02X%02X",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);
				if(strcmp(UPDATEFILEHEAD,tmpsync))
				{
					memset(languagebuf,0,150);
					getLanguageValue(sys_language,"uploadFileFail",languagebuf);
					//forwardPage(CGI_NAME,207,languagebuf);
					fprintf(cgiOut,"<script type='text/javascript'>alert('%s');</script>",languagebuf);
					return -1;
				}else{*/
					fwrite(buffer+8,1,got-8,targetFile);
				//}
				flag = 1;
			}else
				fwrite(buffer,1,got,targetFile);
		}
	}
	cgiFormFileClose(file);
	fclose(targetFile);
	memset(languagebuf,0,150);
	getLanguageValue(sys_language,"uploadFileSuccess",languagebuf);
	fprintf(cgiOut,"<script type='text/javascript'>alert('%s');</script>",languagebuf);
	return 0;
}

#endif
static int uploadLogo(void)
{
	FILE *targetFile;
	cgiFilePtr file;
	char name[256] ={0};
	//int retCode = 0;
	int got = 0;
	int t,len;
	char *tmpStr=NULL;
	char fileNameOnServer[64];
	char buffer[1024];
	int ret = 0;
	char tail[5];
//	int flag = 0;

	char tempname[512] = "/opt/reach/logo_bak.png"; 

	char languagebuf[150]={0};
	getLanguageValue(sys_language,"uploadFileFail",languagebuf);
	if (cgiFormFileName("file", name, sizeof(name)) !=cgiFormSuccess)
	{
		//forwardPage(CGI_NAME,207,languagebuf);
		fprintf(cgiOut,"<script type='text/javascript'>alert('%s');</script>",languagebuf);
		return -1;
	}

	t=-1;
	while(1){
		tmpStr=strstr(name+t+1,"\\");
		if(NULL==tmpStr)
			tmpStr=strstr(name+t+1,"/");//if "\\" is not path separator, try "/"
		if(NULL!=tmpStr)
			t=(int)(tmpStr-name);
		else
			break;
	}
	
		

	len=strlen(name);
	tail[0]=name[len-4];
	tail[1]=name[len-3];
	tail[2]=name[len-2];
	tail[3]=name[len-1];
	tail[4]='\0';
	
	if(strncmp(tail,".png",4) !=0  && strncmp(tail,".PNG",4) !=0 )
//	if(strstr(name,".png")==NULL ||(name-strstr(name,".png"))!=(4-strlen(name)))
	{	
		memset(languagebuf,0,150);
		getLanguageValue(sys_language,"pngfile",languagebuf);
		//forwardPage(CGI_NAME,207,languagebuf);
		fprintf(cgiOut,"<script type='text/javascript'>alert('%s');</script>",tail);
		fprintf(cgiOut,"<script type='text/javascript'>alert('%s');</script>",languagebuf);
		return -1;
	}
	if ((ret=cgiFormFileOpen("file", &file)) != cgiFormSuccess) {
		//memset(languagebuf,0,150);
		//sprintf(languagebuf,"%d",rec);
		//forwardPage(CGI_NAME,207,languagebuf);
		fprintf(cgiOut,"<script type='text/javascript'>alert('%s');</script>",languagebuf);
		return -1;
	}
#ifdef LINUX
	strcpy(fileNameOnServer,tempname);
#else
	strcpy(fileNameOnServer,"D:\\update\\update.bin");
#endif
	targetFile=fopen(fileNameOnServer,"wb+");
	if(targetFile==NULL){
		forwardPage(CGI_NAME,207,languagebuf);
		return -1;
	}

	while (cgiFormFileRead(file, buffer, BufferLen, &got) ==cgiFormSuccess){
		if(got>0){
			#if 0
			if(flag == 0){
				char tmpsync[20]={0};
				sprintf(tmpsync,"%02X%02X%02X%02X%02X%02X%02X%02X",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);
				if(strcmp(UPDATEFILEHEAD,tmpsync))
				{
					memset(languagebuf,0,150);
					getLanguageValue(sys_language,"uploadFileFail",languagebuf);
					forwardPage(CGI_NAME,207,languagebuf);
					return -1;
				}else{
					fwrite(buffer+8,1,got-8,targetFile);
				}
				flag = 1;
			}else
			#endif
				fwrite(buffer,1,got,targetFile);
		}
	}
	cgiFormFileClose(file);
	fclose(targetFile);
	ret = WebUploadLogo(tempname);
	if(ret == -3) {
		memset(languagebuf,0,150);
		getLanguageValue(sys_language,"uploadLogoTooLarge",languagebuf);
	} else if(ret == -4) {
		memset(languagebuf,0,150);
		getLanguageValue(sys_language,"pngfile",languagebuf);
	} else {
		memset(languagebuf,0,150);
		getLanguageValue(sys_language,"uploadFileSuccess",languagebuf);
	}
	fprintf(cgiOut,"<script type='text/javascript'>alert('%s');</script>",languagebuf);
	return 0;
}

static int mid_ip_is_multicast(char *ip) {
	struct in_addr	 addr ;
	unsigned int 	dwAddr;
	unsigned int 	value;

	inet_aton(ip, &addr);
	memcpy(&value, &addr, 4);
	dwAddr = htonl(value);

	//PRINTF("ip=%s.dwAddr=0x%08x\n", ip, dwAddr);

	if(((dwAddr & 0xFF000000) >> 24) > 223) {
		return 1;
	}

	return 0;
}


static int changePassword()
{
	char password[21]={0};
	char passwordagain[21]={0};
	char oldpassword[21]={0};
	char tmpbuf[100]={0};
	int rec=0;
	char username[20]={0};
	cgiFormString("password1",password,21);
	cgiFormString("password2",passwordagain,21);
	cgiFormString("oldpassword",oldpassword,21);
	cgiFormString("username",username,20);

	if(strcmp(username, USERNAME) != 0 && strcmp(username, WEBUSERNAME) != 0) {
		memset(tmpbuf,0,100);
		getLanguageValue(sys_language,"usernameNotExist",tmpbuf);
		fprintf(cgiOut, tmpbuf);
		return -1;
	}

	if(strcmp(username, USERNAME) == 0) {
		if(strcmp(oldpassword,sys_password))//旧密码不正确
		{
			memset(tmpbuf,0,100);
			getLanguageValue(sys_language,"oldpassworderror",tmpbuf);
			fprintf(cgiOut, tmpbuf);
			return -1;
		}
		if(strcmp(password,passwordagain)){//密码不同
			memset(tmpbuf,0,100);
			getLanguageValue(sys_language,"passwordTwiceInputNotSame",tmpbuf);
			fprintf(cgiOut, tmpbuf);
			return -1;
		}
		if(password==NULL||strlen(password)==0||!strcmp(password,"")){//密码为空
			memset(tmpbuf,0,100);
			getLanguageValue(sys_language,"passwordIsNull",tmpbuf);
			fprintf(cgiOut, tmpbuf);
			return -1;
		}
		rec=updateConfigFile("sysinfo.txt","password",password);
		memset(tmpbuf,0,100);
		getLanguageValue(sys_language,"modifyPasswordSuccess",tmpbuf);
		fprintf(cgiOut, tmpbuf);
		return 0;

	} else {
		if(strcmp(oldpassword,sys_webpassword))//旧密码不正确
		{
			memset(tmpbuf,0,100);
			getLanguageValue(sys_language,"oldpassworderror",tmpbuf);
			fprintf(cgiOut, tmpbuf);
			return -1;
		}
		if(strcmp(password,passwordagain)){//密码不同
			memset(tmpbuf,0,100);
			getLanguageValue(sys_language,"passwordTwiceInputNotSame",tmpbuf);
			fprintf(cgiOut, tmpbuf);
			return -1;
		}
		if(password==NULL||strlen(password)==0||!strcmp(password,"")){//密码为空
			memset(tmpbuf,0,100);
			getLanguageValue(sys_language,"passwordIsNull",tmpbuf);
			fprintf(cgiOut, tmpbuf);
			return -1;
		}
		rec=updateConfigFile("sysinfo.txt","webpassword",password);
		memset(tmpbuf,0,100);
		getLanguageValue(sys_language,"modifyPasswordSuccess",tmpbuf);
		fprintf(cgiOut, tmpbuf);
		return 0;
	}

}

char* Getline(char* pBuf, char* pLine)
{
	char* ptmp = strchr(pBuf, '\n');
	if(ptmp == NULL)
		return pBuf;
	strncpy(pLine, pBuf, ptmp - pBuf);
	return ptmp + 1;
}

int cgiMain(void) {
	int actionCode  = 0;
	int ret = 0;
	cgiFormInteger("actioncode", &actionCode, 0);
	memset(sys_password,0,100);
	memset(sys_webpassword,0,100);
	memset(sys_timeout,0,100);
	memset(sys_language,0,100);
	ret = getLanguageValue("sysinfo.txt","password",sys_password);
	if(ret == -1 || strlen(sys_password) == 0) {
		strcpy(sys_password,USERNAME);
	}
	getLanguageValue("sysinfo.txt","webpassword",sys_webpassword);
	if(ret == -1 || strlen(sys_webpassword) == 0) {
		strcpy(sys_webpassword,WEBUSERNAME);
	}
	getLanguageValue("sysinfo.txt","timeout",sys_timeout);
	getLanguageValue("sysinfo.txt","language",sys_language);
	if(ret == -1 || strlen(sys_language) == 0) {
		strcpy(sys_language,"us");
	}
	
	if(actionCode == UPDATE_LANGUAGE)
	{
		char local[10]={0};
		cgiFormString("local",local,10);
		if(strcmp(local,"us"))
			//fprintf(cgiOut,"Content-Type:text/html;charset=gbk\n\n");
			fprintf(cgiOut,"Content-Type:text/html;charset=gb2312\n\n");
		else
			//fprintf(cgiOut,"Content-Type:text/html;charset=utf-8\n\n");
			fprintf(cgiOut,"Content-Type:text/html;charset=gb2312\n\n");
	}else if(actionCode == ACTION_DOWNLOAD_SDP) {
		fprintf(cgiOut,"Content-Type:text/plain;charset=gb2312\n");
	}else{
		if(strcmp(sys_language,"us"))
			//fprintf(cgiOut,"Content-Type:text/html;charset=gbk\n\n");
			fprintf(cgiOut,"Content-Type:text/html;charset=gb2312\n\n");
		else
			//fprintf(cgiOut,"Content-Type:text/html;charset=utf-8\n\n");
			fprintf(cgiOut,"Content-Type:text/html;charset=gb2312\n\n");
	}
	switch (actionCode)
	{
    case UPDATE_HIDDEN_VALUE:
        {
            char dtype[20]={0};
	     char temptype[20] = {0};
			cgiFormString("dtype",dtype,16);
			if(strcmp(dtype,""))
				WebSetDeviceType(dtype,temptype);
            forwardPageU(CGI_NAME,PAGE_SYSTEM_SHOW,"ReachWebAdmin");
            break;
        }
	#if 0
	case YTKZ:
		{
			int speed = 0;
			int value = 0;
			int addr = 1;
			cgiFormInteger("speed",&speed,0);
			cgiFormInteger("dotype",&value,1);
			cgiFormInteger("addressbit",&addr,1);
			webFarCtrlCamera(addr,value,speed);
			showPage("./down.html",sys_language);
			break;
		}
	#endif
	case YTKZ_PROTO:
		{
			int index = 0;
			cgiFormInteger("use",&index,0);
			WebSetCtrlProto(index,&index);
			forwardPages(CGI_NAME,208);
			break;
		}
	case SCREEN_CHANGE:
		{
			int speed = 0;
			int value = 0;
			short hporch = 0;
			short vporch =0;
			cgiFormInteger("speed11",&speed,0);
			cgiFormInteger("dotype11",&value,1);
			if(speed==1)
			{
				hporch = 1;
				vporch = 1;
			}
			if(speed == 5)
			{
				hporch=6;
				vporch = 2;
			}
			if(speed == 10)
			{
				hporch = 10;
				vporch = 4;
			}
			if(value == 0)//Reset
				webRevisePicture(0,0);
			if(value == 5)//right
				webRevisePicture(0-hporch,0);
			if(value == 6)//left
				webRevisePicture(hporch,0);
			if(value == 7)//down
				webRevisePicture(0,0-vporch);
			if(value == 8)//up
				webRevisePicture(0,vporch);
			//printf("<script>");
			//printf("alert('%d,%d,%d');",value,hporch,vporch);
			//printf("</script>");
			showPage("./down.html",sys_language);
			break;
		}
	case SCREEN_GREEN:
		{
			int value = 0;
			webSetColorSpace(value, &value);
			forwardPages(CGI_NAME,208);
			break;
		}

	case UPDATE_LANGUAGE:
		{
			char local[10]={0};
			cgiFormString("local",local,10);
			updateConfigFile("sysinfo.txt","language",local);
			fprintf(cgiOut, "window.location.href='encoder.cgi';");
			break;
		}
	case UPDATE_MULTI_SET:
		{
			MultAddr multi;
			int tmp = 0;
			int statusss = -1;
			int aaa = 0;
			//WebGetMultiAddr(&multi,aaa);
			cgiFormString("Addr",multi.chIP,16);
			cgiFormInteger("Port",&tmp,0);
			multi.nPort = (unsigned short)tmp;
			cgiFormInteger("statussss",&statusss,1);
			cgiFormInteger("aaa",&aaa,0);
			if(statusss==0)
				//WebSetMultiStart(&multi,aaa);
			if(statusss==1)
				//WebSetMultiStop(aaa);
			sleep(2);
			forwardPages(CGI_NAME,208);
			break;
		}

		case ACTION_LOGIN:
			{
			  loginaction();
			}
			break;
		//密码修改
		case ACTION_CHANGEPWD:
		{
			changePassword();
		}
		break;
		case ACTION_UPLOADFILE:
			{
				updatesystems();
			}
		break;
		case ACTION_LOGOUT:
		{
			fprintf(cgiOut, "document.cookie = '';\n");
			fprintf(cgiOut, "parent.document.location.replace('/index.html');");
		}
		break;
		case PAGE_LOGIN_SHOW:
			showPage("./login.html",sys_language);
			break;
		case PAGE_TIMEOUT_SHOW:
			showPage("./timeout.html",sys_language);
			break;
		case PAGE_TOP_SHOW:
			showPage("./top.html",sys_language);
			break;
		case PAGE_DOWN_SHOW:
			showPage("./down.html",sys_language);
			break;
		case PAGE_MAIN_SHOW:
			{
				showPage("./index.html",sys_language);
			}
			break;
		case PAGE_SYSTEM_SHOW:
			{
				char user[20]={0};
//				char tmp[100]={0};
				SYSPARAMS sysparams;
				WebGetSysParam(&sysparams);
#ifdef LINUX
				struct in_addr inAddr;
				cgiFormString("u",user,20);
				showPage("./sys_op.html",sys_language);
				printf("<script>\n");
				memcpy(&inAddr,&sysparams.dwAddr,4);
				printf("document.forms[\"networkinfo\"].LanAddr.value='%s';\n",inet_ntoa(inAddr));
				memcpy(&inAddr,&sysparams.dwNetMark,4);
				printf("document.forms[\"networkinfo\"].LanSubMark.value='%s';\n",inet_ntoa(inAddr));
				memcpy(&inAddr,&sysparams.dwGateWay,4);
				printf("document.forms[\"networkinfo\"].LanGateWay.value='%s';\n",inet_ntoa(inAddr));

				if(strcmp(user,"ReachWebAdmin")==0){
					int ret = 0;
					printf("document.forms[\"updatepwd\"].oldpassword.value='%s';\n",sys_password);
					printf("showsetBoxSn.style.display='block';\n");
				}
			//	if(strcmp(user,"admin")==0){
				//	int ret = 0;
				//	printf("phy.style.display='block';\n");
					//ret=WebGetPHYmod();
					//printf("document.forms[\"phymod\"].Phymode.value='%d';\n",ret);

			//	}

				printf("document.getElementById('localeh').value='%s';\n",sys_language);
				if(!strcmp(sys_language,"us"))
					printf("document.getElementById('uslocal').checked=true;\n");
				else
					printf("document.getElementById('cnlocal').checked=true;\n");



				printf("</script>\n");
#else
				cgiFormString("u",user,20);
				showPage("./sys_op.html",sys_language);
				printf("<script>\n");
				if(!strcmp(user,"ReachWebAdmin")){
					printf("document.forms[\"updatepwd\"].oldpassword.value='%s';\n",sys_password);
				}
				printf("</script>\n");
#endif
			}
			break;
		case PAGE_SYSTEM_INFO:
			{/*
				char tmp[100]={0};
				SYSPARAMS sysparams;
#ifdef LINUX
				struct in_addr inAddr;
#endif

				showPage("./system.html",sys_language);
				printf("<script>\n");
				WebGetSysParam(&sysparams);

				printf("var server='%s';",sysparams.strName);//设备序号
//				printf("var types='DSS-ENC-1200';");	//设备类型
				//getLanguageValue(sys_language,"DEVICETYPE",tmp);
				WebGetDevideType(tmp);
				printf("var types='%s';",tmp);	//设备类型
				printf("var sysver='%s';",sysparams.strVer);//系统版本
				//WebGetkernelversion(tmp);
				printf("var linuxver='%s';\n",tmp);		//内核版本
				memset(tmp,0,100);
				//WebGetfpgaversion(tmp);
				printf("var fpgaver='%s';\n",tmp);
				printf("var webver='%s';\n",WEBVERSION);	//web版本
#ifdef LINUX
				memcpy(&inAddr,&sysparams.dwAddr,4);
				printf("var ipaddr='%s';\n",inet_ntoa(inAddr));
				memcpy(&inAddr,&sysparams.dwNetMark,4);
				printf("var mask='%s';\n",inet_ntoa(inAddr));
				memcpy(&inAddr,&sysparams.dwGateWay,4);
				printf("var gateway='%s';\n",inet_ntoa(inAddr));
#else
				printf("var ip='';\n");
				printf("var mask='';\n");
				printf("var gateway='';\n");
#endif
				printf("var mac='%02X-%02X-%02X-%02X-%02X-%02X';\n",sysparams.szMacAddr[0],sysparams.szMacAddr[1],sysparams.szMacAddr[2],sysparams.szMacAddr[3],sysparams.szMacAddr[4],sysparams.szMacAddr[5]);
				printf("initWebHtml();\n");
				printf("</script>\n");
			}
		break;
		case ACTION_NETWORKSET:
			{
				SYSPARAMS param;
				char addr[16]={0};
				char submark[16]={0};
				char gateway[16]={0};
				int rec = 0;
				WebGetSysParam(&param);
				cgiFormString("LanAddr",addr,16);
				cgiFormString("LanSubMark",submark,16);
				cgiFormString("LanGateWay",gateway,16);
#ifdef LINUX
				param.dwAddr = inet_addr(addr);
				param.dwNetMark = inet_addr(submark);
				param.dwGateWay = inet_addr(gateway);
#endif
				rec = WebSetSysParam(&param,&param);
				if(rec == 0)
				{
					char tmp[150]={0};
					getLanguageValue(sys_language,"modifyIPSuccess",tmp);
					forwardPage(CGI_NAME,PAGE_SYSTEM_SHOW,tmp);
				}else
				{
					char tmp[150]={0};
					getLanguageValue(sys_language,"modifyIPFailed",tmp);
					forwardPage(CGI_NAME,PAGE_SYSTEM_SHOW,tmp);
				}
			*/}
			break;
		case PAGE_PARAMETER_SHOW:
			{
				VideoParam vp;
				AudioParam ap;
				textinfo info;
				MultAddr multi;
				Protocol pro={0};
				int vinput = 0;
				int tmpBitRate = 0;
				showPage("./paramset.htm",sys_language);
				int ytkz = 0;

				printf("<script>\n");
				WebGetProtocol(&pro);
				if(pro.status!=0){
					int i=0;
					char buf[20]={0};
					char *host = getenv("HTTP_HOST");
					if(host==NULL){
						sprintf(buf,"127.0.0.1");
					}else{
						char *have=strstr(host,":");
						if(have!=NULL){
							strncpy(buf,host,have-host);
							buf[have-host]='\0';
						}else
							strcpy(buf,host);
						if(!strcmp(buf,"localhost"))
							sprintf(buf,"127.0.0.1");
					}

					for(;i<4;i++){

						if(i==0&&pro.TS==0){
							printf("divTS.style.display='none';\n");
							continue;
						}
						if(i==1&&pro.RTP==0){
							printf("divRTP.style.display='none';\n");
							continue;
						}
						if(i==2&&pro.RTSP==0){
							printf("divRTSP.style.display='none';\n");
							continue;
						}
						if(i==3&&pro.RTMP==0){
							printf("divRTMP.style.display='none';\n");
							continue;
						}
						//WebGetMultiAddr(&multi,i);
						if(multi.nStatus==1){
							printf("divoff%d.style.display='block';\n",i);
							printf("divon%d.style.display='none';\n",i);
							printf("document.getElementById('Addr%d').readOnly = true;\n",i);
							printf("document.getElementById('Port%d').readOnly = true;\n",i);
						}else{
							printf("divoff%d.style.display='none';\n",i);
							printf("divon%d.style.display='block';\n",i);
						}
						printf("document.getElementById('Addr%d').value='%s';\n",i,multi.chIP);
						printf("document.getElementById('Port%d').value=%d;\n",i,multi.nPort);
					}
					printf("document.getElementById('Addr2').value='%s';\n",buf);
					printf("document.getElementById('Addr2').readOnly = true;\n");

				}else{
					printf("divMultiSet.style.display='none';");
				}
				WebGetVideoParam(&vp);
				WebGetAudioParam(&ap);
				gettexttodisplay(&info);
				WebGetinputSource((inputtype*)&vinput);
				//printf("document.getElementById('statussss').value=%d;\n",multi.nStatus);

				setformintvalue("VParamForm0","framerate",vp.nFrameRate);
				setformintvalue("VParamForm0","limitmode",vp.sCbr);
				if(vp.sCbr==0)//质量
				{
					printf("document.getElementById('limitmode0').checked=true;\n");
					printf("changelimitmode(0,0);\n");
					setformintvalue("VParamForm0","AV1Quality",vp.nQuality);
				}else{
					printf("document.getElementById('limitmode1').checked=true;\n");
					printf("changelimitmode(1,0);\n");
					if(vp.sBitrate<256)
						tmpBitRate = 128;
					else if(vp.sBitrate<384)
						tmpBitRate = 256;
					else if(vp.sBitrate<512)
						tmpBitRate = 384;
					else if(vp.sBitrate<768)
						tmpBitRate = 512;
					else if(vp.sBitrate<1000)
						tmpBitRate = 768;
					else if(vp.sBitrate<2000)
						tmpBitRate = 1000;
					else if(vp.sBitrate<3000)
						tmpBitRate = 2000;
					else if(vp.sBitrate<4000)
						tmpBitRate = 3000;
					else if(vp.sBitrate<5000)
						tmpBitRate = 4000;
					else if(vp.sBitrate<6000)
						tmpBitRate = 5000;
					else if(vp.sBitrate<7000)
						tmpBitRate = 6000;
					else if(vp.sBitrate<8000)
						tmpBitRate = 7000;
					else if(vp.sBitrate<9000)
						tmpBitRate = 8000;
					else if(vp.sBitrate<10000)
						tmpBitRate = 9000;
				#if (defined(DSS_ENC_1200) || defined(DSS_ENC_1260))
					else if(vp.sBitrate<15000)
						tmpBitRate = 10000;
					else if(vp.sBitrate<20000)
						tmpBitRate = 15000;
					else
						tmpBitRate = 20000;
				#else
					else
						tmpBitRate = 10000;
				#endif
					setformintvalue("VParamForm0","AV1Bandwidth",tmpBitRate);
				}
				setformintvalue("VParamForm0","vinput",vinput);


				setformintvalue("audioparamform","samplerate",ap.SampleRate);
				setformintvalue("audioparamform","BitRate",ap.BitRate);
				//setformintvalue("audioparamform","leftvolume",ap.LVolume);
				//setformintvalue("audioparamform","rightvolume",ap.RVolume);
				if(ap.InputMode==0)
					printf("document.getElementById('inputmode0').checked=true;\n");
				else
					printf("document.getElementById('inputmode1').checked=true;\n");
				setformintvalue("audioparamform","audioinput",ap.InputMode);
				//msgform
				setformintvalue("msgform","xx",info.xpos);
				setformintvalue("msgform","yy",info.ypos);
				setformdata("msgform","message",info.msgtext);
				if(info.showtime == 1)
					printf("document.msgform.checkbox1.checked=true;\n");
				setformintvalue("msgform","showtime",info.showtime);

				printf("xxdiv.innerHTML='0~%d';\n",(vp.nWidth - 32));
				printf("yydiv.innerHTML='0~%d';\n",(vp.nHight - 32));


				WebGetCtrlProto(&ytkz);
				setformintvalue("ytparamform","use",ytkz);


				printf("</script>\n");
			}
			break;
		case ACTION_UPDATEVIDEOPARAM:
			{
				VideoParam param;
				int rate = 0;
				int vinput = 0;
				int mode = 0;
				int band = 0;
				int quality = 0;
				int rec = 0;
				int rec1 = 0;
				char tmp[100]={0};
				cgiFormInteger("framerate",&rate,0);
				cgiFormInteger("vinput",&vinput,0);
				cgiFormInteger("limitmode",&mode,0);
				cgiFormInteger("AV1Bandwidth",&band,0);
				cgiFormInteger("AV1Quality",&quality,0);
				WebGetVideoParam(&param);
				param.nFrameRate = rate;
				param.sCbr = mode;

				if(param.sCbr ==0)
					param.nQuality = quality;
				else
					param.sBitrate = band;
				rec=WebSetVideoParam(&param,&param);
				rec1=WebSetinputSource(vinput,&vinput);
				if(rec ==0&&rec1==0)
				{
					memset(tmp,0,100);
					getLanguageValue(sys_language,"setvideoSuccess",tmp);
					forwardPage(CGI_NAME,PAGE_PARAMETER_SHOW,tmp);
					return 0;
				}else{
					memset(tmp,0,100);
					getLanguageValue(sys_language,"setvideoFailed",tmp);
					forwardPage(CGI_NAME,PAGE_PARAMETER_SHOW,tmp);
					return -1;
				}
			}
			break;
		case ACTION_UPDATEAUDIOPARAM:
			{
				AudioParam param;
				int sample = 0;
				int bitrate = 0;
				int left = 0;
				int right = 0;
				int ainput = 0;
				int rec = 0;
				char tmp[100]={0};
				WebGetAudioParam(&param);
				cgiFormInteger("samplerate",&sample,2);//考虑是否和其他同步，使用0,1,2
				cgiFormInteger("BitRate",&bitrate,0);
				//cgiFormInteger("leftvolume",&left,0);
				//cgiFormInteger("rightvolume",&right,0);
				cgiFormInteger("audioinput",&ainput,0);
				param.BitRate = bitrate;
				param.SampleRate = sample;
				//param.LVolume = left;
				//param.RVolume = right;
				param.InputMode =ainput;
				rec=WebSetAudioParam(&param,&param);
				if(rec ==0)
				{
					memset(tmp,0,100);
					getLanguageValue(sys_language,"setaudioSuccess",tmp);
					forwardPage(CGI_NAME,PAGE_PARAMETER_SHOW,tmp);
					return 0;
				}else{
					memset(tmp,0,100);
					getLanguageValue(sys_language,"setaudioFailed",tmp);
					forwardPage(CGI_NAME,PAGE_PARAMETER_SHOW,tmp);
					return -1;
				}
			}
			break;
		case ACTION_TEXT_SET:
			{
				/*textinfo info;
				int rec =0;
				char tmp[100]={0};
				VideoParam vp;
				gettexttodisplay(&info);
				cgiFormString("message",&info.msgtext,128);
				cgiFormInteger("xx",&info.xpos,0);
				cgiFormInteger("yy",&info.ypos,0);
				cgiFormInteger("showtime",&info.showtime,0);

				WebGetVideoParam(&vp);
				if(info.xpos >(vp.nWidth -32)){
					memset(tmp,0,100);
					getLanguageValue(sys_language,"paramset.xxerror",tmp);
					sprintf(tmp,"%s%d",tmp,vp.nWidth - 32);
					forwardPage(CGI_NAME,PAGE_PARAMETER_SHOW,tmp);
					return -1;
				}
				if(info.ypos >(vp.nHight-32)){
					memset(tmp,0,100);
					getLanguageValue(sys_language,"paramset.yyerror",tmp);
					sprintf(tmp,"%s%d",tmp,vp.nHight - 32);
					forwardPage(CGI_NAME,PAGE_PARAMETER_SHOW,tmp);
					return -1;
				}

				rec = Webaddtexttodisplay(&info);
				if(rec ==0)
				{
					memset(tmp,0,100);
					getLanguageValue(sys_language,"settextSuccess",tmp);
					//sprintf(tmp,"%s:%d",tmp,info.showtime);
					forwardPage(CGI_NAME,PAGE_PARAMETER_SHOW,tmp);
					return 0;
				}else{
					memset(tmp,0,100);
					getLanguageValue(sys_language,"settextFailed",tmp);
					forwardPage(CGI_NAME,PAGE_PARAMETER_SHOW,tmp);
					return -1;
				}*/
			}
			break;
		case RESTART_SHOW:
			{
				showPage("./restart.html",sys_language);
			}
			break;
		case RESTART_SYS:
			{
				//system("reboot -f");
				webRebootSystem();
				fprintf(cgiOut, "%d", RESULT_SUCCESS);
			}
			break;
		case ACTION_SAVEUPDATE:
			{
				char tmp[100]={0};
				int rec = -1;
				rec = WebSaveParam();
				//showPage("./timeout.html",sys_language);
				memset(tmp,0,100);
				if(!rec)
					getLanguageValue(sys_language,"savesucess",tmp);
				else
					getLanguageValue(sys_language,"savefailed",tmp);
				forwardPage(CGI_NAME,PAGE_PARAMETER_SHOW,tmp);

			}
			break;
		case ACTION_SYNC:
			{
				char time[30]={0};
				char tmpbuff[100]={0};
				int i = 0;
				int rec = 0;
				DATE_TIME_INFO ctime={0};
				char *d="/";
				char *p;
				cgiFormString("clienttime",time,30);
				p=strtok(time,d);
				while(p)
				{
					if(i==0)
						ctime.year = atoi(p);
					if(i==1)
						ctime.month = atoi(p);
					if(i==2)
						ctime.mday = atoi(p);
					if(i==3)
						ctime.hours = atoi(p);
					if(i==4)
						ctime.min = atoi(p);
					if(i==5)
						ctime.sec = atoi(p);
					p = strtok(NULL,d);
					i++;
				}
				rec = Websynctime(&ctime,&ctime);
				if(rec == 0){
					getLanguageValue(sys_language,"synchronizedTimeSuccess",tmpbuff);
				}else {
					getLanguageValue(sys_language,"synchronizedTimeFail",tmpbuff);
				}
				fprintf(cgiOut, tmpbuff);
			}
			break;
//		case SETPHY:
//			{
//				int mod;
//				int ret;
//			    cgiFormInteger("Phymode",&mod,0);
//			   	ret=WebSetPHYmod(mod);
//				printf("ret=%d\n",ret);
//				forwardPageU(CGI_NAME,207,"admin");
//
//			}
		case PAGE_LEFTMENU_SYSPARAMS_SHOW: {
			showPage("./leftmenuParamsSettings.html", sys_language);
			break;
		}
		case PAGE_LEFTMENU_SYSSET_SHOW: {
			showPage("./leftmenuSystemSettings.html", sys_language);
			break;
		}

		/*
		 * sysinfo page show action
		 */
		case PAGE_SYSINFO_SHOW: {
			SYSPARAMS sysParamsOut;
			struct in_addr inAddr; 

			memset(&sysParamsOut, 0, sizeof(SYSPARAMS));
			int outlen = 0;
			char compiledTime[300] = {0};
			int cpuUseRate = 0;
			char deviceModelNO[200] = {0};
			char webVersion[200] = {0};
			char serialNO[200] = {0};
			char softwareVersion[200] = {0};
			int IPAddress = 0;
			int submask = 0;
			int networkGate = 0;
			char maAdress[64] = {0};
			//strcat(deviceModelNO, "ENC1260");
			WebGetDevideType(deviceModelNO, sizeof(deviceModelNO));
			//strcat(webVersion, "v1.0.1");
			appCmdStructParse(MSG_GETSYSPARAM, NULL, sizeof(SYSPARAMS), &sysParamsOut, &outlen);
			appCmdIntParse(MSG_GETCPULOAD, 0, sizeof(int), &cpuUseRate, &outlen);
			appCmdStringParse(MSG_GETSOFTCONFIGTIME, NULL, strlen(compiledTime), compiledTime, &outlen);
			appCmdStringParse(MSG_GETSOFTVERSION, NULL, strlen(softwareVersion), softwareVersion, &outlen);

			
			strcat(serialNO, sysParamsOut.strName);
			//strcat(softwareVersion, sysParamsOut.strVer);
			IPAddress = sysParamsOut.dwAddr;
			submask = sysParamsOut.dwNetMark;
			networkGate = sysParamsOut.dwGateWay;
			strcat(maAdress, sysParamsOut.szMacAddr);
			strcat(webVersion, WEBVERSION);
			sprintf(maAdress,"%02X-%02X-%02X-%02X-%02X-%02X",sysParamsOut.szMacAddr[0],sysParamsOut.szMacAddr[1],sysParamsOut.szMacAddr[2],sysParamsOut.szMacAddr[3],sysParamsOut.szMacAddr[4],sysParamsOut.szMacAddr[5]);
			showPage("./sysinfo.html", sys_language);
			fprintf(cgiOut, "<script type='text/javascript'>\n");
				fprintf(cgiOut, "setFormItemValue('wmform', [{'name': 'deviceModelNO','value': '%s','type': 'text' }", deviceModelNO);
				fprintf(cgiOut, ",{'name': 'webVersion','value': '%s','type': 'text' }", webVersion);
				fprintf(cgiOut, ",{'name': 'compiledTime','value': '%s','type': 'text' }", compiledTime);
				fprintf(cgiOut, ",{'name': 'cpuUseRate','value': '%d%','type': 'text' }", cpuUseRate);
				fprintf(cgiOut, ",{'name': 'serialNO','value': '%s','type': 'text' }", serialNO);
				fprintf(cgiOut, ",{'name': 'softwareVersion','value': '%s','type': 'text' }", softwareVersion);
				memcpy(&inAddr,&IPAddress,4);
				fprintf(cgiOut, ",{'name': 'IPAddress','value': '%s','type': 'text' }", inet_ntoa(inAddr));
				memcpy(&inAddr,&submask,4);
				fprintf(cgiOut, ",{'name': 'submask','value': '%s','type': 'text' }", inet_ntoa(inAddr));
				memcpy(&inAddr,&networkGate,4);
				fprintf(cgiOut, ",{'name': 'networkGate','value': '%s','type': 'text' }", inet_ntoa(inAddr));
				fprintf(cgiOut, ",{'name': 'maAdress','value': '%s','type': 'text' }]);\n", maAdress);
			fprintf(cgiOut, "</script>\n");
			break;
		}


		/*
		 * page input show action
		 */
		case PAGE_INPUT_SHOW: {
			int outlen = 0;
			char inputInfo[100] = {0};
			int colorSpace = 0;
			int hdcp = 0;
			int inputType = 0;
			appCmdStringParse(MSG_GETINPUTSIGNALINFO, NULL, strlen(inputInfo), inputInfo, &outlen);
			appCmdIntParse(MSG_GETCOLORSPACE, 0, sizeof(int), &colorSpace, &outlen);
			appCmdIntParse(MSG_GETHDCPVALUE, 0, sizeof(int), &hdcp, &outlen);
			WebGetinputSource(&inputType);

			showPage("./input.html", sys_language);

			fprintf(cgiOut, "<script type='text/javascript'>\n");
				fprintf(cgiOut, "setFormItemValue('wmform', [{'name': 'inputInfo','value': '%s','type': 'text' }", inputInfo);
				fprintf(cgiOut, ",{'name': 'colorSpace','value': '%d','type': 'select' }",colorSpace);
				fprintf(cgiOut, ",{'name': 'inputType','value': '%d','type': 'select' }",inputType);
				fprintf(cgiOut, ",{'name': 'hdcp','value': '%d','type': 'text'}]);\n", hdcp);
				fprintf(cgiOut, "fixedHDCPText();\n");
				fprintf(cgiOut, "formBeautify();\n");
			fprintf(cgiOut, "</script>\n");
			break;
		}

		/*
		 * set input action
		 */
		case ACTION_INPUT_SET: {
			int outValue = 0;
			int outlen = 0;
			int colorSpace = 0;
			int inputType = 0;
			int oldinput = 0;
			cgiFormInteger("colorSpace", &colorSpace, 0);
			cgiFormInteger("inputType", &inputType, 0);
			WebGetinputSource(&oldinput);
			
			WebSetinputSource(inputType, &outValue);
			if(oldinput == inputType)
			{
				appCmdIntParse(MSG_SETCOLORSPACE, colorSpace, sizeof(int), &outValue, &outlen);
			}

			fprintf(cgiOut, "%d", RESULT_SUCCESS);
			break;
		}

		/*
		 * video page show action
		 */
		case PAGE_VIDEO_SHOW: {
			int outlen = 0;
			int resolution = 0;
			int zoomModel = 0;
			int encodeLevel = 0;
			int sceneSet = 0;
			int fpsValue = 0;
			int interframeSpace = 0;
			int codeRate = 0;
			int captionLogoType = 0;
			int caption = 0;
			int logo = 0;
			OutputVideoInfo output_info;
			#if 0
			appCmdIntParse(MSG_GETOUTPUTRESOLUTION, 0, sizeof(int), &resolution, &outlen);
			appCmdIntParse(MSG_GETRESIZEMODE, 0, sizeof(int), &zoomModel, &outlen);
			appCmdIntParse(MSG_GETENCODELV, 0, sizeof(int), &encodeLevel, &outlen);
			appCmdIntParse(MSG_GETSCENE, 0, sizeof(int), &sceneSet, &outlen);
			appCmdIntParse(MSG_GETFRAMERATE, 0, sizeof(int), &fpsValue, &outlen);
			appCmdIntParse(MSG_GETIFRAMEINTERVAL, 0, sizeof(int), &interframeSpace, &outlen);
			appCmdIntParse(MSG_GETBITRATE, 0, sizeof(int), &codeRate, &outlen);
			appCmdIntParse(MSG_GETSHOWTEXTLOGO, 0, sizeof(int), &captionLogoType, &outlen);
			#else
			webGetOutputVideoParam(&output_info);
			resolution = output_info.resolution;
			zoomModel = output_info.resizeMode;
			encodeLevel = output_info.encodelevel;
			sceneSet = output_info.preset ;
			fpsValue = output_info.nFrameRate;
			interframeSpace = output_info.IFrameinterval;
			codeRate = output_info.sBitrate;
			//captionLogoType = output_info.logotext;
			caption = output_info.text_show;
			logo = output_info.logo_show;
			//appCmdIntParse(MSG_GET_SHOWTEXT_FLAG, 0, sizeof(int), &caption, &outlen);
			//appCmdIntParse(MSG_GET_SHOWLOGO_FLAG, 0, sizeof(int), &logo, &outlen);
			#endif
			#if 0
			switch(captionLogoType) {
				case 0: {
					caption = 0;
					logo = 0;
					break;
				}
				case 1: {
					caption = 1;
					logo = 0;
					break;
				}
				case 2: {
					caption = 0;
					logo = 1;
					break;
				}
				case 3: {
					caption = 1;
					logo = 1;
					break;
				}
			}
			#endif

			showPage("./video.html", sys_language);
			fprintf(cgiOut, "<script type='text/javascript'>\n");
				fprintf(cgiOut, "setFormItemValue('wmform', [{'name': 'resolution','value': '%d','type': 'select' }", resolution);
				fprintf(cgiOut, ",{'name': 'zoomModel','value': '%d','type': 'radio' }", zoomModel);
				fprintf(cgiOut, ",{'name': 'encodeLevel','value': '%d','type': 'select' }", encodeLevel);
				fprintf(cgiOut, ",{'name': 'sceneSet','value': '%d','type': 'select' }", 2);
				fprintf(cgiOut, ",{'name': 'fpsValue','value': '%d','type': 'text' }", fpsValue);
				fprintf(cgiOut, ",{'name': 'interframeSpace','value': '%d','type': 'text' }", interframeSpace);
				fprintf(cgiOut, ",{'name': 'codeRate','value': '%d','type': 'text' }", codeRate);
				fprintf(cgiOut, ",{'name': 'caption','value': '%d','type': 'checkbox' }", caption);
				fprintf(cgiOut, ",{'name': 'logo','value': '%d','type': 'checkbox' }]);\n", logo);
				fprintf(cgiOut, "initSceneSet();\n");
				fprintf(cgiOut, "formBeautify();\n");
			fprintf(cgiOut, "</script>\n");
			break;
		}

		/*
		 * video set action
		 */
		case ACTION_VIDEO_SET: {
			int outValue = 0;
			int outlen = 0;
			int resolution = 0;
			int zoomModel = 0;
			int encodeLevel = 0;
			int sceneSet = 0;
			int fpsValue = 0;
			int interframeSpace = 0;
			int codeRate = 0;
			int captionLogoType = 0;
			int caption = 0;
			int logo = 0;

			OutputVideoInfo oldinfo ;
			OutputVideoInfo newinfo;
			memset(&oldinfo,0,sizeof(OutputVideoInfo));
			memset(&newinfo,0,sizeof(OutputVideoInfo));

			cgiFormInteger("resolution", &resolution, 0);
			cgiFormInteger("zoomModel", &zoomModel, 0);
			cgiFormInteger("encodeLevel", &encodeLevel, 0);
			cgiFormInteger("sceneSet", &sceneSet, 0);
			cgiFormInteger("fpsValue", &fpsValue, 0);
			cgiFormInteger("interframeSpace", &interframeSpace, 0);
			cgiFormInteger("codeRate", &codeRate, 0);
			cgiFormInteger("caption", &caption, 0);
			cgiFormInteger("logo", &logo, 0);
			#if 0
			if(caption==0 && logo==0) {
				captionLogoType = 0;
			} else if(caption==1 && logo==0) {
				captionLogoType = 1;
			} else if(caption==0 && logo==1) {
				captionLogoType = 2;
			} else if(caption==1 && logo==1) {
				captionLogoType = 3;
			}

			#endif

			
			webGetOutputVideoParam(&oldinfo);

			oldinfo.resolution = resolution;
			oldinfo.resizeMode = zoomModel;
			oldinfo.encodelevel = encodeLevel;
			oldinfo.preset = sceneSet;
			oldinfo.nFrameRate = fpsValue;
			oldinfo.IFrameinterval =  interframeSpace;
			oldinfo.sBitrate = codeRate;
			oldinfo.logo_show = logo;
			oldinfo.text_show = caption;
			//oldinfo.logotext = captionLogoType;

			/*
			appCmdIntParse(MSG_LOCK_RESOLUTION, resolution, sizeof(int), &outValue, &outlen);
			appCmdIntParse(MSG_RESIZE_MODE, zoomModel, sizeof(int), &outValue, &outlen);
			appCmdIntParse(MSG_SETENCODELV, encodeLevel, sizeof(int), &outValue, &outlen);
			appCmdIntParse(MSG_SETSCENE, sceneSet, sizeof(int), &outValue, &outlen);
			appCmdIntParse(MSG_SETFRAMERATE, fpsValue, sizeof(int), &outValue, &outlen);
			appCmdIntParse(MSG_SETIFRAMEINTERVAL, interframeSpace, sizeof(int), &outValue, &outlen);
			appCmdIntParse(MSG_SETBITRATE, (codeRate), sizeof(int), &outValue, &outlen);
			appCmdIntParse(MSG_SETSHOWTEXTLOGO, captionLogoType, sizeof(int), &outValue, &outlen);*/
			webSetOutputVideoParam(&oldinfo,&newinfo);

			//appCmdIntParse(MSG_SET_SHOWTEXT_FLAG, caption, sizeof(int), &outValue, &outlen);
			//appCmdIntParse(MSG_SET_SHOWLOGO_FLAG, logo, sizeof(int), &outValue, &outlen);
			

			fprintf(cgiOut, "%d", RESULT_SUCCESS);
			break;
		}

		/*
		 * audio page show action
		 */
		case PAGE_AUDIO_SHOW: {
			int outlen = 0;
			int input = 0;
			int sampleRate = 0;
			char audioLevelLeft = '0';
			char audioLevelRight = '0';
			int bitRate = 0;
			int mute = 0;
			AudioParam audioParam;
			appCmdStructParse(MSG_GETAUDIOPARAM, NULL, sizeof(AudioParam), &audioParam, &outlen);
			input = audioParam.InputMode;
			sampleRate = audioParam.SampleRate;
			//modify by zhangmin ,the app volume is from 0-30
			audioLevelLeft = audioParam.LVolume /3;
			audioLevelRight = audioParam.RVolume /3;
			bitRate = audioParam.BitRate;
			appCmdIntParse(MSG_GETMUTE, 0, sizeof(int), &mute, &outlen);

			showPage("./audio.html", sys_language);

			fprintf(cgiOut, "<script type='text/javascript'>\n");
				fprintf(cgiOut, "setFormItemValue('wmform', [{'name': 'input','value': '%d','type': 'radio' }", input);
				fprintf(cgiOut, ",{'name': 'sampleRate','value': '%d','type': 'select' }", sampleRate);
				fprintf(cgiOut, ",{'name': 'audioLevelLeft','value': '%d','type': 'select' }", (int)audioLevelLeft);
				fprintf(cgiOut, ",{'name': 'audioLevelRight','value': '%d','type': 'select' }", (int)audioLevelRight);
				fprintf(cgiOut, ",{'name': 'mute','value': '%d','type': 'checkbox' }", mute);
				fprintf(cgiOut, ",{'name': 'bitRate','value': '%d','type': 'select' }]);\n", bitRate);
				fprintf(cgiOut, "formBeautify();\n");
			fprintf(cgiOut, "</script>\n");
			break;
		}

		/*
		 * audio page set action
		 */
		case ACTION_AUDIO_SET: {
			int outvalue = 0;
			int outlen = 0;
			int input = 0;
			int sampleRate = 0;
			int audioLevelLeft = 0;
			int audioLevelRight = 0;
			int bitRate = 0;
			int mute = 0;
			cgiFormInteger("input", &input, 0);
			cgiFormInteger("sampleRate", &sampleRate, 0);
			cgiFormInteger("audioLevelLeft", &audioLevelLeft, 0);
			cgiFormInteger("audioLevelRight", &audioLevelRight, 0);
			cgiFormInteger("bitRate", &bitRate, 0);
			cgiFormInteger("mute", &mute, 0);
			AudioParam audioParamIn;
			AudioParam audioParamOut;
			memset(&audioParamIn, 0, sizeof(AudioParam));
			memset(&audioParamOut, 0, sizeof(AudioParam));

			appCmdStructParse(MSG_GETAUDIOPARAM, NULL, sizeof(AudioParam), &audioParamIn, &outlen);

			audioParamIn.InputMode = input;
			
			audioParamIn.SampleRate = sampleRate;
			audioParamIn.LVolume = audioLevelLeft &0xff;
			audioParamIn.RVolume = audioLevelRight &0xff;
			audioParamIn.BitRate = bitRate;

			//add by zm becase the volume is from 0-30
			audioParamIn.LVolume = audioParamIn.LVolume*3;
			audioParamIn.RVolume = audioParamIn.RVolume*3;
	
			appCmdStructParse(MSG_SETAUDIOPARAM, &audioParamIn, sizeof(AudioParam), &audioParamOut, &outlen);
			appCmdIntParse(MSG_SETMUTE, mute, sizeof(int), &outvalue, &outlen);
			fprintf(cgiOut, "%d", RESULT_SUCCESS);
			break;
		}

		/*
		 * output stream page show action.
		 */
		case PAGE_OUTPUT_SHOW: {
			int rowIndex = 0;
			char temp[1024] = {0};
			char formData[1024*2] = {0};
			stream_output_server_config output_out;
			memset(&output_out,0,sizeof(stream_output_server_config));
			int isRtspUsed = 0;
			int mutilCount = 0;
			int i = 0;
			
			strcat(formData,"var formData = [");
			isRtspUsed = app_rtsp_get_used();
				
			if(isRtspUsed == 1) {
				//get rtsp info
				app_rtsp_get_cinfo(&output_out);
				sprintf(temp,"%d", 0);
					strcat(formData, "{'rowIndex': '");
					strcat(formData, temp);
					strcat(formData, "', 'type': '");
				sprintf(temp,"%d", output_out.type);
					strcat(formData, temp);
					strcat(formData, "', 'ip': '");
					strcat(formData, output_out.main_ip);
					//strcat(formData, "192.168.7.9");
					strcat(formData, "', 'videoPort': '");
				sprintf(temp,"%d", output_out.video_port);
					strcat(formData, temp);
					strcat(formData, "', 'status': '");
				sprintf(temp,"%d", output_out.status);
					strcat(formData, temp);
					strcat(formData, "'},");
			}

				// get mutil server info
				mutilCount = app_mult_get_total_num();
			for(i=0; i< mutilCount; i++) {
				app_mult_get_server_config(i, &output_out);
				sprintf(temp,"%d", ++rowIndex);
					strcat(formData, "{'rowIndex': '");
					strcat(formData, temp);
					strcat(formData, "', 'type': '");
				sprintf(temp,"%d", output_out.type);
					strcat(formData, temp);
					strcat(formData, "', 'ip': '");
					strcat(formData, output_out.main_ip);
					strcat(formData, "', 'videoPort': '");
				sprintf(temp,"%d", output_out.video_port);
					strcat(formData, temp);
					strcat(formData, "', 'status': '");
				sprintf(temp,"%d", output_out.status);
					strcat(formData, temp);
				if(i==(mutilCount-1)) {
					strcat(formData, "'}");
				} else {
					strcat(formData, "'},");
				}
			}
			strcat(formData, "];\n");
			showPage("./output.html", sys_language);
			fprintf(cgiOut, "<script type='text/javascript'>");
				fprintf(cgiOut, formData);
				fprintf(cgiOut, "initTableData(formData);\n");
				fprintf(cgiOut, "formBeautify();\n");
				fprintf(cgiOut, "initDetailSetHandler();\n");
			fprintf(cgiOut, "</script>");

			break;
		}

		/*
		 * output stream set action
		 */
		case ACTION_OUTPUT_SET: {
			char temp[1024] = {0};
			char itemName[1024] = {0};
			int rowCount = 0;
			int type = 0;
			char ip[30] = {0};
			int videoPort = 0;
			int status = 0;
			int i = 0;
			stream_output_server_config in;
			stream_output_server_config out;
			memset(&in,0,sizeof(stream_output_server_config));
			memset(&out,0,sizeof(stream_output_server_config));
			cgiFormInteger("rowCount", &rowCount, 0);
			for(i=1; i<=rowCount; i++) {
				sprintf(temp,"%d", i);

				memset(itemName,0,sizeof(itemName));
				strcpy(itemName,"type");
				strcat(itemName, temp);
				cgiFormInteger(itemName, &type, 0);
				in.type = type;

				memset(itemName,0,sizeof(itemName));
				strcpy(itemName,"ip");
				strcat(itemName, temp);
				cgiFormString(itemName, ip, 16);
				if(strlen(ip) == 0)
					strcpy(in.main_ip,"1.2.3.4");
				else
					strcpy(in.main_ip ,ip);
				
				memset(itemName,0,sizeof(itemName));
				strcpy(itemName,"videoPort");
				strcat(itemName, temp);
				cgiFormInteger(itemName, &videoPort, 0);
				in.video_port = videoPort;

				memset(itemName,0,sizeof(itemName));
				strcpy(itemName,"status");
				strcat(itemName, temp);
				cgiFormInteger(itemName, &status, 0);
				in.status = status;

				if(type == TYPE_RTSP) {
					// RTSP
					if(status == S_DEL) {
						app_rtsp_delete();
					} else {
						app_rtsp_set_status(&in, &out);
					}
				} else {
					// mutil
					if(status == S_DEL) {
						app_mult_del_server(&in);
					} else {
						app_mult_set_server_status(&in, &out);
					}
				}
			}

			fprintf(cgiOut, "%d", RESULT_SUCCESS);
			break;
		}

		/*
		 * output add set action.
		 */
		case ACTION_OUTPUTADD_SET: {
			int type = 0;
			char ip[20] = {0};
			int videoPort = 0;
			int audioPort = 0;
			int mtu = 0;
			int ttl = 0;
			int tos = 0;
			int tc_flag = 0;
			int tc_rate = 0;
			stream_output_server_config in;
			stream_output_server_config out;
			
			memset(&in,0,sizeof(stream_output_server_config));
			memset(&out,0,sizeof(stream_output_server_config));

			
			cgiFormInteger("type", &type, 0);
			cgiFormString("ip", ip, 20);
			cgiFormInteger("videoPort", &videoPort, 0);
			cgiFormInteger("audioPort", &audioPort, 0);
			cgiFormInteger("mtu", &mtu, 0);
			cgiFormInteger("ttl", &ttl, 0);
			cgiFormInteger("tos", &tos, 0);
			cgiFormInteger("tc_flag", &tc_flag, 0);
			cgiFormInteger("tc_rate", &tc_rate, 0);

			if(tc_flag == 0) {
				tc_rate = 0;
			}

			if(type == TYPE_TS && mid_ip_is_multicast(ip) == 0) {
				fprintf(cgiOut, "%d", RESULT_MUSTMULTIIP);
				return 0;
			}

			in.type = type;
			//must set status is active
			in.status = S_ACTIVE;
			strcpy(in.main_ip,ip);
			in.video_port = videoPort;
			in.audio_port = audioPort;
			in.mtu = mtu;
			in.ttl = ttl;
			in.tos = tos;
			in.tc_flag = tc_flag;
			in.tc_rate = tc_rate;
			if(in.type == TYPE_RTSP)
				app_rtsp_add_server(&in, &out);
			else
				app_mult_add_server(&in, &out);
			fprintf(cgiOut, "%d", RESULT_SUCCESS);
			break;
		}

		/*
		 * output update show action
		 */
		case ACTION_OUTPUTUPDATE_GET: {
			int rowIndex = 0;
			int type = 0;
			char ip[20] = {0};
			int videoPort = 0;
			int status = 0;
			int avgBandwidth = 0;
			int ceilingBandwidth = 0;
			stream_output_server_config out;
			char temp[1024] = {0};
			char formData[1024*2] = {0};
			cgiFormInteger("rowIndex", &rowIndex, 0);
			cgiFormInteger("type", &type, 0);
			cgiFormString("ip", ip, 20);
			cgiFormInteger("videoPort", &videoPort, 0);
			cgiFormInteger("status", &status, 0);
			out.type = type;
			strcpy(out.main_ip,ip);
			out.video_port = videoPort;
			out.status = status;
			
			//if have rtsp
			/*if(  app_rtsp_get_used() == 1)
			{
				if(rowIndex == 1) {
					app_rtsp_get_cinfo(&out);
				}
				else
				{
					app_mult_get_server_config(rowIndex-2, &out);
				}
			}
			else
			{
				app_mult_get_server_config(rowIndex-1, &out);
			}*/

			if(rowIndex == 0) {
				app_rtsp_get_cinfo(&out);
			} else {
				app_mult_get_server_config(rowIndex-1, &out);
			}

			avgBandwidth = app_mult_get_rate(type);
			ceilingBandwidth = avgBandwidth*(100+out.tc_rate)/100;

				strcat(formData, "setFormItemValue('wmform_outputAdvancedSet', [{'type':'select', 'name':'type', 'value':'");
			sprintf(temp,"%d", out.type);
				strcat(formData, temp);
				strcat(formData, "'}, {'type':'text', 'name':'ip', 'value':'");
				strcat(formData, out.main_ip);
				strcat(formData, "'}, {'type':'text', 'name':'videoPort', 'value':'");
			sprintf(temp,"%d", out.video_port);
				strcat(formData, temp);
				strcat(formData, "'}, {'type':'text', 'name':'audioPort', 'value':'");
			sprintf(temp,"%d", out.audio_port);
				strcat(formData, temp);
				strcat(formData, "'}, {'type':'text', 'name':'mtu', 'value':'");
			sprintf(temp,"%d", out.mtu);
				strcat(formData, temp);
				strcat(formData, "'}, {'type':'text', 'name':'ttl', 'value':'");
			sprintf(temp,"%d", out.ttl);
				strcat(formData, temp);
				strcat(formData, "'}, {'type':'text', 'name':'tos', 'value':'");
			sprintf(temp,"%d", out.tos);
				strcat(formData, temp);
				strcat(formData, "'}, {'type':'checkbox', 'name':'tc_flag', 'value':'");
			sprintf(temp,"%d", out.tc_flag);
				strcat(formData, temp);
				strcat(formData, "'}, {'type':'text', 'name':'tc_rate', 'value':'");
			sprintf(temp,"%d", out.tc_rate);
				strcat(formData, temp);
				strcat(formData, "'}, {'type':'text', 'name':'avgBandwidth', 'value':'");
			sprintf(temp,"%d", avgBandwidth);
				strcat(formData, temp);
				strcat(formData, "'}, {'type':'text', 'name':'ceilingBandwidth', 'value':'");
			sprintf(temp,"%d", ceilingBandwidth);
				strcat(formData, temp);
				strcat(formData, "'}, {'type':'hidden', 'name':'rowIndex', 'value': '");
			sprintf(temp,"%d", rowIndex);
				strcat(formData, temp);
				strcat(formData, "'}]);");


			fprintf(cgiOut, formData);
			break;
		}

		/*
		 * output update set action
		 */
		case ACTION_OUTPUTUPDATE_SET: {
		//	int rowIndex = 0;
			int type = 0;
			char ip[20] = {0};
			int videoPort = 0;
			int audioPort = 0;
			int mtu = 0;
			int ttl = 0;
			int tos = 0;
			int tc_flag = 0;
			int tc_rate = 0;
		//	cgiFormInteger("rowIndex", &rowIndex, 0);
			stream_output_server_config in;
			stream_output_server_config out;
			memset(&in,0,sizeof(stream_output_server_config));
			memset(&out,0,sizeof(stream_output_server_config));
			cgiFormInteger("type", &type, 0);
			cgiFormString("ip", ip, 20);
			cgiFormInteger("videoPort", &videoPort, 9000);
			cgiFormInteger("audioPort", &audioPort, 9000);
			cgiFormInteger("mtu", &mtu, 1500);
			cgiFormInteger("ttl", &ttl, 0);
			cgiFormInteger("tos", &tos, 0);
			cgiFormInteger("tc_flag", &tc_flag, 0);
			cgiFormInteger("tc_rate", &tc_rate, 0);

			if(tc_flag == 0) {
				tc_rate = 0;
			}

			in.type = type;
			strcpy(in.main_ip,ip);
			in.video_port = videoPort;
			in.audio_port = audioPort;
			in.mtu = mtu;
			in.ttl = ttl;
			in.tos = tos;
			in.tc_flag = tc_flag;
			in.tc_rate = tc_rate;
			if(in.type == TYPE_RTSP) {
				app_rtsp_set_cinfo(&in, &out);
			} else {
				app_mult_set_server_config(&in, &out);
			}

			fprintf(cgiOut, "%d", RESULT_SUCCESS);
			break;
		}

		/*
		 * page captionlogo show action
		 */
		case PAGE_CAPTIONLOGO_SHOW: {
			int outvalue = 0;
			int outlen = 0;
			int cap_position = 0;
			
			int cap_x = 0;
			int cap_y = 0;
			char cap_text[128] = {0};
			int cap_alpha = 0;
			int cap_displaytime = 0;

			int logo_position = 0;
			int logo_x = 0;
			int logo_y = 0;
			int logo_alpha = 0;
			textinfo textInfo;
			LogoInfo logoInfo;

			memset(&textInfo, 0, sizeof(textinfo));
			memset(&logoInfo, 0, sizeof(LogoInfo));

			appCmdStructParse(MSG_GET_TEXTINFO, NULL, sizeof(textinfo), &textInfo, &outlen);
			appCmdStructParse(MSG_GET_LOGOINFO, NULL, sizeof(LogoInfo), &logoInfo, &outlen);

			//appCmdIntParse(MSG_GET_TEXTPOS_TYPE, 0, sizeof(int), &cap_position, &outlen);
			//appCmdIntParse(MSG_GET_LOGOPOS_TYPE, 0, sizeof(int), &logo_position, &outlen);
			
			cap_position = textInfo.postype;
			cap_x = textInfo.xpos;
			cap_y = textInfo.ypos;
			cap_alpha = textInfo.alpha;
			cap_displaytime = textInfo.showtime;
			strcpy(cap_text, textInfo.msgtext);

			logo_position = logoInfo.postype;
			logo_x = logoInfo.x;
			logo_y = logoInfo.y;
			logo_alpha = logoInfo.alpha;

			showPage("./captionlogo.html", sys_language);

			fprintf(cgiOut, "<script type='text/javascript'>\n");
				fprintf(cgiOut, "setFormItemValue('wmform', [");
				fprintf(cgiOut, "{'name': 'cap_x','value': '%d','type': 'text' }", cap_x);
				fprintf(cgiOut, ",{'name': 'cap_y','value': '%d','type': 'text' }", cap_y);
				fprintf(cgiOut, ",{'name': 'cap_position','value': '%d','type': 'select' }", cap_position);
				fprintf(cgiOut, ",{'name': 'cap_brightness','value': '%d','type': 'text' }", cap_alpha);
				fprintf(cgiOut, ",{'name': 'cap_text','value': '%s','type': 'text' }", cap_text);
				fprintf(cgiOut, ",{'name': 'cap_displaytime','value': '%d','type': 'checkbox' }", cap_displaytime);
				fprintf(cgiOut, ",{'name': 'logo_x','value': '%d','type': 'text' }", logo_x);
				fprintf(cgiOut, ",{'name': 'logo_y','value': '%d','type': 'text' }", logo_y);
				fprintf(cgiOut, ",{'name': 'logo_position','value': '%d','type': 'select' }", logo_position);
				fprintf(cgiOut, ",{'name': 'logo_brightness','value': '%d','type': 'text' }]);\n", logo_alpha);
				fprintf(cgiOut, "formBeautify();");
				fprintf(cgiOut, "fixBrightnessSlider();");
				fprintf(cgiOut, "initPositionSelect();");
			fprintf(cgiOut, "</script>\n");
			break;
		}


		/*
		 * page captionlogo set action.
		 */
		case ACTION_CAPTIONLOGO_SET: {
			char tmp[150]={0};
			int outlen = 0;
			int cap_position = 0;
			int cap_x = 0;
			int cap_y = 0;
			char cap_text[128] = {0};
			int cap_alpha = 0;
			int cap_displaytime = 0;

			int logo_position = 0;
			int logo_x = 0;
			int logo_y = 0;
			int logo_alpha = 0;
			textinfo textInfoIn;
			textinfo textInfoOut;
			LogoInfo logoInfoIn;
			LogoInfo logoInfoOut;

			memset(&textInfoIn, 0, sizeof(textinfo));
			memset(&textInfoOut, 0, sizeof(textinfo));
			memset(&logoInfoIn, 0, sizeof(LogoInfo));
			memset(&logoInfoOut, 0, sizeof(LogoInfo));

			appCmdStructParse(MSG_GET_TEXTINFO, NULL, sizeof(textinfo), &textInfoIn, &outlen);
			appCmdStructParse(MSG_GET_LOGOINFO, NULL, sizeof(LogoInfo), &logoInfoIn, &outlen);

			cgiFormInteger("cap_position", &cap_position, 0);
			cgiFormInteger("cap_x", &cap_x, 0);
			cgiFormInteger("cap_y", &cap_y, 0);
			cgiFormString("cap_text", cap_text, 1000);
			cgiFormInteger("cap_displaytime", &cap_displaytime, 0);
			cgiFormInteger("cap_brightness", &cap_alpha, 0);
			cgiFormInteger("logo_position", &logo_position, 0);
			cgiFormInteger("logo_x", &logo_x, 0);
			cgiFormInteger("logo_y", &logo_y, 0);
			cgiFormInteger("logo_brightness", &logo_alpha, 0);

			if(cap_position != ABSOLUTE2) {
				WebGetTextPos(cap_position, &cap_x, &cap_y);
			}

			if(logo_position != ABSOLUTE2) {
				WebGetLogoPos(logo_position, &logo_x, &logo_y);
			}

			textInfoIn.postype = cap_position;
			textInfoIn.xpos = cap_x;
			textInfoIn.ypos = cap_y;
			strcpy(textInfoIn.msgtext, cap_text);
			textInfoIn.alpha = cap_alpha;
			textInfoIn.showtime = cap_displaytime;

			logoInfoIn.postype = logo_position;
			logoInfoIn.x = logo_x;
			logoInfoIn.y = logo_y;
			logoInfoIn.alpha = logo_alpha;

			appCmdStructParse(MSG_SET_TEXTINFO, &textInfoIn, sizeof(textinfo), &textInfoOut, &outlen);
			appCmdStructParse(MSG_SET_LOGOINFO, &logoInfoIn, sizeof(LogoInfo), &logoInfoOut, &outlen);
			getLanguageValue(sys_language,"opt.success",tmp);
			fprintf(cgiOut, "<script type='text/javascript'>alert('%s');</script>", tmp);
			break;
		}

		case ACTION_UPLOADLOGOPIC_SET: {
			// update logo pictrue.
			//WebUploadLogo("/opt/reach/logoimg/logo.png");
			uploadLogo();
			//fprintf(cgiOut, "<script type='text/javascript'>alert('%s');</script>", "Success!");
			break;
		}

		/*
		 * page remotectrl show action.
		 */
		case PAGE_REMOTECTRL_SHOW: {
			int ptzProtocal = 0;
			WebGetCtrlProto(&ptzProtocal);
			showPage("./remotectrl.html", sys_language);
			fprintf(cgiOut, "<script type='text/javascript'>\n");
			fprintf(cgiOut, "setFormItemValue('wmform', [");
			fprintf(cgiOut, "{'name': 'ptzProtocal','value': '%d','type': 'select'}]);", ptzProtocal);
			fprintf(cgiOut, "formBeautify();");
			fprintf(cgiOut, "</script>\n");
			break;
		}

		/*
		 * page remotectrl set action.
		 */
		case PAGE_REMOTECTRL_SET: {
			int ptzProtocal = 0;
			int outvalue = 0;
			cgiFormInteger("ptzProtocal",&ptzProtocal,0);
			WebSetCtrlProto(ptzProtocal, &outvalue);
			fprintf(cgiOut, "%d", RESULT_SUCCESS);
			break;
		}

		/*
		 * PTZ control.
		 */
		case ACTION_PTZ_CTRL: {
			int speed = 0;
			int type = 0;
			int addr = 1;
			cgiFormInteger("speed", &speed, 1);
			cgiFormInteger("type", &type, 14);
			cgiFormInteger("addressbit",&addr,1);
			webFarCtrlCamera(addr,type, speed);
			break;
		}

		/*
		 * page network show action
		 */
		case PAGE_NETWORK_SHOW: {
			int outlen = 0;
			int dhcp = 0;
			int IPAddress = 0;
			int gateWay = 0;
			int subMask = 0;
			SYSPARAMS sysParamsOut;
			struct in_addr inAddr;   
			memset(&sysParamsOut, 0, sizeof(SYSPARAMS));
			//webGetDHCPFlag(&dhcp);
			appCmdStructParse(MSG_GETSYSPARAM, NULL, sizeof(SYSPARAMS), &sysParamsOut, &outlen);
			dhcp = sysParamsOut.nTemp[0];
			IPAddress = sysParamsOut.dwAddr;
			subMask = sysParamsOut.dwGateWay;
			gateWay	 = sysParamsOut.dwNetMark;

			showPage("./network.html", sys_language);

			fprintf(cgiOut, "<script type='text/javascript'>\n");
				fprintf(cgiOut, "setFormItemValue('wmform', [");
				memcpy(&inAddr,&IPAddress,4); 
				fprintf(cgiOut, "{'name': 'IPAddress','value': '%s','type': 'text' }", inet_ntoa(inAddr));
				//fprintf(cgiOut, "{'name': 'IPAddress','value': '%s','type': 'text' }", "192.168.7.9");
				memcpy(&inAddr,&gateWay,4); 
				fprintf(cgiOut, ",{'name': 'subMask','value': '%s','type': 'text' }", inet_ntoa(inAddr));
				//fprintf(cgiOut, ",{'name': 'subMask','value': '%s','type': 'text' }", "255.255.255.0");
				memcpy(&inAddr,&subMask,4); 
				fprintf(cgiOut, ",{'name': 'gateWay','value': '%s','type': 'text' }", inet_ntoa(inAddr));
				//fprintf(cgiOut, ",{'name': 'gateWay','value': '%s','type': 'text' }", "192.168.7.254");
				fprintf(cgiOut, ",{'name': 'dhcp','value': '%d','type': 'checkbox' }]);\n", dhcp);
				fprintf(cgiOut, "formBeautify();");
				fprintf(cgiOut, "initDHCPCheckBox();");
			fprintf(cgiOut, "</script>\n");
			break;
		}

		/*
		 * page network set action
		 */
		case ACTION_NETWORK_SET: {
		//	int outval = 0;
			int outlen = 0;
			int dhcp = 0;
			char IPAddress[20] = {0};
			char gateWay[20] = {0};
			char subMask[20] = {0};
			SYSPARAMS sysParamsIn;
			SYSPARAMS sysParamsOut;
			memset(&sysParamsIn, 0, sizeof(SYSPARAMS));
			memset(&sysParamsOut, 0, sizeof(SYSPARAMS));
			appCmdStructParse(MSG_GETSYSPARAM, NULL, sizeof(SYSPARAMS), &sysParamsIn, &outlen);
			
			cgiFormInteger("dhcp", &dhcp, 0);
			cgiFormString("IPAddress", IPAddress, 20);
			cgiFormString("gateWay", gateWay, 20);
			cgiFormString("subMask", subMask, 20);
			sysParamsIn.dwAddr = inet_addr(IPAddress);
			sysParamsIn.dwNetMark = inet_addr(subMask);
			sysParamsIn.dwGateWay = inet_addr(gateWay);
			sysParamsIn.nTemp[0] = dhcp;
			//webSetDHCPFlag(dhcp, &outval);
			appCmdStructParse(MSG_SETSYSPARAM, &sysParamsIn, sizeof(SYSPARAMS), &sysParamsOut, &outlen);

			fprintf(cgiOut, "%d", RESULT_SUCCESS);
			break;
		}

		case PAGE_MODIFYPASSWORD_SHOW: {
			char username[20] = {0};
			char webusername[20] = {0};
			//getLanguageValue("sysinfo.txt","username",username);
			//getLanguageValue("sysinfo.txt","webusername",webusername);
			showPage("./modifypassword.html", sys_language);
			fprintf(cgiOut, "<script type='text/javascript'>\n");
				fprintf(cgiOut, "initUsernameSelect(['%s', '%s']);", USERNAME, WEBUSERNAME);
				fprintf(cgiOut, "formBeautify();");
			fprintf(cgiOut, "</script>\n");
			break;
		}
		case PAGE_OTHERSET_SHOW: {
		//	int  len = 0;
			char temp[10] = {0};
			char encoderTime[256] = {0};
			DATE_TIME_INFO date_time_info;
			memset(&date_time_info, 0, sizeof(DATE_TIME_INFO));
			WebgetEncodetime(&date_time_info);
			sprintf(temp,"%d", date_time_info.year);
			strcpy(encoderTime, temp);
			strcat(encoderTime, "/");
			memset(temp,0,strlen(temp));
			sprintf(temp,"%d", date_time_info.month);
			strcat(encoderTime, temp);
			strcat(encoderTime, "/");
			memset(temp,0,strlen(temp));
			sprintf(temp,"%d", date_time_info.mday);
			strcat(encoderTime, temp);
			strcat(encoderTime, "/");
			memset(temp,0,strlen(temp));
			sprintf(temp,"%d", date_time_info.hours);
			strcat(encoderTime, temp);
			strcat(encoderTime, "/");
			memset(temp,0,strlen(temp));
			sprintf(temp,"%d", date_time_info.min);
			strcat(encoderTime, temp);
			strcat(encoderTime, "/");
			memset(temp,0,strlen(temp));
			sprintf(temp,"%d", date_time_info.sec);
			strcat(encoderTime, temp);

			showPage("./otherset.html", sys_language);

			fprintf(cgiOut, "<script type='text/javascript'>\n");
				fprintf(cgiOut, "initPageTime('%s', '%s');", encoderTime, sys_language);
			fprintf(cgiOut, "</script>\n");
			break;
		}
		case PAGE_SYSUPGRADE_SHOW: {
			showPage("./sysupgrade.html", sys_language);
			break;
		}
		case PAGE_RESETDEFAULT_SHOW: {
			showPage("./resetdefault.html", sys_language);
			break;
		}

		case ACTION_SETDEFAULT_SET: {
			restoreSet();
			fprintf(cgiOut, "%d", RESULT_SUCCESS);
			break;
		}

		case PAGE_INPUTDETAILS_SHOW: {
			showPage("./input_details.html", sys_language);
			break;
		}

		case ACTION_INPUTDETAILS_GET: {
			char inputDetailInfo[2048];
			webSignalDetailInfo(inputDetailInfo,2048);
			fprintf(cgiOut, "setFormItemValue('wmform_videoAdvancedSet', [");
			fprintf(cgiOut, "{'name': 'inputDetailInfo','value': '%s','type': 'textarea' }]);\n", inputDetailInfo);
			break;
		}

		case ACTION_ADJUSTSCREEN_SET: {
			int speed = 0;
			int value = 0;
			short hporch = 0;
			short vporch =0;
			cgiFormInteger("speed", &speed, 1);
			cgiFormInteger("cmdType" ,&value, 1);
			if(speed==1)
			{
				hporch = 1;
				vporch = 1;
			}
			if(speed == 5)
			{
				hporch=6;
				vporch = 2;
			}
			if(speed == 10)
			{
				hporch = 10;
				vporch = 4;
			}
			if(value == 0)//Reset
				webRevisePicture(0,0);
			if(value == 5)//right
				webRevisePicture(0-hporch,0);
			if(value == 6)//left
				webRevisePicture(hporch,0);
			if(value == 7)//down
				webRevisePicture(0,0-vporch);
			if(value == 8)//up
				webRevisePicture(0,vporch);
			break;
		}

		case PAGE_OUTPUTADVANCEDSET_SHOW: {
			showPage("./output_detailSet.html", sys_language);
			break;
		}
		case PAGE_OUTPUTRTSPSET_SHOW: {
			showPage("./output_rtspSet.html", sys_language);
			break;
		}

		case ACTION_OUTPUTRTSPSET_GET: {
			int serverPort = 0;
			int rtspActive = 0;
			int rtspMtu = 0;
			int streamMode = 0;
			char rtspMultiIP[16] = {0};
			int rtspMultiPort = 0;
			rtsp_server_config out;
			memset(&out, 0, sizeof(rtsp_server_config));
			app_rtsp_get_ginfo(&out);
			serverPort = out.s_port;
			rtspActive = out.active;
			rtspMtu = out.s_mtu;
			streamMode = out.mult_flag;
			strcpy(rtspMultiIP, out.mult_ip);
			rtspMultiPort = out.mult_port;
			fprintf(cgiOut, "setFormItemValue('wmform_outputRTSPSet', [");
			fprintf(cgiOut, "{'name': 'rtspActive','value': '%d','type': 'text' }", rtspActive);
			fprintf(cgiOut, ",{'name': 'rtspMtu','value': '%d','type': 'text' }", rtspMtu);
			fprintf(cgiOut, ",{'name': 'serverPort','value': '%d','type': 'text' }", serverPort);
			fprintf(cgiOut, ",{'name': 'streamMode','value': '%d','type': 'select' }", streamMode);
			fprintf(cgiOut, ",{'name': 'rtspMultiIP','value': '%s','type': 'text' }", rtspMultiIP);
			fprintf(cgiOut, ",{'name': 'rtspMultiPort','value': '%d','type': 'text' }]);\n", rtspMultiPort);
			break;
		}

		case ACTION_OUTPUTRTSPSET_SET: {
			int serverPort = 0;
			int rtspActive = 0;
			int rtspMtu = 0;
			int streamMode = 0;
			char rtspMultiIP[16] = {0};
			int rtspMultiPort = 0;
			rtsp_server_config in;
			rtsp_server_config out;
			memset(&in, 0, sizeof(rtsp_server_config));
			memset(&out, 0, sizeof(rtsp_server_config));
			cgiFormInteger("serverPort", &serverPort, 0);
			cgiFormInteger("rtspActive", &rtspActive, 0);
			cgiFormInteger("rtspMtu", &rtspMtu, 0);
			cgiFormInteger("streamMode", &streamMode, 0);
			cgiFormString("rtspMultiIP", rtspMultiIP, 16);
			cgiFormInteger("rtspMultiPort", &rtspMultiPort, 0);
			in.s_port = serverPort;
			in.active = rtspActive;
			in.s_mtu = rtspMtu;
			in.mult_flag = streamMode;
			strcpy(in.mult_ip, rtspMultiIP);
			in.mult_port = rtspMultiPort;
			app_rtsp_set_ginfo(&in, &out);
			fprintf(cgiOut, "%d", RESULT_SUCCESS);
			break;
		}

		case PAGE_VIDEOADVANCEDSET_SHOW: {
			showPage("./video_advancedSet.html", sys_language);
			break;
		}

		case ACTION_VIDEOADVANCEDSET_GET: {
			int param1 = 0;
			int param2 = 0;
			int param3 = 0;
			int param4 = 0;
			int param5 = 0;
			H264EncodeParam h264EncodeParam;
			memset(&h264EncodeParam, 0, sizeof(H264EncodeParam));
			/*webGetH264encodeParam(&h264EncodeParam);
			param1 = h264EncodeParam.param1;
			param2 = h264EncodeParam.param2;
			param3 = h264EncodeParam.param3;
			param4 = h264EncodeParam.param4;
			param5 = h264EncodeParam.param5;*/

			fprintf(cgiOut, "setFormItemValue('wmform_videoAdvancedSet', [");
			fprintf(cgiOut, "{'name': 'param1','value': '%d','type': 'text' }", param1);
			fprintf(cgiOut, ",{'name': 'param2','value': '%d','type': 'text' }", param2);
			fprintf(cgiOut, ",{'name': 'param3','value': '%d','type': 'text' }", param3);
			fprintf(cgiOut, ",{'name': 'param4','value': '%d','type': 'text' }", param4);
			fprintf(cgiOut, ",{'name': 'param5','value': '%d','type': 'text' }]);\n", param5);
			break;
		}

		case ACTION_VIDEOADVANCEDSET_SET: {
			int param1 = 0;
			int param2 = 0;
			int param3 = 0;
			int param4 = 0;
			int param5 = 0;
			H264EncodeParam h264EncodeParamIn;
			H264EncodeParam h264EncodeParamOut;
			memset(&h264EncodeParamIn, 0, sizeof(H264EncodeParam));
			memset(&h264EncodeParamOut, 0, sizeof(H264EncodeParam));
			cgiFormInteger("param1", &param1, 0);
			cgiFormInteger("param2", &param2, 0);
			cgiFormInteger("param3", &param3, 0);
			cgiFormInteger("param4", &param4, 0);
			cgiFormInteger("param5", &param5, 0);
			h264EncodeParamIn.param1 = param1;
			h264EncodeParamIn.param2 = param2;
			h264EncodeParamIn.param3 = param3;
			h264EncodeParamIn.param4 = param4;
			h264EncodeParamIn.param5 = param5;
			//webSetH264encodeParam(&h264EncodeParamIn, &h264EncodeParamOut);
			fprintf(cgiOut, "%d", RESULT_SUCCESS);
			break;
		}

		case PAGE_ADDOUTPUT_SHOW: {
			showPage("./output_addSet.html", sys_language);
			break;
		}

		case ACTION_DOWNLOAD_SDP: {
			char  buff[4096];
			int total_len = 0;
			int w_len = 0;
			//int  n;
			//FILE  *fp;
		//	webSignalDetailInfo(buff,1024);
			webGetSdpInfo(buff, 10);  

			total_len = strlen(buff);
			fprintf(cgiOut, "Content-Disposition:attachment;filename=encode.sdp\n\n");
			/*fp=fopen("./output_addSet.html", "r" );
			while ((n=fread(&buff, sizeof (char),65535,fp))>0){
				fwrite(buff, 1, n, cgiOut);
			}
			fclose(fp);*/
			#if 1
			while(total_len>0)
			{
				w_len = fwrite(buff, 1, total_len -w_len, cgiOut);
				total_len -= w_len;
				if(w_len <= 0)
					break;
			}
			#endif
			//fwrite(buff, 1, total_len -w_len, cgiOut);
			
			break;
		}

		default:
			{
				showPage("./login.html",sys_language);
			}
		break;

}
	return 0;

}
	fflush(stdout);
