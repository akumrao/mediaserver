### CUSTOMIZE BELOW HERE #######################################################

FFMPEG_ROOT = /workspace/ffmpeg/

CC	= gcc
CXX  = g++
TARGET=h264Decoder_sample # define the name of the executable
SOURCES    = $(wildcard *.cpp  ../ffmpeg_cpp/*.cpp  ../ffmpeg_cpp/decoder/*.cpp   ../ffmpeg_cpp/decoder/x86/*.cpp  )

CXXFLAGS   := -g -std=c++14  -Wno-reserved-user-defined-literal -Wno-inconsistent-missing-override -Wno-uninitialized -Wno-attributes -Wno-fpermissive -Wno-deprecated-declarations -fPIC 
 

CPPFLAGS   := -I./h264Decoder/ -I../ffmpeg_cpp/ -I../ffmpeg_cpp/compat/atomics/gcc/ -I../ffmpeg_cpp/decoder/ -I../ffmpeg_cpp/decoder/x86/
#LDFLAGS   := -L/export/views/video/ffmpeg/Obj/

#/export/views/video/ffmpeg/libavutil/libavutil.a /export/views/video/ffmpeg/libavcodec/libavcodec.a
LDLIBS    := -lpthread -lm -ldl  -lssl -lcrypto -lz -lfdk-aac -no-pie


#SLIBS     := $(OUT_FILE_NAME)
SLIBS     := $(OUT_FILE_NAME)  /usr/local/lib/libx264.a



#SLIBS     := /data/mozilla/webrtc-builds/out/webrtc-22215-d8831bb-linux-x64/lib/x64/Debug/libwebrtc_full.a


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

