ROOTPATH = $(shell pwd)/../
include $(ROOTPATH)/config.mak
########################################################
####################设置库名称/外部使用头文件#######################
LIBMODULE:=lib$(notdir $(shell pwd)).a

OUTPUT_INCLUDE = $(notdir $(shell pwd)).h
#OUTPUT_INCLUDE +=
########################################################

#MVTOOL_DIR=/opt/mv_pro_5.0/montavista/pro/devkit/arm/v5t_le
#MVTOOL_PREFIX=$(MVTOOL_DIR)/bin/arm_v5t_le-
#CC:=arm_v5t_le-gcc
CC:=$(MVTOOL_PREFIX)gcc
AR:=$(MVTOOL_PREFIX)ar
RANLIB:=ranlib

ARFLAGS = rcs



SRCS = 	$(wildcard *.c)	
OBJS = 	$(patsubst %.c, %.o, $(SRCS))						

CPPFLAGS:=
CFLAGS:=-g -Wall -I./
LDFLAGS:=-g -L.



FMTEXE := /bin/astyle
FMT_ARGS := --style=linux --indent=tab --brackets=break --indent-classes --indent-switches \
	--indent-namespaces --indent-labels --indent-preprocessor --indent-col1-comments \
	--min-conditional-indent=0 --break-blocks --pad-oper --unpad-paren --add-brackets \
	--align-pointer=name --align-reference=name --mode=c --lineend=linux --quiet 
	
#all: format  ${LIBMODULE} 
all:  ${LIBMODULE} 

format:
	$(FMTEXE) $(FMT_ARGS) ./*.c
	rm -f ./*.orig
	chmod -R 777 ../$(notdir $(CURDIR))
	
	
install:
	cp -frv *.so /opt/mv_pro_5.0/montavista/pro/devkit/arm/v5t_le/target/usr/lib/
	cp -frv *.so /home/workdir/filesys/usr/lib/
	cp -frv ts.h /home/dvsdk_2_00_00_18/dvsdk_demos_2_00_00_04/dm6467/HDencode
	cp -frv ts.h /home/dvsdk_2_00_00_18/dvsdk_demos_2_00_00_04/dm6467/HDencode_test
clean:
	rm -f ${OBJS} $(LIBMODULE)

${LIBMODULE}: ${OBJS}
	@echo "Creating library ${LIBMODULE} ..."
	${AR} $(ARFLAGS)  ${LIBMODULE} $(OBJS)
	cp ${LIBMODULE} $(LIBS)
	cp $(OUTPUT_INCLUDE) $(INCLUDE)

.c.o:
	${CC} -c ${CFLAGS} ${CPPFLAGS} $< -o $@	
	
