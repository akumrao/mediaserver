################################################################################
### CUSTOMIZE BELOW HERE #######################################################


FFMPEG_ROOT = /workspace/ffmpeg
#WEBRTC_PATH = /data/delete/mediaserverdocker/webrtc_android/src/
WEBRTC_PATH = /workspace/webrtc_android/src/

CC	= gcc
CXX     = g++
TARGET=runWebRtcClient # define the name of the executable
SOURCES=$(wildcard *.cpp ../src/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp ../../base/src/*.cpp ../../crypto1/src/*.cpp ../../net/src/*.cpp ../../http/src/*.cpp ../../http_parser/*.cpp ../../signal/src/*.cpp )

CXXFLAGS   := -g -fno-rtti -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized

CPPFLAGS   := -I../../json/include/ -I/workspace/ffmpeg/  -I../include -I../../signal/include -I../../base/token/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ -I../webrtc/include  -DOPENSSL_IS_BORINGSSL -DWEBRTC_POSIX  -DWEBRTC_INCLUDE_DIRS=/workspace/webrtc_android/src  \
               -I${WEBRTC_PATH} -I${WEBRTC_PATH}/third_party/abseil-cpp -DHTTPSSL
#LDFLAGS   := -L/export/views/video/ffmpeg/Obj/

LDLIBS    := -lpthread -lm -ldl -lz -lfdk-aac 

SLIBS     := ${WEBRTC_PATH}/out/m85/obj/libwebrtc.a  ${FFMPEG_ROOT}/libavformat/libavformat.a ${FFMPEG_ROOT}/libavcodec/libavcodec.a ${FFMPEG_ROOT}/libavutil/libavutil.a ${FFMPEG_ROOT}/libswresample/libswresample.a /usr/local/lib/libx264.a



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
