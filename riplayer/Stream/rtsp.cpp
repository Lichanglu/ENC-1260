
#include "stream.h"
#include <string.h>


#define BUF_SIZE 4098
/*
 * constants
 */

const char rtsp_protocol_version[]="RTSP/1.0";

/* server states */
#define RTSP_CONNECTED 1
#define RTSP_INIT      2
#define RTSP_READY     4
#define RTSP_PLAYING   8
#define RTSP_RECORDING 16

/* server capabilities */
#define RTSP_OPTIONS       0x001
#define RTSP_DESCRIBE      0x002
#define RTSP_ANNOUNCE      0x004
#define RTSP_SETUP         0x008
#define RTSP_GET_PARAMETER 0x010
#define RTSP_SET_PARAMETER 0x020
#define RTSP_TEARDOWN      0x040
#define RTSP_PLAY          0x080
#define RTSP_RECORD        0x100

#define VLC_UNUSED(x) (void)(x)



static char *rtsp_get( rtsp_client_t *rtsp );
static char *net_gets( SOCKET fd);
static int rtsp_read_line(SOCKET fd,char *p_buffer,int i_buffer);
static int reach_rtsp_tcp_connect(char *ip,unsigned short port);
static int reach_rtsp_request_describe( rtsp_client_t *rtsp, const char *what );
static int app_rtsp_get_host(char *ip);
static rtsp_client_t *reach_rtsp_init();
static int reach_rtsp_connect( rtsp_client_t *rtsp, const char *psz_mrl, const char *psz_user_agent );
static void rtsp_close( rtsp_client_t *rtsp );
static void rtsp_free_answers( rtsp_client_t *rtsp );
static void rtsp_unschedule_all( rtsp_client_t *rtsp );
static void rtsp_set_session( rtsp_client_t *rtsp, const char *id );
static char *rtsp_get_session( rtsp_client_t *rtsp );
static char *rtsp_get_mrl( rtsp_client_t *rtsp );
static void rtsp_schedule_field( rtsp_client_t *rtsp, const char *string );
static void rtsp_unschedule_field( rtsp_client_t *rtsp, const char *string );
static int rtsp_request_options( rtsp_client_t *rtsp, const char *what );
static int rtsp_send_request( rtsp_client_t *rtsp, const char *psz_type, const char *psz_what );
static int rtsp_put( rtsp_client_t *rtsp, const char *psz_string );
static int WriteData(SOCKET s ,void *pBuf,int nSize);
static int mid_recv(SOCKET s,char *buff,int len,int flags);
static int rtsp_get_answers( rtsp_client_t *rtsp );
static int rtsp_get_status_code( rtsp_client_t *rtsp, const char *psz_string );
static void rtsp_schedule_standard( rtsp_client_t *rtsp );
static int rtsp_request_describe( rtsp_client_t *rtsp, const char *what );
static int rtsp_request_setup( rtsp_client_t *rtsp, const char *what );
static int rtsp_request_setparameter( rtsp_client_t *rtsp, const char *what );
static int rtsp_request_getparameter( rtsp_client_t *rtsp, const char *what );
static int rtsp_request_play( rtsp_client_t *rtsp, const char *what );
static int rtsp_request_tearoff( rtsp_client_t *rtsp, const char *what );
static int rtsp_recv_sdp_info(rtsp_client_t *rtsp,char *buff,int len);
static int rtsp_rtp_recv_start(RTSP_HANDLE *rtsp_handle);
static int rtsp_rtp_recv_stop(RTSP_HANDLE *rtsp_handle);
static void rtsp_parse_thread(void *rtsp_handle2);


/*
 * rtsp_get gets a line from stream
 * and returns a null terminated string (must be freed).
 */
 
static char *rtsp_get( rtsp_client_t *rtsp )
{
  char *psz_buffer = (char*)malloc( BUF_SIZE );
  char *psz_string = NULL;

	rtsp_t *info = rtsp->p_private;
	SOCKET s = info->s;
		
	
  if( rtsp_read_line(s,psz_buffer, (unsigned int)BUF_SIZE ) >= 0 )
  {
  	//PRINTF("rtsp_get:psz_string=%s\n",psz_string);
    //printf( "<< '%s'\n", psz_buffer );
      psz_string = strdup( psz_buffer );
  }

  free( psz_buffer );
  return psz_string;
}


static char *net_gets( SOCKET fd)
{
    char *psz_line = NULL, *ptr = NULL;
    size_t  i_line = 0, i_max = 0;


    for( ;; )
    {
        if( i_line == i_max )
        {
            i_max += 1024;
         //   psz_line = (char*)realloc( psz_line, i_max );
		psz_line = (char *)malloc(i_max);
		memset(psz_line,0,i_max);
            ptr = psz_line + i_line;

	    
        }
		//PRINTF("ptr = %p,psz_line=%p,i_max=%d\n",ptr,psz_line,i_max);
        if( mid_recv( fd, ptr, 1,0) != 1 )
        {
            if( i_line == 0 )
            {
                free( psz_line );
                return NULL;
            }
            break;
        }
	//	PRINTF("net_gets:ptr=s\n",ptr);
        if ( *ptr == '\n' )
            break;

        i_line++;
        ptr++;
    }

    *ptr-- = '\0';

    if( ( ptr >= psz_line ) && ( *ptr == '\r' ) )
        *ptr = '\0'; 
 
	PRINTF("net_gets:recv=%s\n",psz_line);
    return psz_line;
}



