BASE=~/source/freeswitch/
#BASE=/home/ctrip/software/csta/freeswitch-1.4.0/
#SNDFILE_DIR=$(switch_srcdir)/libs/libsndfile
#SNDFILE_BUILDDIR=$(switch_builddir)/libs/libsndfile

LOCAL_OBJS=ThreadUtil.o  Config.o Handler.o  Notification.o CommandExecutor.o Controler.o 
LOCAL_SOURCES=ThreadUtil.cpp Config.cpp Handler.cpp  Notification.cpp CommandExecutor.cpp Controler.cpp
#LOCAL_SOURCES=$(wildcard *.cpp)
#LOCAL_OBJS=$(LOCAL_SOURCES:.cpp=.o)

#LOCAL_CFLAGS=-I$(SNDFILE_DIR)/src -I$(SNDFILE_BUILDDIR)/src -I./include/

#LOCAL_LDFLAGS=-lpthread

include $(BASE)/build/modmake.rules
