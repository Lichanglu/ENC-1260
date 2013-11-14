#include "stream.h"


#define   SDP_FILE_NAME  "sdp"

/*
~{EP6O~}ts  or~{!!~}rtp ~{Aw~}
0 = rtp
1 = ts
2 = rtp_ts
-1 = err
*/
INT32 parse_url(char* url,char *addr,unsigned short *port,char *multicast)
{
	char temp[32];
	int i ,j=0;
	int ret =  -1;
	int val=0;
	int colon_cnt = 0;
	
	if( !url)
		return -1;
	
	
	for(i=0,j=0;url[i] !='\0';i++){
		if( url[i] == ':')
			colon_cnt++;
	}
	
	if( colon_cnt < 2  ){
		return -1;
		}

	//~{=bNvP-Ri~}
	memset(temp,0,sizeof(temp));
	for(j=0,i=0;url[i] != ':';i++,j++)
		temp[j]=url[i];
	if( (0 == strncmp(temp,"rtp",3)) || (0 == strncmp(temp,"RTP",3)))
		ret = RTP_TS_STREAM;//  1 == rtp
	else if( (0 == strncmp(temp,"udp",3)  )|| ( 0 == strncmp(temp,"UDP",3)) ){
		ret =  TS_STREAM; // ts
		}
	else if( (0 == strncmp(temp,"rtsp",4) )|| (0 ==  strncmp(temp,"RTSP",4)) ){
		ret = RTSP_STREAM;
	}else 
		return -1;

	if( url[i+3] == '@')
	{
		j=0;
		memset(temp,0,sizeof(temp));
		for(;url[i+j+4]!='.';j++)
			temp[j]=url[i+j+4];

		val = atoi(temp);
		if( val <= 239 && val >= 224 )
			*multicast = 1;//~{<SHkWi2%~}
		else 
			return -1;
	}else{
		if( TS_STREAM == ret)
			return -1;
	}
	//~{=bNv5XV7~}
	memset(temp,0,sizeof(temp));
	j=0;
	for(i+=4;url[i] !=':';i++,j++)
		temp[j]=url[i];
	temp[j+1]='\0';
	memcpy(addr,temp,sizeof(temp));
	
	//~{=bNv6K?Z~}
	memset(temp,0,sizeof(temp));
	j=0;
	for(i+=1;url[i]!='\0';i++,j++)
		temp[j]=url[i];
	temp[j+1]='\0';
	*port = (unsigned short)atoi(temp);

	if(ret == RTSP_STREAM ){
		if( *port != 554 && *port != 0)
			return -1;		
		else 
			*port = 554;
	}

	return ret ;

}

static int htoi(char s[])   
{   
    int i;   
    int n = 0;   
    if (s[0] == '0' && (s[1]=='x' || s[1]=='X'))   
    {   
        i = 2;   
    }   
    else  
    {   
        i = 0;   
    }   
    for (; (s[i] >= '0' && s[i] <= '9') ;++i)   
    {   
        if (tolower(s[i]) > '9')   
        {   
            n = 16 * n + (10 + tolower(s[i]) - 'a');   
        }   
        else  
        {   
            n = 16 * n + (tolower(s[i]) - '0');   
        }   
    }   
    return n;   
}

static int parse_com_line(char* buf,int buf_len,SDP_Info *info,int line)
{
	int i =0,j=0;
	char temp[16];
	int type=0;
	memset(temp,0,16);

	switch(line)
	{
	case 1:
	{
		while( j < buf_len - strlen("audio") ){
			if( 0 == strncmp(buf+j,"audio",strlen("audio"))){
				j+=strlen("audio")+1;
				for(i=0;buf[j] !=  ' ' ; i++,j++)
					temp[i]=buf[j];
				info->audio_port =atoi(temp);
				type=1;
				break;
			}else if(0 == strncmp(buf+j,"video",strlen("video"))){
				j+=strlen("video")+1;
				for(i=0;buf[j+i] !=  ' ' ; i ++)
					temp[i]=buf[j+i];
				info->video_port =atoi(temp);
		
				break;
			}
			j++;
		}
		while( 0 != strncmp(buf+j,"RTP/AVP",strlen("RTP/AVP")) ){
			if(j > buf_len - strlen("RTP/AVP") )
				return -1;
			j++;
		}
		j+=strlen("RTP/AVP")+1;
		for(i=0;buf[j] !=  '\0' ; i++,j++)
			temp[i]=buf[j];
		if(0 == info->type && 1==type)
			info->type =atoi(temp);
		printf("info->type=%d!! \n",info->type);
		
		break;
		}
	case 10:
		break;
	case 20:
	{	
		while( buf[j] != ':')
			j++;
		j++;
		sprintf(temp,"%d",info->type);
		//~{LaJ>4mNs~}
		if( 0 != strncmp(buf+j,temp,strlen(temp))){
			PRINTF("sdp_info type is difference \n");
		}
			
//		if(line < 4)
//			break;
		while( 0 != strncmp(buf+j,"config=",strlen("config="))  )
		{
			if( j > buf_len-strlen("config=")){
				return -1;
				//error
			}
			j++;
		}
		memset(temp,0,sizeof(temp));
		j += strlen("config=");
		while( buf[j] != ';' && j < buf_len){
			temp[i]=buf[j];
			i++;
			j++;
		}
		info->config = htoi(temp);
		break;
		}
	case 30:
	{
		char ip[16]={0};
		int k=0;
		while( j < buf_len - strlen("IP4")){
			if(0 == strncmp(buf+j,"IP4",strlen("IP4")))
					break;
			j++;
		}
		j += strlen("IP4")+1;
		
		i=0;
		memset(temp,0,sizeof(temp));
		
		while(buf[j] != 0 ){
			temp[i]=buf[j];
			i++;
			j++;
		}

		//~{9}BK~}IP~{PEO"~}
		i=0;
		k=0;
		while(temp[i] != '\0'){
			if((temp[i]<='9'&&temp[i]>='0')||temp[i] =='.'){
				ip[k]=temp[i];
				k++;
				}
			i++;
			}
		strncpy(info->IP,ip,k+1);
		PRINTF("parse_com_line:ip:%s\n",info->IP);
		break;
		}
	default:
		break;
	}
}

