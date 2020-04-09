################################################################################
### CUSTOMIZE BELOW HERE #######################################################

CC	= clang
CXX     = clang++
TARGET=sfu # define the name of the executable
SOURCES=$(wildcard *.cpp ../src/*.cpp ../src/RTCP/*.cpp ../src/Codecs/*.cpp ../src/handles/*.cpp ../src/SctpDictionaries/*.cpp ../src/Channel/*.cpp ../src/Utils/*.cpp ../src/RtpDictionaries/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp ../../base/src/*.cpp ../../net/src/*.cpp ../../http/src/*.cpp ../../http_parser/*.cpp ../../signal/src/*.cpp)

CXXFLAGS   := -g -std=c++11 -Wno-inconsistent-missing-override -Wno-uninitialized

CPPFLAGS   := -I../include/ -I../../json/include/ -I../../json/include/json -I../../signal/include -I../../crypto/include/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ -I../deps/libsrtp/srtp/include -I../deps/libwebrtc/ -I../deps/libwebrtc/libwebrtc -I../deps/libwebrtc/deps/abseil-cpp/abseil-cpp -DMS_LITTLE_ENDIAN -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DDEBUG -DMS_LOG_TRACE -DMS_LOG_FILE_LINE

LDFLAGS   := -L../out/Release/

LDLIBS    := -lpthread -lm -ldl 

SLIBS     := ../out/Release/libopenssl.a ../out/Release/libsrtp.a ../out/Release/libusrsctp.a ../out/Release/libwebrtc.a ../out/Release/libabseil.a 


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
