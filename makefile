#------------------------------------------------------------------------------#
# This makefile was generated by 'cbp2make' tool rev.147                       #
#------------------------------------------------------------------------------#


WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC_G = -Iinclude
CFLAGS_G = -std=c++11 -Wall -fexceptions -DBOOST_LOG_DYN_LINK
RESINC_G =
DEP_G =
LIBDIR_G =
LIB_G = /usr/local/lib/libavcodec.so /usr/local/lib/libavdevice.so /usr/local/lib/libavfilter.so /usr/local/lib/libavformat.so /usr/local/lib/libavutil.so /usr/local/lib/libswresample.so /usr/local/lib/libswscale.so /usr/local/lib/libboost_system.so /usr/local/lib/libboost_thread.so /usr/local/lib/libboost_filesystem.so /usr/local/lib/libboost_date_time.so /usr/local/lib/libboost_timer.so /usr/local/lib/libboost_log.so /usr/local/lib/mir/libSkyLibs.so /usr/local/lib/mir/libSkyLibsDB.so
LDFLAGS_G = -lz -lm -lpthread -lmp3lame -lx264 -lvorbis -lfdk-aac -lfaac

OUTNAME = captura

debug: INC = $(INC_G)
debug: CFLAGS = $(CFLAGS_G)
debug: RESINC = $(RESINC_G)
debug: DEP = $(DEP_G)
debug: LIBDIR = $(LIBDIR_G)
debug: LIB = $(LIB_G)
debug: LDFLAGS = $(LDFLAGS_G) -s
debug: DIR = Debug
debug: before out after

release: INC = $(INC_G)
release: CFLAGS = $(CFLAGS_G) -O2
release: RESINC = $(RESINC_G)
release: DEP = $(DEP_G)
release: LIBDIR = $(LIBDIR_G)
release: LIB = $(LIB_G)
release: LDFLAGS = $(LDFLAGS_G) -s
release: DIR = Release
release: before out after

OBJDIR = obj/$(DIR)
BINDIR = bin/$(DIR)
OUT = $(BINDIR)/$(OUTNAME)

OBJ = $(OBJDIR)/src/main.o $(OBJDIR)/src/captura.o $(OBJDIR)/src/configuration.o $(OBJDIR)/src/logger.o $(OBJDIR)/src/LogClass.o $(OBJDIR)/src/threadpool.o $(OBJDIR)/src/threadcapture.o $(OBJDIR)/src/queue.o $(OBJDIR)/src/streamradio.o $(OBJDIR)/src/sliceprocess.o $(OBJDIR)/src/parser.o $(OBJDIR)/src/rawdata.o $(OBJDIR)/src/filedata.o

all: debug release

before:
	test -d obj || mkdir -p obj
	test -d $(OBJDIR) || mkdir -p $(OBJDIR)
	test -d $(OBJDIR)/src || mkdir -p $(OBJDIR)/src
	test -d bin || mkdir -p bin
	test -d $(BINDIR) || mkdir -p $(BINDIR)

after:

out: before $(OBJ) $(DEP)
	$(LD) $(LIBDIR) -o $(OUT) $(OBJ)  $(LDFLAGS) $(LIB)

$(OBJDIR)/src/main.o: src/main.cpp
	$(CXX) $(CFLAGS) $(INC) -c src/main.cpp -o $(OBJDIR)/src/main.o

$(OBJDIR)/src/captura.o: src/captura.cpp
	$(CXX) $(CFLAGS) $(INC) -c src/captura.cpp -o $(OBJDIR)/src/captura.o

$(OBJDIR)/src/configuration.o: src/configuration.cpp
	$(CXX) $(CFLAGS) $(INC) -c src/configuration.cpp -o $(OBJDIR)/src/configuration.o

$(OBJDIR)/src/logger.o: src/logger.cpp
	$(CXX) $(CFLAGS) $(INC) -c src/logger.cpp -o $(OBJDIR)/src/logger.o

$(OBJDIR)/src/LogClass.o: src/LogClass.cpp
	$(CXX) $(CFLAGS) $(INC) -c src/LogClass.cpp -o $(OBJDIR)/src/LogClass.o

$(OBJDIR)/src/threadpool.o: src/threadpool.cpp
	$(CXX) $(CFLAGS) $(INC) -c src/threadpool.cpp -o $(OBJDIR)/src/threadpool.o

$(OBJDIR)/src/threadcapture.o: src/threadcapture.cpp
	$(CXX) $(CFLAGS) $(INC) -c src/threadcapture.cpp -o $(OBJDIR)/src/threadcapture.o

$(OBJDIR)/src/queue.o: src/queue.cpp
	$(CXX) $(CFLAGS) $(INC) -c src/queue.cpp -o $(OBJDIR)/src/queue.o

$(OBJDIR)/src/streamradio.o: src/streamradio.cpp
	$(CXX) $(CFLAGS) $(INC) -c src/streamradio.cpp -o $(OBJDIR)/src/streamradio.o

$(OBJDIR)/src/sliceprocess.o: src/sliceprocess.cpp
	$(CXX) $(CFLAGS) $(INC) -c src/sliceprocess.cpp -o $(OBJDIR)/src/sliceprocess.o

$(OBJDIR)/src/parser.o: src/parser.cpp
	$(CXX) $(CFLAGS) $(INC) -c src/parser.cpp -o $(OBJDIR)/src/parser.o

$(OBJDIR)/src/rawdata.o: src/rawdata.cpp
	$(CXX) $(CFLAGS) $(INC) -c src/rawdata.cpp -o $(OBJDIR)/src/rawdata.o

$(OBJDIR)/src/filedata.o: src/filedata.cpp
	$(CXX) $(CFLAGS) $(INC) -c src/filedata.cpp -o $(OBJDIR)/src/filedata.o

clean:
	rm -rf bin
	rm -rf obj

.PHONY: before after clean

