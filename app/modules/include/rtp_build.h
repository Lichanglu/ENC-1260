#ifndef _RTP_BUILD_H__
#define _RTP_BUILD_H__
/*
**************************************************************************************
��������:
1.void RtpInit()
    ���ܽ���:
        �ú�����Ҫ���RTP ���������ʼ��
            a.��ʼ����
            b.������Ƶ���������ڴ�ռ�
2.void RtpSetUdp(int UdpVideoSocket, struct sockaddr_in UdpVideoAddr,int UdpAudioSocket, struct sockaddr_in UdpAudioAddr)
    ���ܽ���:
        �ú�������RTP�鲥socket�͵�ַ���ؽ�ʱ��� 
            a.������Ƶ�鲥��socket�͵�ַ
            b.������Ƶ�鲥��socket�͵�ַ
            c.�ؽ�ʱ���
    ����˵��:
        UdpVideoSocket:��Ƶ�鲥socket
        UdpVideoAddr:��Ƶ�鲥��ַ
        UdpAudioSocket:��Ƶ�鲥socket
        UdpAudioAddr:��Ƶ�鲥��ַ
3.RtpVideoPack(int nLen, unsigned char *pData, int nFlag, unsigned char index)
    ���ܽ���:
        �ú�����Ҫ�����Ƶ��rtp���
    ����˵��:
        nLen:Ҫ�����264��Ƶ����
        pData:Ҫ�����264��Ƶ��ַ
        nFlag:֡��־��=1 I֡��=0 P֡
4.RtpAudioPack(int nLen, unsigned char * pData, int nFlag, unsigned char index)
    ���ܽ���:
        �ú�����Ҫ�����Ƶ��rtp���
    ����˵��:
        nLen:Ҫ�����aac��Ƶ����
        pData:Ҫ�����aac��Ƶ��ַ
5.void RtpExit()
    ���ܽ���:
        �˳� RTP Э��
            a.������
            b.�ͷŷ�����ڴ� 
6.char *RtpGetVersion()
    ���ܽ���:
        ��ȡ��ǰrtp �汾��
**************************************************************************************
*/
int rtp_build_audio_data(int nLen, unsigned char *pData,int samplerate,int mtu,unsigned int nowtime,void *info);
int rtp_build_video_data(int nLen, unsigned char *pData, int nFlag,int mtu,unsigned int nowtime,void *info);
void rtp_build_init();
void rtb_build_uninit();
int rtp_build_reset_time();


#endif

