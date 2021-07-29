################################################################################
### CUSTOMIZE BELOW HERE #######################################################

CC	= gcc
CXX     = g++
TARGET=runHttp # define the name of the executable
SOURCES=$(wildcard *.cpp ../src/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp ../../base/src/*.cpp ../../crypto/src/*.cpp ../../net/src/*.cpp ../../net/uv_multiplex/src/*.cpp ../../http_parser/*.cpp )

CXXFLAGS   := -g  -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized
 

CPPFLAGS   := -I../../json/include/ -I../../crypto/include/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../net/uv_multiplex/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ 

              
LDFLAGS   := -L/data/sound/lib -L/usr/lib/x86_64-linux-gnu



LDLIBS    := -lpthread -lm -ldl  -lssl -lcrypto  

#SLIBS     := /data/mozilla/webrtc-builds/out/webrtc-22215-d8831bb-linux-x64/lib/x64/Debug/libwebrtc_full.a


################################################################################
### DO NOT EDIT THE FOLLOWING LINES ############################################


# define list of objects
OBJSC=$(SOURCES:.cpp=.o)
OBJS=$(OBJSC:.cpp=.o)

# the target is obtained linking all .o files
all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) $(SLIBS) -o $(TARGET)

purge: clean
	rm -f $(TARGET)

clean:
	rm -f  $(OBJS)

################################################################################
################################################################################
