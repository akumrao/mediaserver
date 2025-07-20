#
# http://www.gnu.org/software/make/manual/make.html
#
FFMPEG_ROOT = /workspace/ffmpeg
CC:=g++


CC	= gcc
CXX     = g++
TARGET=runMD # define the name of the executable
SOURCES=$(wildcard *.cpp ./MVAnalysis/*.cpp  )

CXXFLAGS   := -g -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized -Wno-deprecated-declarations

CPPFLAGS   := -I./MVAnalysis/ -I${FFMPEG_ROOT}/ -I/usr/include/opencv4 
              
LDFLAGS   := -L/usr/lib/x86_64-linux-gnu


LDLIBS    := -lpthread -lz -lm -ldl -lfdk-aac -lopencv_highgui -lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs -lopencv_core -lpng -lopencv_calib3d -lopencv_photo `sdl2-config --cflags --libs`

SLIBS     :=  ${FFMPEG_ROOT}/libavformat/libavformat.a  ${FFMPEG_ROOT}/libavfilter/libavfilter.a  ${FFMPEG_ROOT}/libavcodec/libavcodec.a ${FFMPEG_ROOT}/libavutil/libavutil.a  ${FFMPEG_ROOT}/libswscale/libswscale.a ${FFMPEG_ROOT}/libswresample/libswresample.a /usr/local/lib/libx264.a 


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