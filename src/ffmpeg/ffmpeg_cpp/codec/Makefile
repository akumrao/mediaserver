
CC = gcc
CXX = g++
TARGET_FILE_NAME = decode_video.a
DECODER_FOLDER = decoder

CFLAGS = -g -Wall -c -fPIC
CFLAGS_TARGET := -g -Wno-inconsistent-missing-override -Wno-uninitialized -Wno-attributes -Wno-fpermissive -Wno-deprecated-declarations -fPIC 
INC = -I../$(DECODER_FOLDER) -I../../ffmpeg -I../$(DECODER_FOLDER)/x86

OUT_DIR=./

.DEFAULT_GOAL=all
.PHONY: all
all: $(SOURCES) $(OBJS) $(TARGET_FILE_NAME)

#SOURCES = decode_video.c

# define list of objects
SOURCES = $(patsubst %.c,%.o,$(wildcard ../$(DECODER_FOLDER)/*.c ../$(DECODER_FOLDER)/x86/*.c))
OBJS=$(SOURCES:.c=.o)
LDLIBS    := -lpthread -lm -ldl -lfdk-aac -no-pie

# Enumerating of every *.cpp as *.o and using that as dependency
$(TARGET_FILE_NAME):$(OBJS) 
	$(CXX) -c $(INC) $(CFLAGS) -o $@  $<
	ar -r -o $(OUT_DIR)/$@ $^

#Compiling every *.c to *.o
%.o: %.c 
	$(CXX) -c $(INC) $(CFLAGS) -o $@  $<
	
dirmake:
	@mkdir -p $(OUT_DIR)
	
clean:
	rm -f *.o
	rm -f *.d
	rm -f ./x86/*.o
	rm -f ./x86/*.d	
	rm -f $(TARGET_FILE_NAME)

rebuild: clean build

purge: clean
	rm -f $(TARGET_FILE_NAME)