static int rtsp_read_line(SOCKET fd,char *p_buffer,int i_buffer)
{
	char *psz = net_gets(fd);
	if(psz)
		strncpy(p_buffer,psz,i_buffer);
	else
	 *p_buffer = 0;
	 
	 free(psz);
	 
	 return 0;
	
}



static int reach_rtsp_tcp_connect(char *ip,unsigned short port)
{
	SOCKET client_socket = 0;
	struct sockaddr_in client_addr;
	int ret=0;
	 	unsigned long ul = 0;  //设置成原来的阻塞模式
	 	
	WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,1),&wsaData)) //调用Windows Sockets DLL
	    { 
	         PRINTF("reach_rtsp_tcp_connect:Winsock ERROR \n");
	         WSACleanup();
	         return -1;
	    }

	client_socket = socket(AF_INET,SOCK_STREAM , 0);
			
	memset(&client_addr,0 , sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(port);
	client_addr.sin_addr.s_addr = inet_addr(ip);
	
	struct	 timeval   timeout={5000,0};
	setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, ( char * )&timeout, sizeof( timeout ) ); 
		

	ret = connect(client_socket , (struct sockaddr *)&client_addr ,sizeof(struct sockaddr)); 
	if(ret < 0)
	{
		PRINTF("reach_rtsp_tcp_connect:connect ERROR!\n");
		closesocket(client_socket);
		client_socket = -1;
		return ret;;
	}
	ioctlsocket(client_socket, FIONBIO, (unsigned long*)&ul);
	 
	return client_socket;
}


static int reach_rtsp_request_describe( rtsp_client_t *rtsp, const char *what )
{
    char *buf;

    if( what )
    {
        buf = strdup(what);
    }
    else
    {
        buf = (char*)malloc( strlen(rtsp->p_private->host) +
                      strlen(rtsp->p_private->path) + 16 );
        sprintf( buf, "rtsp://%s:%i/%s", rtsp->p_private->host,
                 rtsp->p_private->port, rtsp->p_private->path );
    }
    PRINTF("DESCRIBR=#%s#\n",buf);
    rtsp_send_request( rtsp, "DESCRIBE", buf );
    free( buf );

    return rtsp_get_answers( rtsp );
}

static int app_rtsp_get_host(char *ip)
{
	strcpy(ip,"192.168.4.30");
	return 0;
}

static rtsp_client_t *reach_rtsp_init()
{
	rtsp_client_t *client = NULL;
	client = (rtsp_client_t *)malloc(sizeof(rtsp_client_t));
	if(client == NULL)
		return NULL;
	memset(client,0,sizeof(rtsp_client_t));
		 
	return client;
}



/*
 * connect to a rtsp server
 */

