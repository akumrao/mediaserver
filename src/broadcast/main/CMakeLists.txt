
#ninja -t clean webrtc
#ninja -j8 -v  webrtc


# documentation: To comple with cmake.. and make -j directly, enable line SET(NVR "/workspace/acvs-VideoEdge-webrtc/NVR")



set(NVR "${PROJECTPATH}")
set(webrtc-devel "${AD_3RDPARTY_DIR}/webrtc-devel")
SET(WEBRTC_PATH_INC "${webrtc-devel}/include/webrtc")
SET(WEBRTC_PATH_LIB "${webrtc-devel}/lib")
SET(FFMPEG_ROOT "/usr/include/ffmpeg")


#cmake_minimum_required(VERSION 3.4.1)
#SET(WEBRTC_PATH_INC "/workspace/webrtc_android/src")
#SET(WEBRTC_PATH_LIB "/workspace/webrtc_android/src/out/m75/obj")
#SET(FFMPEG_ROOT "/workspace/ffmpeg")
#SET(NVR "/workspace/nvrmain/acvs-videoedge-nvr/NVR")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g")
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)



SET(THIRDPARTY "${NVR}/3rdParty")
SET(RTC "${NVR}/3rdParty/rtc/src")
SET(xstore "${NVR}/xStore")




file(GLOB SOURCES "*.cpp" "../src/*.cpp" "../videoEdge/controller.cpp" "../videoEdge/mediaread_if.cpp" "../videoEdge/pipeline.cpp" "../videoEdge/alarm_pipeline.cpp" "../videoEdge/transmitSession.cpp" "${RTC}/libuv/src/*.cpp" "${RTC}/libuv/src/unix/*.cpp" "../../base/src/*.cpp"  "../../net/src/*.cpp" "../../http/src/*.cpp" "${RTC}/http_parser/*.cpp" "../../signal/src/*.cpp" "../../json/src/*.cpp" )


#set(CMAKE_C_COMPILER "gcc")
#set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")





#set (CMAKE_POSITION_INDEPENDENT_CODE OFF)
#set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-pie")
#set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fno-pie")
#set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-no-pie")



add_executable(webrtc  ${SOURCES})

target_compile_definitions(webrtc PRIVATE OPENSSL_IS_BORINGSSL WEBRTC_POSIX  WEBRTC_INCLUDE_DIRS=${WEBRTC_PATH_INC} )


target_include_directories(webrtc PRIVATE   /usr/include/gstreamer-1.0 /usr/include/x86_64-linux-gnu /usr/include/glib-2.0 /usr/lib/x86_64-linux-gnu/glib-2.0/include /usr/lib64/glib-2.0/include ../../json/include/ ${FFMPEG_ROOT}/  ../include ../../signal/include ../../base/token/ ../../http/include/ ../../base/include/ ../../net/include/ ${RTC}/libuv/include ${RTC}/libuv/src ../../../build/ ${RTC}/http_parser/ ../webrtc/include  ${WEBRTC_PATH_INC} ${WEBRTC_PATH_INC}/third_party/abseil-cpp  ${WEBRTC_PATH_INC}/third_party/libvpx/source/libvpx   ../videoEdge/  ${xstore}/include/ ${xstore}/libutil ${xstore}/libfailover/   ${xstore}/playback/libs/libstreamcaps/include/ ${xstore}/playback/rtsp/libs/libprimitives/include/  ${xstore}/playback/rtp/include/   ${xstore}/libvegst/  ${xstore}/playback/transmit_manager/libs/libtmgr/include ${xstore}/libvirtualcam ${xstore}/playback/libs/libpalette/include/ ${xstore}/libTranscodeCalculator/   ${xstore}/playback/rtp/libs/include/ ${xstore}/veapi/libveapitext/ ${xstore}/playback/rtsp/libs/libpreprocessor/include/ ${xstore}/libmt/src/ ${xstore}/common/serializers/trivialserializer/ ${xstore}/common/others/ioc/ ${xstore}/playback/transmit_manager/libs/libautoconfigstream/include/     ${xstore}/libinputconfigutils/  ${xstore}/libvecgiutils/    ${xstore}/libstreamConfig    ${xstore}/libbackuprestore     /usr/include/libnl3   /usr/include/pgsql   /usr/include/pango-1.0     /usr/include/cairo   /opt/americandynamics/3rdParty/include/rapidxml  ${xstore}/failover/libfailoverdefs  ${THIRDPARTY}/libh/src/include  ${THIRDPARTY}/nlohmann-json/src/  ${xstore}/playback/transmit_manager/libs/libtranscodemib    ${xstore}/playback/transmit_manager/libs/libtranscodestream   ${xstore}/playback/rtsp/libs/librtsp/include  ${xstore}/playback/rtsp/libs/liburi/include   ${xstore}/playback/rtsp/include  ${xstore}/common/time  ${xstore}/wan/libbandwidthmanager  ${xstore}/libvecommon/logging/  ${xstore}/playback/rtsp/libs/libpayload/include   ${xstore}/libSysMon/ ${xstore}/bwmonitor ${xstore}/libvesysconf  ${xstore}/failover/libfailovernvrstate  ${xstore}/playback/rtp/playbackelements/include/  ${xstore}/veapi/libveapi/ ${xstore}/libnvrauth/ /opt/intel/mediasdk/include )


target_link_options(webrtc PRIVATE -no-pie)

target_compile_options(webrtc PRIVATE -Wno-return-type -Wno-format-overflow -Wno-format -Wno-unused-but-set-variable -Wno-write-strings -no-pie)
target_compile_features(webrtc PRIVATE cxx_std_17)

target_link_directories(webrtc PRIVATE ${xstore}/appliance/build/root/usr/lib64/ ${NVR}/deliverables/staging/rootfs/opt/americandynamics/venvr/lib/ /opt/intel/mediasdk/lib64 /opt/intel/mediasdk/lib/lin_x64)

target_link_libraries( webrtc PRIVATE  ${WEBRTC_PATH_LIB}/libwebrtc.a avformat avcodec avutil swresample swscale avdevice avutil avfilter pthread m dl z  ssl crypto gstreamer-1.0 gobject-2.0 glib-2.0 gstapp-1.0 X11 va va-drm va-x11 X11 drm play_mediadbsrc vegst rtptransmit_log mediadatabase streamcaps cam veutil  tmgr transcodemib  autoconfigstream bandwidthmanagerserver cmdparam boost_filesystem boost_program_options boost_regex payload virtualcam vecommon failover failovernvrstate primitives preprocessor palette vertsp uri log4cpp vecgiutils inputconfigutils veapi json-glib-1.0

 )




#target_link_libraries( webrtc PRIVATE
#    Boost::program_options
#    Boost::thread
#    PkgConfig::GLib
#    PkgConfig::GStreamer
#    payload
#    tmgr
#    transcodemib
#    vecommon
#    vesysconf
#)






