################################################################################
### CUSTOMIZE BELOW HERE #######################################################

CC	= clang
CXX     = clang++
TARGET=runStreamer # define the name of the executable
SOURCES=$(wildcard *.cpp ../src/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp ../../base/src/*.cpp ../../crypto1/src/*.cpp ../../net/src/*.cpp ../../http/src/*.cpp ../../http_parser/*.cpp ../../signal/src/*.cpp ../../ffmpeg/src/*.cpp )

CXXFLAGS   := -g -stdlib=libc++ -std=c++11 -Wno-inconsistent-missing-override -Wno-uninitialized

CPPFLAGS   := -I../../json/include/ -I../../signal/include -I../../crypto/include/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ -I../../webrtc/include -I../../ffmpeg/include -DOPENSSL_IS_BORINGSSL -DWEBRTC_POSIX -D_GLIBCXX_USE_CXX11_ABI=0 -DWEBRTC_INCLUDE_DIRS=/data/del/webrtc-builds/out/webrtc-22215-d8831bb-linux-x64/include \
               -I/data/del/webrtc-builds/out/webrtc-22215-d8831bb-linux-x64/include
LDFLAGS   := -L/usr/lib/x86_64-linux-gnu


LDLIBS    := -lpthread -lm -ldl  -lX11 -lavformat -lavcodec -lavutil -lswresample -lswscale -lavdevice -lavutil

SLIBS     := /data/del/webrtc-builds/out/webrtc-22215-d8831bb-linux-x64/lib/x64/Debug/libwebrtc_full.a


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
