CROSS_COMPILE ?=
TARGET ?= nmea_parser

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

INCDIRS := 	. \
			.. \
			unity/src \
			unity/extras/fixture/src \
			unity/extras/memory/src \

INCLUDE := $(patsubst %, -I %, $(INCDIRS))

SRCDIRS :=	. \
			.. \
			unity/src \
			unity/extras/fixture/src \
			unity/extras/memory/src \

CFILES := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))

CFILENDIR := $(notdir $(CFILES))
COBJS := $(patsubst %, obj/%, $(CFILENDIR:.c=.o))
OBJS := $(COBJS)
VPATH := $(SRCDIRS)

CFLAGS += -Wall -Werror -std=c89

release: clean
	$(CC) $(CFLAGS) $(INCLUDE) $(CFILES) -o $(TARGET) $(LDFLAGS) -g

clean :
	rm -rf *.o $(TARGET)