static int reach_rtsp_connect( rtsp_client_t *rtsp, const char *psz_mrl,
                  const char *psz_user_agent )
{
    rtsp_t *s = NULL;
	int ret = 0;
    char *mrl_ptr;
    char *slash, *colon;
    unsigned int hostend, pathbegin, i;

    if( !psz_mrl ) return -1;
    s = (rtsp_t*)malloc( sizeof(rtsp_t) );
    rtsp->p_private = s;

    if( !strncmp( psz_mrl, "rtsp://", 7 ) ) psz_mrl += 7;
    mrl_ptr = strdup( psz_mrl );

    for( i=0; i<MAX_FIELDS; i++ )
    {
        s->answers[i]=NULL;
        s->scheduled[i]=NULL;
    }

	s->s = 0;
    s->host = NULL;
    s->port = 554; /* rtsp standard port */
    s->path = NULL;
    s->mrl  = strdup(psz_mrl);

    s->server = NULL;
    s->server_state = 0;
    s->server_caps = 0;

    s->cseq = 0;
    s->session = NULL;

    if( psz_user_agent ) 
    		s->user_agent = strdup( psz_user_agent );
    else 
    		s->user_agent = strdup( "User-Agent: RealMedia Player Version "
                                 "6.0.9.1235 (linux-2.0-libc6-i386-gcc2.95)" );

    slash = strchr( mrl_ptr, '/' );
    colon = strchr( mrl_ptr, ':' );

    if( !slash ) slash = mrl_ptr + strlen(mrl_ptr) + 1;
    if( !colon ) colon = slash;
    if( colon > slash ) colon = slash;

    pathbegin = slash - mrl_ptr;
    hostend = colon - mrl_ptr;

    s->host = (char*)malloc(hostend+1);
    strncpy( s->host, mrl_ptr, hostend );
    s->host[hostend] = 0;

    if( pathbegin < strlen(mrl_ptr) )
     	s->path = strdup(mrl_ptr+pathbegin+1);
    else
    	s->path =strdup("/");
    if( colon != slash )
    {
        char* buffer = (char*)malloc(pathbegin-hostend);

        strncpy( buffer, mrl_ptr+hostend+1, pathbegin-hostend-1 );
        buffer[pathbegin-hostend-1] = 0;
        s->port = atoi(buffer);
        if( s->port < 0 || s->port > 65535 ) s->port = 554;
		free(buffer);
		buffer=NULL;
    }

    free( mrl_ptr );
	PRINTF( "reach_rtsp_connect: got mrl=%s %i %s\n", s->host, s->port, s->path );

	ret = reach_rtsp_tcp_connect( s->host, s->port );

    if( ret < 0)
    {
       PRINTF( "reach_rtsp_connect : rtsp: failed to connect to '%s'\n", s->host);
      //  rtsp_close( rtsp );
        return -1;
    }	
    s->s = (SOCKET)ret;

    s->server_state = RTSP_CONNECTED;

    /* now lets send an options request. */
    rtsp_schedule_field( rtsp, "CSeq: 2");
    //rtsp_schedule_field( rtsp, s->user_agent);
   // rtsp_schedule_field( rtsp, "ClientChallenge: "
    //                           "9e26d33f2984236010ef6253fb1887f7");
 //   rtsp_schedule_field( rtsp, "PlayerStarttime: [28/03/2003:22:50:23 00:00]");
    rtsp_schedule_field( rtsp, "CompanyID: szreach" );
   // rtsp_schedule_field( rtsp, "GUID: 00000000-0000-0000-0000-000000000000" );
  //  rtsp_schedule_field( rtsp, "RegionData: 0" );
 //   rtsp_schedule_field( rtsp, "ClientID: "
//                               "Linux_2.4_6.0.9.1235_play32_RN01_EN_586" );
    /*rtsp_schedule_field( rtsp, "Pragma: initiate-session" );*/
    
    Sleep(2000);
    ret = rtsp_request_options( rtsp, NULL );
	PRINTF( "rtsp_request_options: ret=%d\n", ret );

	if(ret == 200)
    	return 0;
	else
		return -1;
}

/*
 * closes an rtsp connection
 */

static void rtsp_close( rtsp_client_t *rtsp )
{
    if( rtsp->p_private->server_state )
    {
		if(rtsp->p_private->s != 0){
	    	closesocket( rtsp->p_private->s);
			rtsp->p_private->s=0;
		}
        /* TODO: send a TEAROFF */
//	 rtsp->pf_disconnect( rtsp->p_userdata );
    }
		if(rtsp->p_private != NULL)
		{
		if( rtsp->p_private->path  != NULL)
   	 		free( rtsp->p_private->path );
   	 	if( rtsp->p_private->host  != NULL)	
    		free( rtsp->p_private->host );
    	if( rtsp->p_private->mrl  != NULL)
    		free( rtsp->p_private->mrl );
    	if( rtsp->p_private->session  != NULL)
    		free( rtsp->p_private->session );
    	if( rtsp->p_private->user_agent  != NULL)
   	 		free( rtsp->p_private->user_agent );
   	 	if( rtsp->p_private->server  != NULL)
    		free( rtsp->p_private->server );
    }
    rtsp_free_answers( rtsp );
    rtsp_unschedule_all( rtsp );
    free( rtsp->p_private );
}

/*
 * free answers
 */

static void rtsp_free_answers( rtsp_client_t *rtsp )
{
    char **answer;

    if( !rtsp->p_private->answers ) return;
    answer = rtsp->p_private->answers;

    while( *answer )
    {
        free( *answer );
        *answer = NULL;
        answer++;
    }
}

/*
 * unschedule all fields
 */

static void rtsp_unschedule_all( rtsp_client_t *rtsp )
{
    char **ptr;

    if( !rtsp->p_private->scheduled ) return;
    ptr = rtsp->p_private->scheduled;

    while( *ptr )
    {
        free( *ptr );
        *ptr = NULL;
        ptr++;
    }
}


/*
 * session id management
 */

static void rtsp_set_session( rtsp_client_t *rtsp, const char *id )
{
    free( rtsp->p_private->session );
    rtsp->p_private->session = strdup(id);
}

static char *rtsp_get_session( rtsp_client_t *rtsp )
{
    return rtsp->p_private->session;
}

static char *rtsp_get_mrl( rtsp_client_t *rtsp )
{
    return rtsp->p_private->mrl;
}

/*
 * schedules a field for transmission
 */

static void rtsp_schedule_field( rtsp_client_t *rtsp, const char *string )
{
    int i = 0;

    if( !string ) return;

    while( rtsp->p_private->scheduled[i] ) i++;

    rtsp->p_private->scheduled[i] = strdup(string);
}

