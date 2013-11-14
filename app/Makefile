# Makefile
#
# ============================================================================
# Copyright (c) Texas Instruments Inc 2009
#
# Use of this software is controlled by the terms and conditions found in the
# license agreement under which this software has been supplied or provided.
# ============================================================================

ROOTDIR = ../../..
FILESYS_PATH = /home/workdir/filesys
TARGET = $(notdir $(CURDIR))

include ./modules.mak

ifeq "$(findstring HP,$(ENC))" "HP"
include $(ROOTDIR)/Rules_HP.make
DEFINES += -DNEW_ENCODE_LIB_HP
else
include $(ROOTDIR)/Rules.make
endif
FMTEXE := /bin/astyle
FMT_ARGS := --style=linux --indent=tab --brackets=break --indent-classes --indent-switches \
	--indent-namespaces --indent-labels --indent-preprocessor --indent-col1-comments \
	--min-conditional-indent=0 --break-blocks --pad-oper --unpad-paren --add-brackets \
	--align-pointer=name --align-reference=name --mode=c --lineend=linux --quiet 

#FORMAT_FILE := $(wildcard *.c) $(wildcard *.h)

CURRENTDIR = $(notdir $(shell pwd))
VERSION= 0
#######################附加模块###宏定义#####开始###############
UPDATE_DIR = ./update

ifeq "$(findstring CL4000-DVI,$(MODULES))" "CL4000-DVI"
DEFINES += -DCL4000_DVI
DEFINES += -DADD_UPDATE_HEADER
REACHDLL = $(TARGET)_DVI.elf
RUN_FILE = ModuleResize
BOX_VER = v$(shell grep "BOX_VER" common.h  |sed 's/"/ /g' |awk '{print $$3}'|sed -e '2!d')
BOARD_TYPE = CL4000_DVI
endif

ifeq "$(findstring CL4000-SDI,$(MODULES))" "CL4000-SDI"
DEFINES += -DCL4000_SDI
DEFINES += -DADD_UPDATE_HEADER
REACHDLL = $(TARGET)_SDI.elf
RUN_FILE = ModuleResize
BOX_VER = v$(shell grep "BOX_VER" common.h  |sed 's/"/ /g' |awk '{print $$3}'|sed -e '1!d')
BOARD_TYPE = CL4000_SDI
endif

ifeq "$(findstring DSS-ENC-1200,$(MODULES))" "DSS-ENC-1200"
DEFINES += -DDSS_ENC_1200
DEFINES += -DADD_UPDATE_HEADER
REACHDLL = $(TARGET)_1200.elf
RUN_FILE = HDencode
BOX_VER =v$(shell grep "BOX_VER" common.h|sed 's/\"/ /g' |awk '{print $$3}'|sed -e '3!d')
BOARD_TYPE = ENC1200
endif

ifeq "$(findstring DSS-ENC-1100,$(MODULES))" "DSS-ENC-1100"
DEFINES += -DDSS_ENC_1100
REACHDLL = $(TARGET)_1100.elf
RUN_FILE = HDencode
BOX_VER =v$(shell grep "BOX_VER" common.h|sed 's/\"/ /g' |awk '{print $$3}'|sed -e '4!d')
BOARD_TYPE = ENC1100
endif

ifeq "$(findstring DSS-ENC-1260,$(MODULES))" "DSS-ENC-1260"
DEFINES += -DDSS_ENC_1260
DEFINES += -DADD_UPDATE_HEADER
REACHDLL = $(TARGET)_1260.elf
RUN_FILE = HDencode
BOX_VER =v$(shell grep "BOX_VER" common.h|sed 's/\"/ /g' |awk '{print $$3}'|sed -e '3!d')
BOARD_TYPE = ENC1260
endif

ifeq "$(findstring rtsp_quicktime,$(MODULES))" "rtsp_quicktime"
DEFINES += -DQUICKTIME_RTSP
endif


ifeq "$(findstring TEST-MODE,$(MODULES))" "TEST-MODE"
DEFINES += -DTEST_MODE
endif

###############################log on/off##################

ifeq "$(findstring ENABLE-DEUBG,$(MODULES))" "ENABLE-DEUBG"
DEFINES += -DENABLE_DEUBG
endif

#######################附加模块###########结束##################
DEFINES += -DUSE_LINUX_PLATFORM
ifeq "$(findstring DSS-ENC-1200,$(MODULES))" "DSS-ENC-1200"
DEFINES += -DHAVE_RTSP_MODULE
endif

ifeq "$(findstring DSS-ENC-1100,$(MODULES))" "DSS-ENC-1100"
DEFINES += -DHAVE_RTSP_MODULE
endif

