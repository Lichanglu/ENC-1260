
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>



extern int ListenSeries(void *pParam);
/*
90 50 xx xx xx xx xx xx xx xx ff (����D70������λ����:D70->DSP��

a0 50 xx ff chksum��Ҫ��D70���ص�ǰλ������:PC->DSP��
      01 ����_��λ�ã�
      02 ����_��λ�ã�

b0 00 xx ff chksum���ⲿ��Ƶ�л���������:EXT->DSP��
      04 �� VGA  )
      01 �� ��ʦ ��
      03 �� ѧ�� ��
      02 �� ���� ��

08 09 08 xx���ⲿ��Ƶ�л���������:DSP->¼������)
         04 �� VGA  )
         01 �� ��ʦ ��
         03 �� ѧ�� ��
         02 �� ���� ��

c0 00 xx ff chksum���ⲿ�Զ����ֶ��л��������
      00 �� �Զ� ��
      01 �� �ֶ� ��
*/

#define TCONNECT	0xFF//���ӽ�ʦ������ѧ����
/*��ʦ���������͵ĺ�*/


/**
==============����˳��Ϊ��-����  ��-����  *  ==========================

(0,0)--------------------------
|                              |
|                              |
|                              |          ����������(4*4)����Ϊ��λ
|                              |
|                              |          ��:���߽���1����(28,128)( ���Ӧ����Ϊ��7,32��ת����ʮ��������Ϊ��0x07,0x20��
|                              |
|                              |
 ------------------------------(720,576)
*/
#define TCHRESET	0x00       //��λ          		��Чֵ:0xaa/0xbb          *(�ȷ�aa,�ٷ�bb)=�������ú���Ч״̬* 

#define TCHPOINTNUM	0x01       //���߽�����      ��Чֵ:(04)(03~0a)

#define TCHPOINT1X      0x02         // X(00)           ff     (00~ b4)
#define TCHPOINT1Y      0x03         // Y(20)           ff     (00~ 90)

#define TCHPOINT2X      0x04         // X(00)           ff
#define TCHPOINT2Y     	0x05          //Y(5a)           ff

#define TCHPOINT3X      0x06          //X(b2)           ff
#define TCHPOINT3Y      0x07         // Y(5a)           ff

#define TCHPOINT4X      0x08          //X(b2)           ff
#define TCHPOINT4Y      0x09          //Y(20)           ff

#define TCHPOINT5X      0x0A         // X               ff
#define TCHPOINT5Y      0x0B          //Y               ff 

#define TCHPOINT6X      0x0C         // X               ff
#define TCHPOINT6Y      0x0D          //Y               ff 

#define TCHPOINT7X      0x0E         // X               ff
#define TCHPOINT7Y      0x0F          //Y               ff 

#define TCHPOINT8X      0x10         // X               ff
#define TCHPOINT8Y      0x11          //Y               ff 

#define TCHPOINT9X      0x12          //X               ff
#define TCHPOINT9Y      0x13          //Y               ff 

#define TCHPOINT10X     0x14          //X               ff
#define TCHPOINT10Y     0x15          //Y               ff 

#define TCHDSTWIDTH     0x1E          //X(0C)           ff
#define TCHDSTHEIGHT    0x1F          //Y(0C)           ff


#define TCHVIDEO_HEAD    0x20          //X(b0)           ff
#define TCHVIDEO_ADDR    0x21          //X(00)           ff
#define TCHVIDEO_VGA     0x22          //X(04)           ff   
#define TCHVIDEO_TCH     0x23          //X(01)           ff 
#define TCHVIDEO_STD     0x24          //X(03)           ff 
#define TCHVIDEO_NONE    0x25          //X(02)           ff 


#define TCHVIDEO_1       0x26          //X(01)           ff    ��01~04����ʦ���½�̨����
#define TCHVIDEO_2       0x27          //X(01��          ff    ��01~04����ʱ������
#define TCHVIDEO_END     0x28          //X(ff)           ff  

#define TCHVIDEO_SENV    0x29          //X(64)           ff 

#define TCHVIDEO_DELAY   0x2f          //X(64)           ff 

#define TCHCENTX         0x30          //X(5a)           ff    
#define TCHCENTY       	 0x31         // Y(2d)           ff   

#define TCHPARITY        0x32          //X(00)           ff     (9600=0x00,1200=0x04,2400=0x06
// 4800=0x08,19200=0x0b,38400=0x0d)

#define TCHPROT       	 0x33          //X(00)           ff     (Visca = 00,Pelco_P = 01)      

#define TCHSPEEDUD       0x36          //X(14)           ff     (00=Stop,01=Max,30=Min)

#define TCHSPEEDLR       0x37          //X(10)           ff     (00=Stop,01=Max,30=Min)


#define TCHTURNUD        0x39         // X(00)           ff     (00=Stop,01,02,�ϡ�����)       

#define TCHTURNLR        0x3a          //X(01)           ff     (00=Stop,01,02,�󡶡���) 

