
CC = gcc
OUT_FILE_NAME = ffmpeg.a

CFLAGS=  -g -Wall -c 

TARGETS=$(OUT_FILE_NAME) $(TARGET)

INC = -I../ffmpeg

OBJ_DIR=../ffmpeg

OUT_DIR=./

# Enumerating of every *.cpp as *.o and using that as dependency.	
# filter list of .c files in a directory.
# FILES =dump_l.c \
#	kter.c \
#
# $(OUT_FILE_NAME): $(patsubst %.c,$(OBJ_DIR)/%.o,$(wildcard $(FILES))) 


.DEFAULT_GOAL=all
.PHONY: all
all: $(TARGETS)



# Enumerating of every *.cpp as *.o and using that as dependency
$(OUT_FILE_NAME): $(patsubst %.c,%.o,$(wildcard ../ffmpeg/*.c))
	ar -r -o $(OUT_DIR)/$@ $^



#Compiling every *.cpp to *.o
%.o: %.c 
	$(CC) -c $(INC) $(CFLAGS) -o $@  $<
	
dirmake:
	@mkdir -p $(OUT_DIR)
	
clean:
	rm -f $(OBJ_DIR)/*.o $(OUT_DIR)/$(OUT_FILE_NAME) Makefile.bak
	rm -f  $(OBJS)

rebuild: clean buil

LIVE555_ROOT = /experiment/mse/valkka-core/ext/live
CXX     = g++
TARGET=mediaserver # define the name of the executable
SOURCES    = $(wildcard *.cpp ../src/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp ../../base/src/*.cpp ../../crypto/src/*.cpp ../../net/src/*.cpp ../../http/src/*.cpp  ../../http_parser/*.cpp )

CXXFLAGS   := -g -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized -Wno-attributes -Wno-fpermissive -Wno-deprecated-declarations
 

CPPFLAGS   := -I../../rtsp/include/ -I../ffmpeg -I../../json/include/ -I../../crypto/include/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../net/uv_multiplex/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ -I${LIVE555_ROOT}/liveMedia/include -I${LIVE555_ROOT}/groupsock/include -I${LIVE555_ROOT}/BasicUsageEnvironment/include -I${LIVE555_ROOT}/UsageEnvironment/include
              
LDFLAGS   := -L/export/views/video/ffmpeg/Obj/

#/export/views/video/ffmpeg/libavutil/libavutil.a /export/views/video/ffmpeg/libavcodec/libavcodec.a
LDLIBS    := -lpthread -lm -ldl -lssl -lcrypto -lfdk-aac 

#SLIBS     := $(OUT_FILE_NAME) 

SLIBS     := $(OUT_FILE_NAME) ${LIVE555_ROOT}/liveMedia/libliveMedia.a  ${LIVE555_ROOT}/groupsock/libgroupsock.a   ${LIVE555_ROOT}/BasicUsageEnvironment/libBasicUsageEnvironment.a  ${LIVE555_ROOT}/UsageEnvironment/libUsageEnvironment.a



# define list of objects
OBJSC=$(SOURCES:.cpp=.o)
OBJS=$(OBJSC:.cpp=.o)

# the target is obtained linking all .o files
all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJS) $(SLIBS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

purge: clean
	rm -f $(TARGET)
	rm -f $(OUT_FILE_NAME)
