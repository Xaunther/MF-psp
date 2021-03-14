#Makefile for gpsp

#Add global definitions
include ../config.mak

#PSP toolchain config
PSPSDK          = ${shell psp-config --pspsdk-path}
PREFIX          = ${shell psp-config --psp-prefix}

#Logos
#PSP_EBOOT_ICON = ICON0.PNG
#PSP_EBOOT_PIC1  = ICON1.PNG

#Target name
TARGET = MF-psp

#.o Objects to create
OBJS = ../src/cheats.o ../src/cpu.o ../src/dvemgr.o ../src/gu.o ../src/homehook.o ../src/main.o ../src/message.o \
../src/SystemButtons.o ../src/zip.o ../src/common.o ../src/draw.o ../src/fbm_print.o ../src/gui.o  ../src/input.o \
../src/memory.o ../src/sound.o ../src/video.o

#Add adhoc source if requested
ifeq ($(USE_ADHOC), 1)
OBJS        += adhoc.o
endif

#PSP version
PSP_FW_VERSION = 660

#If a test build, add the "test" tag to eboot title
ifeq ($(TEST_MODE), 1)
VERSION_RELEASE = test
endif

#Examples: test 1, test 2, 1, 2, ...
VERSION_OPT = ${VERSION_RELEASE} ${VERSION_BUILD}

#Eboot title definition, Example: MF-psp-gpSP Xaunther 0.1 test 1
PSP_EBOOT_TITLE = MF-psp-gpSP Xaunther ${VERSION_MAJOR}.${VERSION_MINOR} ${VERSION_OPT}

#Add main path
VPATH   += ..
#C Flags
CFLAGS  += -O3 -DPSP_BUILD -G0 -funsigned-char
CFLAGS  += -Wall
CFLAGS  += -ffast-math -fforce-addr -fmerge-all-constants -funsafe-loop-optimizations -ftree-loop-linear
CFLAGS  += -DVERSION_MAJOR=${VERSION_MAJOR}
CFLAGS  += -DVERSION_MINOR=${VERSION_MINOR}
CFLAGS  += -DVERSION_BUILD=${VERSION_BUILD}
#C flags only added if requested
ifeq ($(TEST_MODE), 1)
CFLAGS  += -DTEST_MODE
endif
ifeq ($(USE_DEBUG), 1)
CFLAGS  += -DUSE_DEBUG
endif
ifeq ($(USE_ADHOC), 1)
CFLAGS  += -DUSE_ADHOC
endif
ifeq ($(USE_C_CORE), 1)
CFLAGS  += -DUSE_C_CORE
endif
ifeq ($(PSP_LARGE_MEMORY), 0)
CFLAGS  += -DUSE_EXT_MEM
endif

#Define c++ flags and assembly flags
CXXFLAGS = $(CFLAGS) -std=c++2a -fno-rtti
ASFLAGS = $(CFLAGS)
#Add EBOOT.PBP and the corresponding PRX module
EXTRA_TARGETS = EBOOT.PBP
BUILD_PRX = 1

#Libraries to use
#LIBS    += -lpspgu -lpsprtc -lpspaudio -lz -lpsppower -lpspkubridge -lpspgum -lpspsystemctrl_user -lm
#ifdef USE_ADHOC
#LIBS    += -lpspnet -lpspwlan -lpspnet_adhoc -lpspnet_adhocctl -lpspnet_adhocmatching
#endif

#Build using the toolchain
include $(PSPSDK)/lib/build.mak