#define TCHRSTLUX        0x3b          //X(46)           ff     (00~ b4)
#define TCHRSTLUY        0x3c          //X(10)           ff     (00~ 90)

#define TCHRSTRD         0x3d         // X(6e)           ff     (00~ b4)
//#define TCHRSTRD         0x3e         // X(1a)           ff     (00~ 90)

#define TCHWAITTIME      0x3f         // X(14)           ff     (���źŵ���ȫ���ȴ�ʱ�䣩
#define TCHTURNTIME      0x40          //X(0f)           ff     (�������Ԥ��λʱ�䣩


//#define TCHCENTY         0x60        //  X(20)           ff
//#define TCHCENTY         0x61         // X(03)           ff
//==============================================================================

/*ѧ�����������͵ĺ�*/
#define STDRESET	    0x00       //��λ          		��Чֵ:0xaa/0xbb          *(�ȷ�aa,�ٷ�bb)=�������ú���Ч״̬* 

#define STDPOINTNUM	    0x01       //���߽�����      ��Чֵ:(04)(03~0a)

#define STDPOINT1X      0x02         // X(00)           ff     (00~ b4)
#define STDPOINT1Y      0x03         // Y(20)           ff     (00~ 90)

#define STDPOINT2X      0x04         // X(00)           ff
#define STDPOINT2Y     	0x05          //Y(5a)           ff

#define STDPOINT3X      0x06          //X(b2)           ff
#define STDPOINT3Y      0x07         // Y(5a)           ff

#define STDPOINT4X      0x08          //X(b2)           ff
#define STDPOINT4Y      0x09          //Y(20)           ff

#define STDPOINT5X      0x0A         // X               ff
#define STDPOINT5Y      0x0B          //Y               ff 

#define STDPOINT6X      0x0C         // X               ff
#define STDPOINT6Y      0x0D          //Y               ff 

#define STDPOINT7X      0x0E         // X               ff
#define STDPOINT7Y      0x0F          //Y               ff 

#define STDPOINT8X      0x10         // X               ff
#define STDPOINT8Y      0x11          //Y               ff 

#define STDPOINT9X      0x12          //X               ff
#define STDPOINT9Y      0x13          //Y               ff 

#define STDPOINT10X     0x14          //X               ff
#define STDPOINT10Y     0x15          //Y               ff 

#define STDDSTWIDTH     0x1E          //X(0C)           ff
#define STDDSTHEIGHT    0x1F          //Y(0C)           ff
//==============================================================================

#define STDVIDEO_HEAD    0x20          //X(b0)           ff
#define STDVIDEO_ADDR    0x21          //X(00)           ff
#define STDVIDEO_VGA     0x22          //X(04)           ff   
#define STDVIDEO_STD     0x23          //X(01)           ff 
//#define STDVIDEO_STD     0x24          //X(03)           ff
#define STDVIDEO_NONE    0x25          //X(02)           ff 


#define STDVIDEO_1       0x26          //X(01)           ff    ��01~04����ʦ���½�̨����
#define STDVIDEO_2       0x27          //X(01��          ff    ��01~04����ʱ������
#define STDVIDEO_END     0x28          //X(ff)           ff  

#define STDVIDEO_SENV    0x29          //X(64)           ff 

#define STDVIDEO_DELAY   0x2f          //X(64)           ff 

#define STDCENTX         0x30          //X(5a)           ff    
#define STDCENTY       	 0x31         // Y(2d)           ff   

#define STDPARITY        0x32          //X(00)           ff     (9600=0x00,1200=0x04,2400=0x06
// 4800=0x08,19200=0x0b,38400=0x0d)

#define STDPROT       	 0x33          //X(00)           ff     (Visca = 00,Pelco_P = 01)      

#define STDSPEEDUD       0x36          //X(14)           ff     (00=Stop,01=Max,30=Min)

#define STDSPEEDLR       0x37          //X(10)           ff     (00=Stop,01=Max,30=Min)


#define STDTURNUD        0x39         // X(00)           ff     (00=Stop,01,02,�ϡ�����)       

#define STDTURNLR        0x3a          //X(01)           ff     (00=Stop,01,02,�󡶡���) 

#define STDRSTLUX        0x3b          //X(46)           ff     (00~ b4)
#define STDRSTLUY        0x3c          //X(10)           ff     (00~ 90)

#define STDRSTRD         0x3d         // X(6e)           ff     (00~ b4)
//#define STDRSTRD         0x3e         // X(1a)           ff     (00~ 90)

#define STDWAITTIME      0x3f         // X(14)           ff     (���źŵ���ȫ���ȴ�ʱ�䣩
#define STDTURNTIME      0x40          //X(0f)           ff     (�������Ԥ��λʱ�䣩


//#define STDCENTY         0x60        //  X(20)           ff
//#define STDCENTY         0x61         // X(03)           ff
typedef struct point {
	unsigned char x;
	unsigned char y;
} point;
typedef struct teachdsp {
	unsigned char usedstat;
	unsigned char pointnum;
	point points[10];
} teachdsp;