ifeq "$(findstring DSS-ENC-1260,$(MODULES))" "DSS-ENC-1260"
DEFINES += -DHAVE_RTSP_MODULE
endif


###默认开启的宏定义
####质量调整
DEFINES +=-DHAVE_QUALITY_MODULE 

# Comment this out if you want to see full compiler and linker output.
VERBOSE = @

# Package path for the XDC tools
XDC_PATH = $(USER_XDC_PATH);../../packages;$(DMAI_INSTALL_DIR)/packages;$(CE_INSTALL_DIR)/packages;$(FC_INSTALL_DIR)/packages;$(LINK_INSTALL_DIR)/packages;$(XDAIS_INSTALL_DIR)/packages;$(CMEM_INSTALL_DIR)/packages;$(CODEC_INSTALL_DIR)/packages

# Where to output configuration files
XDC_CFG		= $(TARGET)_config

# Output compiler options
XDC_CFLAGS	= $(XDC_CFG)/compiler.opt

# Output linker file
XDC_LFILE	= $(XDC_CFG)/linker.cmd

# Input configuration file
XDC_CFGFILE	= $(TARGET).cfg

# Platform (board) to build for
XDC_PLATFORM = ti.platforms.evmDM6467

# Target tools
XDC_TARGET = gnu.targets.MVArm9

# The XDC configuration tool command line
CONFIGURO = $(XDC_INSTALL_DIR)/xs xdc.tools.configuro

C_FLAGS += -Wall -I./ 
RUN_PATH = $(FILESYS_PATH)/opt/reach
LD_FLAGS += -L$(RUN_PATH)
LD_FLAGS += -lpthread -lpng -ljpeg -lfreetype -lasound -lssl -lcrypto -lrt

COMPILE.c = $(VERBOSE) $(MVTOOL_PREFIX)gcc $(C_FLAGS) $(CPP_FLAGS) -c $(DEFINES)
LINK.c = $(VERBOSE) $(MVTOOL_PREFIX)gcc $(LD_FLAGS)

