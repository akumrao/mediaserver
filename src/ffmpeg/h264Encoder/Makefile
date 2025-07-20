CC	= gcc
CXX     = g++

FFMPEG_ROOT = /workspace/ffmpeg

FFMPEG_ROO_W = /workspace/webrtc_android/src/third_party/ffmpeg

FFMPEG_ROOT_WH = /workspace/webrtc_android/src/third_party/ffmpeg/chromium/config/Chrome/linux/x64


TARGET=runH264Encoder # define the name of the executable
#SOURCES=$(wildcard *.cpp  ../h24Decoder/H*.cpp  )
SOURCES=$(wildcard *.cpp )

CXXFLAGS   := -g -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized


#CPPFLAGS   := -I../h24Decoder/ -I../../json/include/ -I${FFMPEG_ROOT} -I../../signal/include -I../../base/include/ -I../../net/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ 

CPPFLAGS   := -I../h24Decoder/ -I../../json/include/ -I${FFMPEG_ROO_W} -I${FFMPEG_ROOT_WH} -I../../signal/include -I../../base/include/ -I../../net/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ 
              
LDFLAGS   := -L/usr/lib/x86_64-linux-gnu

LDLIBS    := -lpthread -lm -ldl -lz -lssl -lcrypto -lfdk-aac -lX11

#SLIBS     := ${FFMPEG_ROOT}/libavformat/libavformat.a ${FFMPEG_ROOT}/libavcodec/libavcodec.a ${FFMPEG_ROOT}/libavutil/libavutil.a ${FFMPEG_ROOT}/libswresample/libswresample.a  /usr/local/lib/libx264.a


SLIBS     := /workspace/webrtc_android/src/out/m85/obj/libwebrtc.a  /usr/local/lib/libx264.a





################################################################################
### DO NOT EDIT THE FOLLOWING LINES ############################################


# define list of objects
OBJSC=$(SOURCES:.cpp=.o)
OBJS=$(OBJSC:.cpp=.o)

# the target is obtained linking all .o files
all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJS) $(SLIBS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

purge: clean
	rm -f $(TARGET)

clean:
	rm -f  $(OBJS)

################################################################################
################################################################################
