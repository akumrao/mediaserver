################################################################################
### CUSTOMIZE BELOW HERE #######################################################

LIVE555_ROOT = /workspace/live


CC	= gcc
CXX     = g++
TARGET=mediaserver # define the name of the executable
SOURCES    = $(wildcard *.cpp ../src/*.cpp ../ffmpeg/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp ../../base/src/*.cpp ../../json/src/*.cpp ../../crypto/src/*.cpp ../../net/src/*.cpp ../../http/src/*.cpp  ../../http_parser/*.cpp )

CXXFLAGS   := -g -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized -Wno-attributes -Wno-fpermissive -Wno-deprecated-declarations
 

CPPFLAGS   := -I../../rtsp/include/ -I../ffmpeg -I../ffmpeg/compat/atomics/gcc/ -I../../json/include/ -I../../crypto/include/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../net/uv_multiplex/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ -I${LIVE555_ROOT}/liveMedia/include -I${LIVE555_ROOT}/groupsock/include -I${LIVE555_ROOT}/BasicUsageEnvironment/include -I${LIVE555_ROOT}/UsageEnvironment/include \
              -DHTTPSSL
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


clean:
	rm -f  $(OBJS)