SDP_Info* parse_sdp(const char* buff,int buff_len)
{
	SDP_Info *sdp_info = NULL;
	int i=0,j=0,k=0;
	int ret= -1;
	int line =0;
	char temp[128];
	char cnt=0;
	int  cnt_temp=1;
	int fmtp_cnt=0,rtpmap_cnt=0,m_cnt=0;

	sdp_info=(SDP_Info*)malloc(sizeof(SDP_Info));
	if(sdp_info == NULL)
		return NULL;
	
	memset(sdp_info,0,sizeof(SDP_Info));
	

	k=0;		
	while( k < buff_len-strlen("m=") ){
		if(0 == strncmp(buff+k,"m=",strlen("m=")) )
			m_cnt++;
		k++;
	}

	k=0;		
	while( k < buff_len-strlen("a=fmtp") ){
		if(0 == strncmp(buff+k,"a=fmtp",strlen("a=fmtp")) )
			fmtp_cnt++;
		k++;
	}

	k=0;		
	while( k < buff_len-strlen("a=rtpmap") ){
		if(0 == strncmp(buff+k,"a=rtpmap",strlen("a=rtpmap")) )
			rtpmap_cnt++;
		k++;
	}

	if( m_cnt== 0 )
		goto _exit;

	PRINTF("parse_sdp:fmtp_cnt=%d,rtpmap_cnt=%d\n",fmtp_cnt,rtpmap_cnt);

	// m =
	for(cnt = 0 ;cnt < m_cnt ;cnt++){
		memset(temp,0,128);
		while( 0 != strncmp(buff+j,"m=",strlen("m="))  ){
			if( j > buff_len-strlen("m="))
				goto _exit;
			j++;
		}
		for(i=0,k=j;buff[k] != 0xa;i++,k++)
					temp[i]=buff[k];
		j+=i;
		line=1;
		parse_com_line(temp,i+1,sdp_info,line);
		
	}
	
	j=0;
	//a=rtpmap
	for(cnt=0;cnt<rtpmap_cnt;cnt++){
		line=10;
		memset(temp,0,sizeof(temp));
			
		while(0 != strncmp(buff+j,"a=rtpmap",strlen("a=rtpmap")) ){
			if( j > buff_len-strlen("a=rtpmap"))
				goto _exit;
			j++;
		}	
		for(i=0,k=j;buff[k] != 0xa;i++,k++)
					temp[i]=buff[k];
		j+=i;
		parse_com_line(temp,i+1,sdp_info,line);
		}
	
	//fmtp
	j=0;
	for(cnt=0;cnt < fmtp_cnt;cnt++){
		line=20;
		memset(temp,0,sizeof(temp));
		while(0 != strncmp(buff+j,"a=fmtp",strlen("a=fmtp")) ){
			if( j > buff_len-strlen("a=fmtp"))
				goto _exit;
			j++;
		}	
	
		for(i=0,k=j;buff[k] != 0xa;i++,k++)
					temp[i]=buff[k];
	
		j+=i;
		parse_com_line(temp,i+1,sdp_info,line);
	}

	j=0;
	line=30; 
	memset(temp,0,sizeof(temp));
	while(0 != strncmp(buff+j,"c=IN",strlen("c=IN")) ){
		//	printf("buf:%s\n",buff+j);
		if( j > buff_len-strlen("c=IN"))
			goto _exit;
		j++;
	}	
//	j+= strlen("c=IN IP4 ");
	for(i=0; buff[j] != 0x0a ;i++,j++)
				temp[i]=buff[j];
//	memset(sdp_info->IP,0,sizeof(sdp_info->IP));
	parse_com_line(temp,i+1,sdp_info,line);
	//~{EP6OJG7qWi2%~}
	if(sdp_info->IP[0] != 0 ){
		if(0 == strncmp((sdp_info->IP),"234",3))
			sdp_info->multicast = 1;
	}
	
	
	PRINTF("VEDIO _port = %d audio_port=%d\n",sdp_info->video_port,sdp_info->audio_port);

	return sdp_info;	
_exit:
	memset(&sdp_info,0,sizeof(sdp_info));
	return NULL;
}

