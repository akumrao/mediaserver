### CUSTOMIZE BELOW HERE #######################################################
FFMPEG_ROOT = /workspace/ffmpeg

CC	= gcc
CXX     = g++
TARGET=demuxer # define the name of the executable
SOURCES=$(wildcard demuxer_buffer.cpp    ../../ffmpeg/ffmpeg_cpp/*.cpp ../../ffmpeg/ffmpeg_cpp/codec/*.cpp  ../../ffmpeg/ffmpeg_cpp/codec/x86/*.cpp ../../ffmpeg/ffmpeg_cpp/mp4Probe/*.cpp)

CXXFLAGS   := -g -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized -Wno-deprecated-declarations -fPIC 

CPPFLAGS   := -I../../ffmpeg/ffmpeg_cpp/ -I../../ffmpeg/ffmpeg_cpp/compat/atomics/gcc/ -I../../ffmpeg/ffmpeg_cpp/codec -I../../ffmpeg/ffmpeg_cpp/codec/x86/ -fpermissive -Wno-inconsistent-missing-override -Wno-uninitialized -Wno-deprecated-declarations
              
LDFLAGS   := -L/usr/lib/x86_64-linux-gnu


LDLIBS    := -lpthread  -lz -lm -ldl -lfdk-aac  -no-pie

#SLIBS     :=  ${FFMPEG_ROOT}/libavformat/libavformat.a  ${FFMPEG_ROOT}/libavfilter/libavfilter.a  ${FFMPEG_ROOT}/libavcodec/libavcodec.a ${FFMPEG_ROOT}/libavutil/libavutil.a  ${FFMPEG_ROOT}/libswscale/libswscale.a ${FFMPEG_ROOT}/libswresample/libswresample.a /usr/local/lib/libx264.a 
SLIBS     :=  /usr/local/lib/libx264.a 



################################################################################
### DO NOT EDIT THE FOLLOWING LINES ############################################


# define list of objects
OBJSC=$(SOURCES:.cpp=.o)
OBJS=$(OBJSC:.cpp=.o)

# the target is obtained linking all .o files
all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJS) $(SLIBS) $(LDLIBS) -o $(TARGET)

purge: clean
	rm -f $(TARGET)

clean:
	rm -f  $(OBJS)

################################################################################
################################################################################
