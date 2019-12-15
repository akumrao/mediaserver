################################################################################
### CUSTOMIZE BELOW HERE #######################################################

CC	= clang
CXX     = g++
TARGET=runKafka # define the name of the executable
SOURCES=$(wildcard *.cpp ../src/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp ../../crypto/src/*.cpp ../../base/src/*.cpp )

CXXFLAGS   := -g -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized


CPPFLAGS   := -I./cppkafka/include/ -I../../json/include/ -I../../crypto/include/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ 
LDFLAGS   := -L/data/sound/lib -L/usr/lib/x86_64-linux-gnu 



LDLIBS    := -lz -lpthread -lm -ldl -lssl -lcrypto 

SLIBS     := ./cppkafka/build/src/lib/libcppkafka.a ./librdkafka/src-cpp/librdkafka++.a  ./librdkafka/src/librdkafka.a


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
