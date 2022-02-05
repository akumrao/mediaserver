################################################################################
### CUSTOMIZE BELOW HERE #######################################################

CC	= gcc
CXX     = g++
FFMPEG_ROOT = /workspace/ffmpeg


TARGET=runH264Encoder # define the name of the executable
SOURCES=$(wildcard *.cpp  ../h24Decoder/H*.cpp  )

CXXFLAGS   := -g -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized

CPPFLAGS   := -I../h24Decoder/ -I../../json/include/ -I../../ffmpeg/include -I../../signal/include -I../../base/include/ -I../../net/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ 
              
LDFLAGS   := -L/usr/lib/x86_64-linux-gnu

LDLIBS    := -lpthread -lm -ldl -lz -lssl -lcrypto -lfdk-aac

SLIBS     := ${FFMPEG_ROOT}/libavformat/libavformat.a ${FFMPEG_ROOT}/libavcodec/libavcodec.a ${FFMPEG_ROOT}/libavutil/libavutil.a ${FFMPEG_ROOT}/libswresample/libswresample.a  /usr/local/lib/libx264.a




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
