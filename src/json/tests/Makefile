################################################################################
### CUSTOMIZE BELOW HERE #######################################################
TARGET_ARCH = mips

TARGET=runConfTest # define the name of the executable

SOURCES=$(wildcard *.cpp ../src/*.cpp ../../base/src/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp )

CXXFLAGS   +=  -g -uClibc++ -std=c++11 -uClibc++ -std=c++11  -Wl,-Bstatic -lm -Wl,-Bdynamic -Wl,--dynamic-linker=/lib/ld-uClibc.so.0  

CPPFLAGS   += -I../../json/include/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser -I../xailient-sdk/include/ 

LDFLAGS   += -L../xailient-sdk/lib/ -L/experiment/mediaserver/src/json/xailient-sdk/lib/

LDLIBS    += -lpthread -ldl -lxailient-fi


# /workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/resource/toolchain/gcc_540/mips-gcc540-glibc222-64bit-r3.3.0/mips-linux-gnu/libc/uclibc/usr/lib/libm.a 

#-uClibc++ -std=c++11  -Wl,-Bstatic -lm -Wl,-Bdynamic -Wl,--dynamic-linker=/lib/ld-uClibc.so.0 


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
