ROOTPATH = $(shell pwd)/
include $(ROOTPATH)/config.mak

#dirs += ./ts 
dirs += ./rtmp
#dirs += ./rtp
dirs += ./rtsp
dirs += ./log
dirs += ./ts_build
dirs += ./rtp_build

SVN_REV = 0


default:build_info  new_header .PHONY 

build_info:
	@echo "const static char *g_make_build_date = \"`date`\";" > $(notdir $(shell pwd))_build_info.h
	@echo "const static char *g_make_VSS_version = \"$(SVN_REV)\";" >> $(notdir $(shell pwd))_build_info.h
	mkdir -p output
	mkdir -p  output/include
	mkdir -p  output/libs

new_header:
	@sed -e "s#<version>#$$(git describe --dirty --always)#g" <version.h.in> version.h.tmp
	@if diff -q version.h.tmp version.h >/dev/null 2>&1; then \
		rm version.h.tmp; \
	else \
		echo "version.h.in => version.h" ; \
	mv version.h.tmp version.h; \
	fi


# Sub directories target
.PHONY: subdirs $(dirs:%=subdir-%)
	@echo "$(DATA1)"
	tar zcvf $(notdir $(shell pwd))_$(SVN_REV).tar.gz output  $(notdir $(shell pwd))_build_info.h
	rm -rf $(notdir $(shell pwd))_build_info.h
	cp -rf output/libs/*.a ../modules/libs
	cp -rf output/include/*.h ../modules/include
	
subdirs: $(dirs:%=subdir-%)
	@#

$(dirs:%=subdir-%): makedestdirs
	@echo "$(MAKE) -C $(subst subdir-,,$@)"
	$(MAKE) -C $(subst subdir-,,$@)

makedestdirs:
	@#


clean: cleandepend-notsubdir
	@echo Removing generated files..
	@echo "Cleaning directory       $(CURDIR) ..."
	$(RM) -rf $(LIBS)/*.a $(INCLUDE)/*.h build_info.h $(notdir $(shell pwd))_*.tar.gz output
	$(RM) $(OBJS:.o=.d) $(OBJS) *~ *.bak *.o *.d build_info.h *.elf TAGS
	$(foreach dir, $(dirs), $(MAKE) -C $(dir) clean;)

cleandepend-notsubdir:
	@echo "Cleaning dependencies in $(CURDIR) ..."
	$(RM) .depend *.d

cleandepend: cleandepend-notsubdir
	$(foreach dir, $(dirs), $(MAKE) -C $(dir) cleandepend;)	
