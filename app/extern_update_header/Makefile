include ../modules.mak
DEFINES += -DCL4000_DVI
REACHDLL = add_header.elf

$(REACHDLL):app_update_header.c  
	rm -rf *.elf
	gcc -o $(REACHDLL) $(DEFINES) -DPC_APP -I./ app_update_header.c
	rm -rf *.o

clean:
	@echo Removing generated files..
	rm -rf *.elf
	rm -rf *.o
