CC		= arm-lwm-linux-gnueabi-gcc
LD		= arm-lwm-linux-gnueabi-ld
AR		= arm-lwm-linux-gnueabi-ar
OBJCOPY	= arm-lwm-linux-gnueabi-objcopy
OBJDUMP	= arm-lwm-linux-gnueabi-objdump

INCLUDEDIR	:= $(shell pwd)/include
CFLAGS		:= -Wall -O2
CPPFLAGS	:= -nostdinc -fno-builtin -I$(INCLUDEDIR)

BUILT_IN_OBJ = built-in.o

export CC LD AR OBJCOPY OBJDUMP INCLUDEDIR CFLAGS CPPFLAGS

export BUILT_IN_OBJ 

SUBDIRS =  start device lib app main
SUBOBJS := $(foreach n, $(SUBDIRS), $(n)/$(BUILT_IN_OBJ))

all: mini-boot.bin mini-boot.dis

mini-boot.bin : mini-boot.elf
	${OBJCOPY} -O binary -S $^ $@
mini-boot.dis : mini-boot.elf
	${OBJDUMP} -D -m arm $^ > $@
mini-boot.elf : $(SUBOBJS)
	${LD} -Tmini-boot.lds -o $@ $^
	
$(SUBOBJS) : $(SUBDIRS)
	
.PHONY : $(SUBDIRS)
$(SUBDIRS) :
	@make -C $@ all
	
.PHONE : clean
clean:
	rm -f mini-boot.dis mini-boot.bin mini-boot.elf *.o
	@for subdir in $(SUBDIRS); do \
	(make clean -C $$subdir); \
	done
	
