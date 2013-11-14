/***************************************************************************
*  描述：主要用于保存 kernel的一些函数或者结构体，定义等等
*
*
*
****************************************************************************/
#ifndef _KERNEL_COMMON_H__
#define _KERNEL_COMMON_H__

#include <sys/ioctl.h>
//严重注意，该表项一定要和kernel匹配，在加入分辨率之后，该表项也要改变
//原始结构体在fpga.h
enum VIDEO_SUPPORT_FORMAT {
    APP_VGA_640X480X60, //----------0
    APP_VGA_640X480X72, //----------1
    APP_VGA_640X480X75,//----------2
    APP_VGA_640X480X85,//----------3
    APP_VGA_800X600X60,//----------4
    APP_VGA_800X600X72,//----------5
    APP_VGA_800X600X75,//----------6
    APP_VGA_800X600X85,//----------7
    APP_VGA_1024X768X60,//----------8
    APP_VGA_1024X768X72,//----------9
    APP_VGA_1024X768X75,//----------10
    APP_VGA_1024X768X85,//----------11
    APP_VGA_1280X768X60,//----------12
    APP_VGA_1280X800X60,//----------13
    APP_VGA_1280X960X60,//----------14
    APP_VGA_1440X900X60,//----------15
    APP_VGA_1400X1050X60,//----------16
    APP_VGA_1280X1024X60,//----------17
    APP_VGA_1280X1024X75,//----------18
    APP_VGA_1600X1200X60,//----------19
    APP_VGA_1280X720VX60,//----------20

    APP_VIDEO_1280X720X50P,//----------21
    APP_VIDEO_1280X720X60P,//----------22
    APP_VIDEO_1920X1080X50I,//----------23
    APP_VIDEO_1920X1080X60I,//----------24
    APP_VIDEO_1920X1080X25P,//----------25
    APP_VIDEO_1920X1080X30P,//----------26
    APP_VIDEO_1920X1080X50P,//----------27
    APP_VIDEO_1920X1080X60P,//----------28
    APP_VIDEO_720X480X60I,//----------29
    APP_VIDEO_720X576X50I,//----------30
    APP_VIDEO_720X480X60P,//----------31
    APP_VIDEO_720X576X50P,//----------32

    APP_VGA_1366X768X60,//----------33
    APP_VIDEO_MAX_COUNT
};
#if 0
////////gpio/////////
//原始结构体在videodev2.h
typedef struct app_gpio_data {
	unsigned int data;
	unsigned int nums;
} app_gpio_op;

#define SET_PHY					_IOWR('g', 11, app_gpio_op)
#endif
#endif