/*
 * removes the first scheduled field which prefix matches string.
 */

static void rtsp_unschedule_field( rtsp_client_t *rtsp, const char *string )
{
    char **ptr = rtsp->p_private->scheduled;

    if( !string ) return;

    while( *ptr )
    {
      if( !strncmp(*ptr, string, strlen(string)) ) break;
    }
    free( *ptr );
    ptr++;
    do
    {
        *(ptr-1) = *ptr;
    } while( *ptr );
}






/*
 * implementation of must-have rtsp requests; functions return
 * server status code.
 */

static int rtsp_request_options( rtsp_client_t *rtsp, const char *what )
{
    char *buf;

    if( what ) buf = strdup(what);
    else
    {
        buf =(char*) malloc( strlen(rtsp->p_private->host) + 16 );
        sprintf( buf, "rtsp://%s:%i", rtsp->p_private->host,
                 rtsp->p_private->port );
    }
	PRINTF("rtsp_request_options:buf=%s\n",buf);
    rtsp_send_request( rtsp, "OPTIONS", buf );
    free( buf );

    return rtsp_get_answers( rtsp );
}


/*
 * send a request
 */

static int rtsp_send_request( rtsp_client_t *rtsp, const char *psz_type,
                              const char *psz_what )
{
    char **ppsz_payload = rtsp->p_private->scheduled;
    char *psz_buffer;
    int i_ret;

    psz_buffer =(char*) malloc( strlen(psz_type) + strlen(psz_what) +
                         sizeof("RTSP/1.0") + 2 );

    sprintf( psz_buffer, "%s %s %s", psz_type, psz_what, "RTSP/1.0" );
    i_ret = rtsp_put( rtsp, psz_buffer );
    free( psz_buffer );
		

    if( ppsz_payload ){		
        PRINTF("rtsp_send_request:pp sz_payload = %s\n",*ppsz_payload);
        while( *ppsz_payload )
        {
            rtsp_put( rtsp, *ppsz_payload );
            ppsz_payload++;
        }
    }
        
        
 	if(rtsp->p_private->user_agent != NULL && strlen(rtsp->p_private->user_agent) != 0)
		rtsp_put(rtsp,rtsp->p_private->user_agent);
			
			       
    rtsp_put( rtsp, "" );
    rtsp_unschedule_all( rtsp );
	PRINTF("rtsp_send_request:rtsp_send_request END\n");
    return i_ret;
}



/*
 * rtsp_put puts a line on stream
 */

static int rtsp_put( rtsp_client_t *rtsp, const char *psz_string )
{
    unsigned int i_buffer = strlen( psz_string );
    char *psz_buffer =(char*) malloc( i_buffer + 3 );
    int i_ret;
	rtsp_t *info = rtsp->p_private;
	int s = info->s;

	memset(psz_buffer,0,i_buffer + 3);
	  
    strcpy( psz_buffer, psz_string );
    psz_buffer[i_buffer] = '\r'; psz_buffer[i_buffer+1] = '\n';
    psz_buffer[i_buffer+2] = 0;
	
	PRINTF("rtsp_put:send:%s\n",psz_buffer);

	i_ret=	WriteData(s,(uint8_t*)psz_buffer, i_buffer + 2 );
  //  i_ret = rtsp->pf_write( rtsp->p_userdata, (uint8_t*)psz_buffer, i_buffer + 2 );

	if(i_buffer+2 != i_ret)
		PRINTF("rtsp_put:len=%d,iret=%d,psz_buffer =#\n%s#\n",i_buffer+2,i_ret,psz_buffer);
		
    free( psz_buffer );
    return i_ret;
}


static int WriteData(SOCKET s ,void *pBuf,int nSize)
{
	int nWriteLen = 0;
	int nRet = 0;
	int nCount = 0;
	while(nWriteLen < nSize){
		nRet = send(s,(char *)pBuf+nWriteLen,nSize-nWriteLen,0);
		if(nRet < 0){
			if((errno == WSAENOBUFS) && (nCount < 10)){
				PRINTF("WriteData:network buffer have been full!\n");
				Sleep(10);
				nCount++;
				continue;
			}
			return nWriteLen;
		}
		else if(nRet == 0){
			PRINTF("WriteData:Send Net Data Error nRet = %d\n",nRet);
			continue;
		}
		nWriteLen += nRet;
		nCount = 0;
	}		
	return nWriteLen;
}


static int mid_recv(SOCKET s,char *buff,int len,int flags)
{
	int toplen = 0;
	int readlen = 0;

	while(len - toplen > 0)
	{
		readlen =  recv(s, buff+toplen,len - toplen,flags);
		if(len - toplen != 1)
			PRINTF("mid_recv:S=%d,buff = %p==%d\n",s,buff,readlen);
		if(readlen < 0)
		{
			PRINTF("mid_recv:recv ERROR\n");
			return -1;
		}else if(readlen == 0){
			PRINTF("mid_recv:socket close!\n");
			break;
		}
		if(readlen != len - toplen)
		{
			PRINTF("mid_recv:WARNNING,i read the buff len = %d,i need len = %d\n",readlen,len);
		}
		toplen += readlen;
	}
	return toplen;
}

