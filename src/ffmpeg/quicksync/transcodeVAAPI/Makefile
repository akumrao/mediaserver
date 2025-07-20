### CUSTOMIZE BELOW HERE #######################################################
FFMPEG_ROOT = /workspace/ffmpeg

CC	= gcc
CXX     = g++
TARGET= qsvTranscodeVAapi
SOURCES=$(wildcard *.cpp )

CXXFLAGS   := -g -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized -fpermissive

CPPFLAGS   := -I${FFMPEG_ROOT}/ -I/opt/intel/mediasdk/include/mfx  -I/opt/intel/mediasdk/include
              
LDFLAGS   := -L/usr/lib/x86_64-linux-gnu


LDLIBS    := -lpthread -lm -ldl -lm -ldl -lz -lfdk-aac -lmfx -lva -lva-drm -lva-x11 -lX11 -ldrm

# SLIBS     := ${FFMPEG_ROOT}/libavformat/libavformat.a ${FFMPEG_ROOT}/libavcodec/libavcodec.a ${FFMPEG_ROOT}/libavutil/libavutil.a ${FFMPEG_ROOT}/libswresample/libswresample.a /usr/local/lib/libx264.a
SLIBS     := ${FFMPEG_ROOT}/libavfilter/libavfilter.a  ${FFMPEG_ROOT}/libavformat/libavformat.a ${FFMPEG_ROOT}/libavcodec/libavcodec.a ${FFMPEG_ROOT}/libswscale/libswscale.a  ${FFMPEG_ROOT}/libavutil/libavutil.a ${FFMPEG_ROOT}/libswresample/libswresample.a ${FFMPEG_ROOT}/libavdevice/libavdevice.a  ${FFMPEG_ROOT}/libpostproc/libpostproc.a  /usr/local/lib/libx264.a


# LDFLAGS   := -L/usr/lib/x86_64-linux-gnu -L/opt/intel/mediasdk/lib64 -L/opt/intel/mediasdk/lib/lin_x64

# LDLIBS    := -lpthread -lm -ldl -lm -ldl -lz -lmfx -lva -lva-drm  -ldrm -lavformat -lavcodec -lavutil -lswresample -lswscale -lavdevice -lavutil -lavfilter


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
