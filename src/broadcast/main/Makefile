################################################################################
### CUSTOMIZE BELOW HERE #######################################################

LIVE555_ROOT = /workspace/live
FFMPEG_ROOT = /workspace/ffmpeg
#WEBRTC_PATH = /data/delete/mediaserverdocker/webrtc_android/src/
WEBRTC_PATH = /workspace/webrtc_android/src/

CC	= gcc
CXX     = g++
TARGET=runWebrtc # define the name of the executable
SOURCES=$(wildcard *.cpp ../src/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp ../../base/src/*.cpp ../../base/token/*.cpp ../../net/src/*.cpp ../../http/src/*.cpp ../../http_parser/*.cpp ../../signal/src/*.cpp ../../json/src/*.cpp )

CXXFLAGS   := -g -fno-rtti -std=c++14 -Wno-inconsistent-missing-override -Wno-uninitialized

CPPFLAGS   := -I../../json/include/ -I/workspace/ffmpeg/  -I../include -I../../signal/include -I../../base/token/ -I../../http/include/ -I../../base/include/ -I../../net/include/ -I../../libuv/include -I../../libuv/src -I../../../build/ -I../../http_parser/ -I../webrtc/include  -DOPENSSL_IS_BORINGSSL -DWEBRTC_POSIX  -DWEBRTC_INCLUDE_DIRS=/workspace/webrtc_android/src  -I${LIVE555_ROOT}/liveMedia/include -I${LIVE555_ROOT}/groupsock/include -I${LIVE555_ROOT}/BasicUsageEnvironment/include -I${LIVE555_ROOT}/UsageEnvironment/include  \
               -I${WEBRTC_PATH} -I${WEBRTC_PATH}/third_party/abseil-cpp -I${WEBRTC_PATH}/third_party/libvpx/source/libvpx -I/opt/intel/mediasdk/include/mfx  -I/opt/intel/mediasdk/include -DHTTPSSL -DRTC_ENABLE_VP9
LDFLAGS   :=  -Wl,-rpath,/usr/local/lib -L/opt/intel/mediasdk/lib64 -L/opt/intel/mediasdk/lib/lin_x64 -L/opt/intel/mediasdk/lib

LDLIBS    := -lpthread -lm -ldl -lz -lssl -lcrypto -lmfx -lva -lva-drm -lva-x11 -lbz2 -ldrm
#LDLIBS    := -lpthread -lm -ldl -lz -lfdk-aac 

#SLIBS     := ${WEBRTC_PATH}/out/m75/obj/libwebrtc.a  ${FFMPEG_ROOT}/libavformat/libavformat.a ${FFMPEG_ROOT}/libavcodec/libavcodec.a ${FFMPEG_ROOT}/libavutil/libavutil.a ${FFMPEG_ROOT}/libswresample/libswresample.a ${LIVE555_ROOT}/liveMedia/libliveMedia.a  ${LIVE555_ROOT}/groupsock/libgroupsock.a   ${LIVE555_ROOT}/BasicUsageEnvironment/libBasicUsageEnvironment.a  ${LIVE555_ROOT}/UsageEnvironment/libUsageEnvironment.a /usr/local/lib/libx264.a
SLIBS     := ${WEBRTC_PATH}/out/m75/obj/libwebrtc.a ${FFMPEG_ROOT}/libavfilter/libavfilter.a  ${FFMPEG_ROOT}/libavformat/libavformat.a ${FFMPEG_ROOT}/libavcodec/libavcodec.a ${FFMPEG_ROOT}/libswscale/libswscale.a  ${FFMPEG_ROOT}/libavutil/libavutil.a ${FFMPEG_ROOT}/libswresample/libswresample.a ${FFMPEG_ROOT}/libavdevice/libavdevice.a  ${LIVE555_ROOT}/liveMedia/libliveMedia.a  ${LIVE555_ROOT}/groupsock/libgroupsock.a  ${LIVE555_ROOT}/BasicUsageEnvironment/libBasicUsageEnvironment.a  ${LIVE555_ROOT}/UsageEnvironment/libUsageEnvironment.a /usr/local/lib/libx264.a ${FFMPEG_ROOT}/libpostproc/libpostproc.a /workspace/x264/libx264.a



################################################################################
### DO NOT EDIT THE FOLLOWING LINES ############################################


# define list of objects
OBJSC=$(SOURCES:.cpp=.o)
OBJS=$(OBJSC:.cpp=.o)

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

# the target is obtained linking all .o files
all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJS) $(SLIBS) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

purge: clean
	rm -f $(TARGET)

clean:
	rm -f  $(OBJS)
rpm:
	rpmbuild -ba --build-in-place --define "_topdir $(mkfile_dir)/rpm" runWebrtc.spec --define '_bindir /workspace/acvs-VideoEdge-webrtc/NVR/xStore/rtc/src/webrtc/main'  --define '_sysconfigdir /workspace/acvs-VideoEdge-webrtc/NVR/xStore/rtc/src/webrtc/signalserver/codelab'

copyrpm:
	mv rpm/RPMS/*/*.rpm . && rm -rf rpm


################################################################################
################################################################################