/*
 * get the answers, if server responses with something != 200, return NULL
 */

static int rtsp_get_answers( rtsp_client_t *rtsp )
{
    char *answer = NULL;
    unsigned int answer_seq;
    char **answer_ptr = rtsp->p_private->answers;
    int code;
    int ans_count = 0;

		int sdp_len = 0;
		
    answer = rtsp_get( rtsp );
    if( !answer ) return 0;
    code = rtsp_get_status_code( rtsp, answer );
    free( answer );
	PRINTF("rtsp_get_answers:code =%d\n",code);
    rtsp_free_answers( rtsp );

    do { /* while we get answer lines */

      answer = rtsp_get( rtsp );
	 PRINTF("rtsp_get_answers:answer=#%s#\n",answer); 
      if( !answer ) return 0;

      if( !strncmp( answer, "Cseq:", 5 ) )
      {
          sscanf( answer, "%*s %u", &answer_seq );
          if( rtsp->p_private->cseq != answer_seq )
          {
            //fprintf( stderr, "warning: Cseq mismatch. got %u, assumed %u",
            //       answer_seq, rtsp->p_private->cseq );

              rtsp->p_private->cseq = answer_seq;
          }
      }
      if( !strncmp( answer, "Server:", 7 ) )
      {
          char *buf = (char*)malloc( strlen(answer) );
          sscanf( answer, "%*s %s", buf );
          free( rtsp->p_private->server );
          rtsp->p_private->server = buf;
      }
      if( !strncmp( answer, "Content-Length:", strlen("Content-Length:")) )
      {
					sdp_len = atoi(answer + strlen("Content-Length:"));
          rtsp->p_private->sdp_len = sdp_len;
          PRINTF("rtsp_get_answers:sdp_len=%d\n",sdp_len);
       }
    
      if( !strncmp( answer, "Session:", 8 ) )
      {
          char *buf = (char*)malloc( strlen(answer) );
          sscanf( answer, "%*s %s", buf );
          if( rtsp->p_private->session )
          {
              if( strcmp( buf, rtsp->p_private->session ) )
              {
                  //fprintf( stderr,
                  //         "rtsp: warning: setting NEW session: %s\n", buf );
                  free( rtsp->p_private->session );
                  rtsp->p_private->session = strdup( buf );
              }
          }
          else
          {
              //fprintf( stderr, "setting session id to: %s\n", buf );
              rtsp->p_private->session = strdup( buf );
          }
          free( buf );
      }

      *answer_ptr = answer;
      answer_ptr++;
    } while( (strlen(answer) != 0) && (++ans_count < MAX_FIELDS) );

    rtsp->p_private->cseq++;

    *answer_ptr = NULL;
    rtsp_schedule_standard( rtsp );

    return code;
}


/*
 * extract server status code
 */

static int rtsp_get_status_code( rtsp_client_t *rtsp, const char *psz_string )
{
    VLC_UNUSED(rtsp);
    char psz_buffer[4];
    int i_code = 0;

    if( !strncmp( psz_string, "RTSP/1.0", sizeof("RTSP/1.0") - 1 ) )
    {
        memcpy( psz_buffer, psz_string + sizeof("RTSP/1.0"), 3 );
        psz_buffer[3] = 0;
        i_code = atoi( psz_buffer );
    }
    else if( !strncmp( psz_string, "SET_PARAMETER", sizeof("SET_PARAMETER") - 1 ) )
    {
        return RTSP_STATUS_SET_PARAMETER;
    }
	PRINTF("rtsp_get_status_code:psz_buffer=%s\n",psz_buffer);

    if( i_code != 200 )
    {
        PRINTF("librtsp: server responds: '%s'\n", psz_string );
    }

    return i_code;
}


/*
 * schedule standard fields
 */

static void rtsp_schedule_standard( rtsp_client_t *rtsp )
{
    char tmp[17];

    sprintf( tmp, "CSeq: %u", rtsp->p_private->cseq);
    rtsp_schedule_field( rtsp, tmp );

    if( rtsp->p_private->session )
    {
        char *buf;
        buf =(char*) malloc( strlen(rtsp->p_private->session) + 15 );
        sprintf( buf, "Session: %s", rtsp->p_private->session );
        rtsp_schedule_field( rtsp, buf );
        free( buf );
    }
}

