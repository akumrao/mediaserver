
CC	= gcc
CXX     = g++
TARGET=mp4 # define the name of the executable
SOURCES=$(wildcard *.cpp   ../../ffmpeg/ffmpeg_cpp/*.cpp ../../ffmpeg/ffmpeg_cpp/codec/*.cpp  ../../ffmpeg/ffmpeg_cpp/codec/x86/*.cpp ../../ffmpeg/ffmpeg_cpp/audio/*.cpp ../../ffmpeg/ffmpeg_cpp/mp4Probe/*.cpp  )

CXXFLAGS   := -g -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized -Wno-attributes -fPIC -fno-permissive -Wno-literal-suffix

CPPFLAGS   := -I../../ffmpeg/include -I../../ffmpeg/ffmpeg_cpp/ -I../../ffmpeg/ffmpeg_cpp/compat/atomics/gcc/ -I../../ffmpeg/ffmpeg_cpp/codec -I../../ffmpeg/ffmpeg_cpp/codec/x86/ -I../../ffmpeg/ffmpeg_cpp/audio/  -I../../ffmpeg/ffmpeg_cpp/mp4Probe/ 
  
#/export/views/video/ffmpeg/libavutil/libavutil.a /export/views/video/ffmpeg/libavcodec/libavcodec.a
#LDLIBS    := -lpthread -lm -ldl -lavformat -lavfilter -lavcodec -lavutil -lswscale -lavdevice -lswresample

LDLIBS    := -lpthread -lm -ldl -lz -lfdk-aac  -no-pie

SLIBS     :=   /usr/local/lib/libx264.a



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