SOURCES = $(wildcard *.c) $(wildcard middle/*.c) $(wildcard modules/porting/*.c)  $(wildcard extern_update_header/*.c)  $(wildcard stream_output/*.c) 
HEADERS = $(wildcard *.h) $(wildcard middle/*.h) $(wildcard modules/porting/*.h)  $(wildcard extern_update_header/*.h)  $(wildcard stream_output/*.h) 

FORMAT_FILE := $(SOURCES) $(HEADERS)
C_FLAGS += -I./modules/include -I./middle  -I./modules/porting -I./stream_output

ifeq "$(findstring DSS-ENC-1100,$(MODULES))" "DSS-ENC-1100"
	LOCAL_LIBS += ./modules/libs/librtmp.a
	LOCAL_LIBS += ./modules/libs/librtp.a
	LOCAL_LIBS += ./modules/libs/librtsp.a
	LOCAL_LIBS += ./modules/libs/libts.a
endif

ifeq "$(findstring DSS-ENC-1200,$(MODULES))" "DSS-ENC-1200"
	LOCAL_LIBS += ./modules/libs/librtmp.a
	LOCAL_LIBS += ./modules/libs/librtp_build.a
	LOCAL_LIBS += ./modules/libs/librtsp.a
	LOCAL_LIBS += ./modules/libs/libts_build.a
endif

ifeq "$(findstring DSS-ENC-1260,$(MODULES))" "DSS-ENC-1260"
	LOCAL_LIBS += ./modules/libs/librtmp.a
	LOCAL_LIBS += ./modules/libs/librtp_build.a
	LOCAL_LIBS += ./modules/libs/librtsp.a
	LOCAL_LIBS += ./modules/libs/libts_build.a
endif


LOCAL_LIBS += ./modules/libs/liblog.a

OBJFILES = $(SOURCES:%.c=%.o)



default:svn_revision begin all

svn_revision:
ifneq ($(wildcard .svn/*), )
SVN_REV = $(strip $(VERSION)).$(word 1, $(shell svn st -vN $(shell pwd) | grep -v "$(shell pwd)/"))
else
SVN_REV = $(VERSION).0
endif
	@echo SVN = $(SVN_REV)
	
begin:
	@echo "const static char *g_make_build_date = \"`date`\";" > build_info.h
	@echo "const static char *g_make_VSS_version = \"$(SVN_REV)\";" >> build_info.h
ifeq "$(findstring CL4000-DVI,$(MODULES))" "CL4000-DVI"
	@echo "now i will intall the CL4000-DVI !"
	rm -rf $(CURRENTDIR).cfg
	ln -s HDencode_1100.cfg $(CURRENTDIR).cfg
endif
ifeq "$(findstring CL4000-SDI,$(MODULES))" "CL4000-SDI"
	@echo "now i will intall the CL4000-SDI !"
	rm -rf $(CURRENTDIR).cfg
	ln -s HDencode_1100.cfg $(CURRENTDIR).cfg
endif

ifeq "$(findstring DSS-ENC-1100,$(MODULES))" "DSS-ENC-1100"

	@echo "now i will intall the DSS-ENC-1100 !"
	rm -rf $(CURRENTDIR).cfg
	ln -s HDencode_1100.cfg $(CURRENTDIR).cfg
endif
ifeq "$(findstring DSS-ENC-1200,$(MODULES))" "DSS-ENC-1200"
	@echo "now i will intall the DSS-ENC-1200 !"
	rm -rf $(CURRENTDIR).cfg
	ln -s HDencode_1100.cfg $(CURRENTDIR).cfg
endif

ifeq "$(findstring DSS-ENC-1260,$(MODULES))" "DSS-ENC-1260"
	@echo "now i will intall the DSS-ENC-1260 !"
	rm -rf $(CURRENTDIR).cfg
	ln -s HDencode_1100.cfg $(CURRENTDIR).cfg
endif


#.PHONY: clean install 

all:  new_header  dm6467
dm6467:	dm6467_al
format: format
new_header:
	@sed -e "s#<version>#$$(git describe --dirty --always)#g" <version.h.in> version.h.tmp
	@if diff -q version.h.tmp version.h >/dev/null 2>&1; then \
		rm version.h.tmp; \
	else \
		echo "version.h.in => version.h" ; \
	mv version.h.tmp version.h; \
	fi

format:
	$(FMTEXE) $(FMT_ARGS) $(FORMAT_FILE)
	find . -name *.orig | xargs rm -f
	chmod -R 777 ../$(notdir $(CURDIR))

dm6467_al:	$(TARGET)

install:	$(if $(wildcard $(TARGET)), install_$(TARGET))

install_$(TARGET):
	@install -d $(EXEC_DIR)
	@install $(TARGET) $(EXEC_DIR)
	@install $(TARGET).txt $(EXEC_DIR)
	@echo
	@echo Installed $(TARGET) binaries to $(EXEC_DIR)..

$(TARGET):	$(OBJFILES) $(XDC_LFILE)
	@echo
	#make -C ../new_protocol/
	@echo Linking $(LINK.c) -o $@ '-Wl,--start-group' $(LOCAL_LIBS) '-Wl,--end-group' $^
	$(LINK.c) -o $@  $^ '-Wl,--start-group' $(LOCAL_LIBS) '-Wl,--end-group'
	mv $(TARGET) $(REACHDLL)
#	cp -rf  $(REACHDLL) $(RUN_PATH)/$(RUN_FILE)
	cp -rf  $(REACHDLL) ../../../../rootfs_compose/opt/reach
#	rm -rf build_info.h $(CURRENTDIR).cfg

	
$(OBJFILES):	%.o: %.c $(HEADERS) $(XDC_CFLAGS)
	@echo Compiling $@ from $<..
	@echo $(COMPILE.c) $(shell cat $(XDC_CFLAGS)) -o $@ $<
	$(COMPILE.c) $(shell cat $(XDC_CFLAGS)) -o $@ $<

$(XDC_LFILE) $(XDC_CFLAGS):	$(XDC_CFGFILE)
	@echo
	@echo ======== Building $(TARGET) ========
	@echo Configuring application using $<
	@echo
	$(VERBOSE) XDCPATH="$(XDC_PATH)" $(CONFIGURO) -c $(MVTOOL_DIR) -o $(XDC_CFG) -t $(XDC_TARGET) -p $(XDC_PLATFORM) $(XDC_CFGFILE)

pkg:
	@echo
	@echo ===============$(BOARD_TYPE) $(BOX_VER)$(ENC)=================
	@echo ...
	cd $(UPDATE_DIR);./mkuppkg $(BOARD_TYPE) $(BOX_VER)$(ENC);cd -
pkg_clean:
	rm -rf $(UPDATE_DIR)/*.v*
clean:
	@echo Removing generated files..
	@echo "$(VERBOSE) -$(RM) -rf $(XDC_CFG) $(OBJFILES) $(TARGET)_*.elf *~ *.d .dep"
	$(VERBOSE) -$(RM) -rf $(XDC_CFG) $(OBJFILES) $(TARGET)_*.elf  *.d .dep
	#-$(RM) -rf ./update/*.bin ./update/*.tgz ./update/ENC* ./update/CL4000*