static int rtsp_request_describe( rtsp_client_t *rtsp, const char *what )
{
    char *buf;

    if( what )
    {
        buf = strdup(what);
    }
    else
    {
        buf =(char*) malloc( strlen(rtsp->p_private->host) +
                      strlen(rtsp->p_private->path) + 16 );
        sprintf( buf, "rtsp://%s:%i/%s", rtsp->p_private->host,
                 rtsp->p_private->port, rtsp->p_private->path );
    }
    rtsp_send_request( rtsp, "DESCRIBE", buf );
    free( buf );

    return rtsp_get_answers( rtsp );
}

static int rtsp_request_setup( rtsp_client_t *rtsp, const char *what )
{
    rtsp_send_request( rtsp, "SETUP", what );
    return rtsp_get_answers( rtsp );
}

static int rtsp_request_setparameter( rtsp_client_t *rtsp, const char *what )
{
    char *buf;

    if( what )
    {
        buf = strdup(what);
    }
    else
    {
        buf =(char*) malloc( strlen(rtsp->p_private->host) +
                      strlen(rtsp->p_private->path) + 16 );
        sprintf( buf, "rtsp://%s:%i/%s", rtsp->p_private->host,
                 rtsp->p_private->port, rtsp->p_private->path );
    }

    rtsp_send_request( rtsp, "SET_PARAMETER", buf );
    free( buf );

    return rtsp_get_answers( rtsp );
}


static int rtsp_request_getparameter( rtsp_client_t *rtsp, const char *what )
{
    char *buf;

    if( what )
    {
        buf = strdup(what);
    }
    else
    {
        buf = (char*)malloc( strlen(rtsp->p_private->host) +
                      strlen(rtsp->p_private->path) + 16 );
        sprintf( buf, "rtsp://%s:%i/%s", rtsp->p_private->host,
                 rtsp->p_private->port, rtsp->p_private->path );
    }

    rtsp_send_request( rtsp, "GET_PARAMETER", buf );
    free( buf );

    return rtsp_get_answers( rtsp );
}



static int rtsp_request_play( rtsp_client_t *rtsp, const char *what )
{
    char *buf;

    if( what )
    {
        buf = strdup( what );
    }
    else
    {
        buf =(char*)malloc( strlen(rtsp->p_private->host) +
                      strlen(rtsp->p_private->path) + 16 );
        sprintf( buf, "rtsp://%s:%i/%s", rtsp->p_private->host,
                 rtsp->p_private->port, rtsp->p_private->path );
    }

    rtsp_send_request( rtsp, "PLAY", buf );
    free( buf );

    return rtsp_get_answers( rtsp );
}

static int rtsp_request_tearoff( rtsp_client_t *rtsp, const char *what )
{
    rtsp_send_request( rtsp, "TEAROFF", what );
    return rtsp_get_answers( rtsp );
}


static int rtsp_recv_sdp_info(rtsp_client_t *rtsp,char *buff,int len)
{
	rtsp_t *info = rtsp->p_private;
	if(info->sdp_len <= 0)
		return -1;
	int len2 = mid_recv(info->s, buff,info->sdp_len,0);
	PRINTF("rtsp_recv_sdp_info:info->sdp_len =%d\n",info->sdp_len);	
	if(len2 != info->sdp_len)
	{
		PRINTF("rtsp_recv_sdp_info:Error,len=%d,sdp_len=%d\n",len,info->sdp_len);
		return -1;
	}
	if(buff)
		PRINTF("rtsp_recv_sdp_info:sdp_inof:%s\n",buff);
	return 0;
}

//open the sdp info
static int rtsp_rtp_recv_start(RTSP_HANDLE *rtsp_handle)
{
	char sdp_info[2048] = {0};
	if(rtsp_handle == NULL || strlen(rtsp_handle->sdp_info) == 0)
	{
		PRINTF("rtsp_rtp_recv_start:the sdp info ERROR\n");
		return -1;
	}
	SDP_Info* info=parse_sdp(rtsp_handle->sdp_info,strlen(rtsp_handle->sdp_info));
	if( !info )
	{
		PRINTF("rtsp_rtp_recv_start:sdp_info NULL\n");
		return -1;
	}
	PRINTF("rtsp_rtp_recv_start:info->video=%d,audio=%d,config=0x%x,IP=%s,info->type=%d\n",
		info->video_port,info->audio_port,info->config,info->IP,info->type);
	
	/****** video ********/
	rtsp_handle->video_handle = rtp_video_open_url(info);
	if(rtsp_handle->video_handle == NULL)
		{
		PRINTF("rtsp_rtp_recv_start:rtsp_handle->video_handle  NULL\n");
		return -1;
	}else{
		rtsp_handle->video_handle->fCCEvent = (Msg_func)rtsp_handle->fCCEvent;
		if(rtsp_handle->fGetVideoEs)
			rtsp_handle->video_handle->fGetVideoEs = (VideoEsCallBack)rtsp_handle->fGetVideoEs;
	}
	/******* audio *********/
	rtsp_handle->audio_handle= rtp_audio_open_url(info);
	if(rtsp_handle->audio_handle == NULL)
		{
		PRINTF("rtsp_rtp_recv_start:rtsp_handle->audio_handle  NULL\n");
		return -1;
	}else {
		rtsp_handle->audio_handle->fCCEvent = (Msg_func)rtsp_handle->fCCEvent;
		if(rtsp_handle->fGetAudioEs)
			rtsp_handle->audio_handle->fGetVideoEs = (VideoEsCallBack)rtsp_handle->fGetAudioEs;
		}
	free(info);
	info=NULL;
	return 0;
}

static int rtsp_rtp_recv_stop(RTSP_HANDLE *rtsp_handle)
{
	if(rtsp_handle == NULL)
	{
		PRINTF("rtsp_rtp_recv_stop:the sdp info ERROR\n");
		return -1;
	}		
	
	if(rtsp_handle->video_handle != NULL)
	{
		//rtp_set_msg_func(rtsp_handle->video_handle,NULL);
		rtp_close_stream(&(rtsp_handle->video_handle));
	}
	Sleep(1000);
	if(rtsp_handle->video_handle == NULL)
		PRINTF("rtsp_rtp_recv_stop:rtsp_handle->video_handle NULL");
	
	if(rtsp_handle->audio_handle){		
		//rtp_set_msg_func(rtsp_handle->audio_handle,NULL);
		rtp_close_stream(&(rtsp_handle->audio_handle ));
		}
	Sleep(1000);
	return 1;
}

static void rtsp_parse_thread(void *rtsp_handle2)
{
	SDP_Info* sdp_info=NULL;
	
		RTSP_HANDLE *rtsp_handle = (RTSP_HANDLE *)rtsp_handle2;
		
//		char url[1024] = {0};
		
		char buff[512] = {0};
		rtsp_client_t *handle = rtsp_handle->client;
		int ret = 0;
		int x = 0;
		if(handle == NULL)
		{
			PRINTF("rtsp_parse_thread:ERROR,the rtsp handle is NULL\n");
			goto MSG_EXIT;
		}

		ret = 	reach_rtsp_connect(handle, rtsp_handle->rtsp_url,"User-Agent: LibVLC/2.0.1 (LIVE555 Streaming Media v2011.12.23)" );
		if(ret != 0)
		{
			PRINTF("rtsp_parse_thread:ERROR!!!!!!!!! reach_rtsp_connect\n");
			goto MSG_EXIT;
		}
	
		PRINTF("rtsp_parse_thread:*** rtsp_request_describe ***\n");
		rtsp_schedule_field( handle, "Accept: application/sdp");	
		ret = rtsp_request_describe(handle,NULL);		
		if(ret != 200)
		{
			PRINTF("rtsp_parse_thread:rtsp_request_describe ERROR!\n");
			goto MSG_EXIT;
		}
		
		PRINTF("\nrtsp_parse_thread:*** rtsp_recv_sdp_info ***\n");
		ret =rtsp_recv_sdp_info(handle,rtsp_handle->sdp_info,sizeof(rtsp_handle->sdp_info));	
		if(ret != 0)
		{
			PRINTF("rtsp_parse_thread:rtsp_request_describe ERROR! \n");
			goto MSG_EXIT;
		}
		PRINTF("rtsp_parse_thread:sdp_info=#%s#",rtsp_handle->sdp_info);

		/** sdp info **/
		sdp_info = parse_sdp(rtsp_handle->sdp_info,strlen(rtsp_handle->sdp_info));
		if(sdp_info==NULL){
			PRINTF("rtsp_parse_thread:parse_sdp ERROR\n");
			goto MSG_EXIT; ;
		}
		if( sdp_info->video_port == 0 )
			sdp_info->video_port = 8436;
	
		if(sdp_info->audio_port == 0 )
			sdp_info->audio_port = sdp_info->video_port+2;
		
		PRINTF("rtsp_parse_thread:*** rtsp_request_setup ***\n");
		sprintf(buff,"Transport: RTP/AVP;unicast;client_port=%d-%d",
			sdp_info->video_port,sdp_info->video_port+1);
		rtsp_schedule_field( handle, buff);	
		memset(buff,0,sizeof(buff));
		sprintf(buff,"rtsp://%s:554/trackID=1",sdp_info->IP);
		ret = rtsp_request_setup(handle,buff); //"rtsp://192.168.4.161:554/trackID=1");
		if(ret != 200)
		{
			PRINTF("rtsp_parse_thread:rtsp_request_setup ERROR! \n");
			goto MSG_EXIT;
		}		
		
		PRINTF("rtsp_parse_thread:*** rtsp_request_setup ***\n");
		memset(buff,0,sizeof(buff));
		sprintf(buff,"Transport: RTP/AVP;unicast;client_port=%d-%d",
			sdp_info->audio_port,sdp_info->audio_port+1);
		rtsp_schedule_field( handle,buff);// "Transport: RTP/AVP;unicast;client_port=8502-8503");	
		memset(buff,0,sizeof(buff));
		sprintf(buff,"rtsp://%s:554/trackID=2",sdp_info->IP);
		ret = rtsp_request_setup(handle, buff);//"rtsp://192.168.4.161:554/trackID=2");	
		if(ret != 200)
		{
			PRINTF("rtsp_parse_thread: rtsp_request_setup ERROR!\n");
			goto MSG_EXIT;
		}

		PRINTF("rtsp_parse_thread:*** rtsp_request_play ***\n");
		rtsp_schedule_field( handle, "Range: npt=0.000-");	
		ret = rtsp_request_play(handle, NULL);			
		if(ret != 200)
		{
			PRINTF("rtsp_parse_thread:rtsp_request_play ERROR! \n");
			goto MSG_EXIT;
		}	
				
		//add rtp handle
		ret = rtsp_rtp_recv_start(rtsp_handle);
		if( ret < 0){
			PRINTF("rtsp_parse_thread:rtsp_rtp_recv_start ERROR! \n");
			goto MSG_EXIT;
			}
		

    while(1)
    {
    	
    	//if need stop
    	if(rtsp_handle ->status == 1 
			|| rtsp_handle->video_handle == NULL 
			|| rtsp_handle->audio_handle == NULL)
    	{
    		rtsp_rtp_recv_stop(rtsp_handle);
			Sleep(1000);
    		rtsp_request_tearoff(handle, "");
    		Sleep(1000);
    		break;
    	}
    	Sleep(3000);
    	if(x++ != DEFAULT_HEARTBIT_TIME/3)
    		continue;
  		x =0;
#if 0
		ret = 	rtsp_request_getparameter(handle, NULL);
		PRINTF("rtsp_request_play ret =%d\n",ret);	
		if(ret != 200)
		{
			PRINTF("ERROR! rtsp_request_play\n");
			rtsp_rtp_recv_stop( rtsp_handle);
			break;
		}
#endif
    }       	

EXIT:
	if(handle)
		rtsp_close(handle );          	
	if(rtsp_handle)
	{
		rtsp_handle->fCCEvent=NULL;
		rtsp_handle->fGetAudioEs=NULL;
		rtsp_handle->fGetVideoEs=NULL;
		free(rtsp_handle);
		rtsp_handle = NULL;
		}
	return;
MSG_EXIT:
	if(rtsp_handle != NULL  && rtsp_handle->fCCEvent)
		rtsp_handle->fCCEvent(RTSP_STREAM,MSG_RECV_FAIL);
	
	if(handle)
		rtsp_close(handle );          	
	if(rtsp_handle)
	{
		rtsp_handle->fCCEvent=NULL;
		rtsp_handle->fGetAudioEs=NULL;
		rtsp_handle->fGetVideoEs=NULL;
		free(rtsp_handle);
		rtsp_handle = NULL;
		}
	return;
}


void rtsp_set_video_output_func(RTSP_HANDLE *handle,void *func)
{
	if(handle != NULL)
	{
		handle->fGetVideoEs =(VideoEsCallBack) func;
	}
}

void rtsp_set_audio_output_func(RTSP_HANDLE *handle,void *func)
{
	if(handle != NULL)
	{
		handle->fGetAudioEs =(VideoEsCallBack) func;
	}	
}


void rtsp_set_msg_output_func(RTSP_HANDLE *handle,void *func)
{
	if(handle != NULL)
	{
		handle->fCCEvent = (Msg_func)func;
	}		
}

RTSP_HANDLE* rtsp_open_stream(char *url)
{
	RTSP_HANDLE *rtsp_handle = NULL;
	int ret =0;
	
	rtsp_handle = (RTSP_HANDLE *)malloc(sizeof(RTSP_HANDLE));
	if(rtsp_handle == NULL)
	{
			PRINTF("ERROR\n");
			return NULL;
	}
	memset(rtsp_handle,0,sizeof(RTSP_HANDLE));
	_snprintf(rtsp_handle->rtsp_url,sizeof(rtsp_handle->rtsp_url),"%s",url);
		
	rtsp_client_t *handle = reach_rtsp_init();
	if(handle == NULL)
	{
			PRINTF("ERROR\n");
			free(rtsp_handle);
			return NULL;		
	}
	rtsp_handle->client = handle;

	
	_beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))rtsp_parse_thread,rtsp_handle, 0, 0);
	
	return rtsp_handle;
}

void rtsp_close_stream(RTSP_HANDLE *handle)
{
	PRINTF("i will close %s.\n",handle->rtsp_url);
	if(handle != NULL)
	{
		handle ->status = 1;
		handle->fCCEvent=NULL;
	}
	if(handle->video_handle != NULL)
	{
		rtp_set_output_func(handle->video_handle,NULL);
		rtp_set_msg_func(handle->video_handle,NULL);
	}
	if(handle->audio_handle != NULL)
	{
		rtp_set_output_func(handle->audio_handle,NULL);
		rtp_set_msg_func(handle->audio_handle,NULL);
	}		
}

