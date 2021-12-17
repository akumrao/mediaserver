# TODO: determine CMAKE_SYSTEM_NAME on OS/390.  Currently assumes "OS/390".
cmake_minimum_required(VERSION 3.4)
project(libuv LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(GNUInstallDirs)
include(CTest)

cmake_dependent_option(LIBUV_BUILD_TESTS
  "Build the unit tests when BUILD_TESTING is enabled and we are the root project" ON
  "BUILD_TESTING;CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

if(MSVC)
  list(APPEND uv_cflags /W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
  #list(APPEND uv_cflags -fvisibility=hidden --std=gnu89)
  list(APPEND uv_cflags -Wall -Wextra -Wstrict-prototypes)
  list(APPEND uv_cflags -Wno-unused-parameter)
endif()

set(uv_sources
    ./src/libuv/src/fs-poll.cpp
    ./src/libuv/src/idna.cpp
    ./src/libuv/src/inet.cpp
    ./src/libuv/src/random.cpp
    ./src/libuv/src/strscpy.cpp
    ./src/libuv/src/threadpool.cpp
    ./src/libuv/src/timer.cpp
    ./src/libuv/src/uv-common.cpp
    ./src/libuv/src/uv-data-getter-setters.cpp
    ./src/libuv/src/version.cpp)

set(uv_test_sources
    test/blackhole-server.cpp
    test/test-walk-handles.cpp
    test/test-watcher-cross-stop.cpp)

if(WIN32)
  list(APPEND media_defines SCY_WIN)
  list(APPEND uv_defines WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0600)
  list(APPEND uv_libraries
       advapi32
       iphlpapi
       psapi
       shell32
       user32
       userenv
       ws2_32)
  list(APPEND uv_sources
       ./src/libuv/src/win/async.cpp
       ./src/libuv/src/win/core.cpp
       ./src/libuv/src/win/detect-wakeup.cpp
       ./src/libuv/src/win/dl.cpp
       ./src/libuv/src/win/error.cpp
       ./src/libuv/src/win/fs.cpp
       ./src/libuv/src/win/fs-event.cpp
       ./src/libuv/src/win/getaddrinfo.cpp
       ./src/libuv/src/win/getnameinfo.cpp
       ./src/libuv/src/win/handle.cpp
       ./src/libuv/src/win/loop-watcher.cpp
       ./src/libuv/src/win/pipe.cpp
       ./src/libuv/src/win/thread.cpp
       ./src/libuv/src/win/poll.cpp
       ./src/libuv/src/win/process.cpp
       ./src/libuv/src/win/process-stdio.cpp
       ./src/libuv/src/win/signal.cpp
       ./src/libuv/src/win/snprintf.cpp
       ./src/libuv/src/win/stream.cpp
       ./src/libuv/src/win/tcp.cpp
       ./src/libuv/src/win/tty.cpp
       ./src/libuv/src/win/udp.cpp
       ./src/libuv/src/win/util.cpp
       ./src/libuv/src/win/winapi.cpp
       ./src/libuv/src/win/winsock.cpp)
  list(APPEND uv_test_libraries ws2_32)
  list(APPEND uv_test_sources ./src/libuv/src/win/snprintf.cpp test/runner-win.cpp)
else()
  list(APPEND uv_defines _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE)
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
    # Android has pthread as part of its c library, not as a separate
    # libpthread.so.
    list(APPEND uv_libraries pthread)
  endif()
  list(APPEND uv_sources
       ./src/libuv/src/unix/async.cpp
       ./src/libuv/src/unix/core.cpp
       ./src/libuv/src/unix/dl.cpp
       ./src/libuv/src/unix/fs.cpp
       ./src/libuv/src/unix/getaddrinfo.cpp
       ./src/libuv/src/unix/getnameinfo.cpp
       ./src/libuv/src/unix/loop-watcher.cpp
       ./src/libuv/src/unix/loop.cpp
       ./src/libuv/src/unix/pipe.cpp
       ./src/libuv/src/unix/poll.cpp
       ./src/libuv/src/unix/process.cpp
       ./src/libuv/src/unix/random-devurandom.cpp
       ./src/libuv/src/unix/signal.cpp
       ./src/libuv/src/unix/stream.cpp
       ./src/libuv/src/unix/tcp.cpp
       ./src/libuv/src/unix/thread.cpp
       ./src/libuv/src/unix/tty.cpp
       ./src/libuv/src/unix/udp.cpp)
  list(APPEND uv_test_sources test/runner-unix.cpp)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
  list(APPEND uv_defines
       _ALL_SOURCE
       _LINUX_SOURCE_COMPAT
       _THREAD_SAFE
       _XOPEN_SOURCE=500)
  list(APPEND uv_libraries perfstat)
  list(APPEND uv_sources ./src/libuv/src/unix/aix.cpp)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  list(APPEND uv_libs dl)
  list(APPEND uv_sources
       ./src/libuv/src/unix/android-ifaddrs.cpp
       ./src/libuv/src/unix/linux-core.cpp
       ./src/libuv/src/unix/linux-inotify.cpp
       ./src/libuv/src/unix/linux-syscalls.cpp
       #./src/libuv/src/unix/procfs-exepath.cpp
       ./src/libuv/src/unix/pthread-fixes.cpp
       #./src/libuv/src/unix/sysinfo-loadavg.cpp
)
endif()

if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "Android|Linux|OS/390")
  list(APPEND uv_sources ./src/libuv/src/unix/proctitle.cpp)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD")
  list(APPEND uv_sources ./src/libuv/src/unix/freebsd.cpp)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|NetBSD|OpenBSD")
  list(APPEND uv_sources ./src/libuv/src/unix/posix-hrtime.cpp ./src/libuv/src/unix/bsd-proctitle.cpp)
  list(APPEND uv_libraries kvm)
endif()

if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|NetBSD|OpenBSD")
  list(APPEND uv_sources ./src/libuv/src/unix/bsd-ifaddrs.cpp ./src/libuv/src/unix/kqueue.cpp)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  list(APPEND uv_sources ./src/libuv/src/unix/random-getrandom.cpp)
endif()

if(APPLE OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
  list(APPEND uv_sources ./src/libuv/src/unix/random-getentropy.cpp)
endif()

if(APPLE)
  list(APPEND uv_defines _DARWIN_UNLIMITED_SELECT=1 _DARWIN_USE_64_BIT_INODE=1)
  list(APPEND uv_sources
       ./src/libuv/src/unix/darwin-proctitle.cpp
       ./src/libuv/src/unix/darwin.cpp
       ./src/libuv/src/unix/fsevents.cpp)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND uv_defines _GNU_SOURCE _POSIX_C_SOURCE=200112)
  list(APPEND uv_libraries dl rt)
  list(APPEND uv_sources
       ./src/libuv/src/unix/linux-core.cpp
       ./src/libuv/src/unix/linux-inotify.cpp
       ./src/libuv/src/unix/linux-syscalls.cpp
       #./src/libuv/src/unix/procfs-exepath.cpp
       ./src/libuv/src/unix/random-getrandom.cpp
       ./src/libuv/src/unix/random-sysctl.cpp
       #./src/libuv/src/unix/sysinfo-loadavg.cpp
)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
  list(APPEND uv_sources ./src/libuv/src/unix/netbsd.cpp)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
  list(APPEND uv_sources ./src/libuv/src/unix/openbsd.cpp)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "OS/390")
  list(APPEND uv_defines PATH_MAX=255)
  list(APPEND uv_defines _AE_BIMODAL)
  list(APPEND uv_defines _ALL_SOURCE)
  list(APPEND uv_defines _LARGE_TIME_API)
  list(APPEND uv_defines _OPEN_MSGQ_EXT)
  list(APPEND uv_defines _OPEN_SYS_FILE_EXT)
  list(APPEND uv_defines _OPEN_SYS_IF_EXT)
  list(APPEND uv_defines _OPEN_SYS_SOCK_EXT3)
  list(APPEND uv_defines _OPEN_SYS_SOCK_IPV6)
  list(APPEND uv_defines _UNIX03_SOURCE)
  list(APPEND uv_defines _UNIX03_THREADS)
  list(APPEND uv_defines _UNIX03_WITHDRAWN)
  list(APPEND uv_defines _XOPEN_SOURCE_EXTENDED)
  list(APPEND uv_sources
       ./src/libuv/src/unix/pthread-fixes.cpp
       ./src/libuv/src/unix/pthread-barrier.cpp
       ./src/libuv/src/unix/os390.cpp
       ./src/libuv/src/unix/os390-syscalls.cpp)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  list(APPEND uv_defines __EXTENSIONS__ _XOPEN_SOURCE=500)
  list(APPEND uv_libraries kstat nsl sendfile socket)
  list(APPEND uv_sources ./src/libuv/src/unix/no-proctitle.cpp ./src/libuv/src/unix/sunos.cpp)
endif()

if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|Linux|NetBSD|OpenBSD")
  list(APPEND uv_test_libraries util)
endif()

add_library(uv SHARED ${uv_sources})
target_compile_definitions(uv
                           INTERFACE USING_UV_SHARED=1
                           PRIVATE ${uv_defines} BUILDING_UV_SHARED=1)
target_compile_options(uv PRIVATE ${uv_cflags})
target_include_directories(uv PUBLIC ./src/libuv/include PRIVATE ./src/libuv/src)
target_link_libraries(uv ${uv_libraries})

add_library(uv_a STATIC ${uv_sources})
target_compile_definitions(uv_a PRIVATE ${uv_defines})
target_compile_options(uv_a PRIVATE ${uv_cflags})
target_include_directories(uv_a PUBLIC ./src/libuv/include PRIVATE ./src/libuv/src)
target_link_libraries(uv_a ${uv_libraries})


file(GLOB SOURCES "./src/base/src/*.cpp" "./src/http_parser/*.cpp" "./src/json/src/*.cpp" )

list(APPEND SOURCES
./src/net/src/IP.cpp
#./src/net/src/SslConnection.cpp
./src/net/src/TcpServer.cpp
#./src/net/src/ssladapter.cpp
./src/net/src/PortManager.cpp
./src/net/src/TcpConnection.cpp 
./src/net/src/UdpSocket.cpp
./src/http/src/HTTPResponder.cpp 
#./src/http/src/HttpsConn.cpp  
./src/http/src/message.cpp    
./src/http/src/url.cpp
./src/http/src/HttpClient.cpp 
./src/http/src/client.cpp 
./src/http/src/packetizers.cpp
./src/http/src/websocket.cpp
./src/http/src/HttpConn.cpp 
./src/http/src/cookie.cpp 
./src/http/src/parser.cpp
./src/http/src/HttpServer.cpp  
./src/http/src/form.cpp    
./src/http/src/request.cpp
#./src/http/src/HttpsClient.cpp  
./src/http/src/httputil.cpp 
./src/http/src/response.cpp

./src/rtsp/ffmpeg/aac_ac3_parser.cpp
./src/rtsp/ffmpeg/ac3_parser.cpp
./src/rtsp/ffmpeg/ac3tab.cpp
./src/rtsp/ffmpeg/aes.cpp
./src/rtsp/ffmpeg/aes_ctr.cpp
./src/rtsp/ffmpeg/allcodecs.cpp
./src/rtsp/ffmpeg/allformats.cpp
./src/rtsp/ffmpeg/atomic.cpp
./src/rtsp/ffmpeg/audio_frame_queue.cpp
./src/rtsp/ffmpeg/audiointerleave.cpp
./src/rtsp/ffmpeg/avbuffer.cpp
./src/rtsp/ffmpeg/avc.cpp
./src/rtsp/ffmpeg/avfft.cpp
./src/rtsp/ffmpeg/avio.cpp
./src/rtsp/ffmpeg/aviobuf.cpp
./src/rtsp/ffmpeg/avpacket.cpp
./src/rtsp/ffmpeg/avparser.cpp
./src/rtsp/ffmpeg/avstring.cpp
./src/rtsp/ffmpeg/avtime.cpp
./src/rtsp/ffmpeg/bitstream.cpp
./src/rtsp/ffmpeg/bitstream_filters.cpp
./src/rtsp/ffmpeg/bprint.cpp
./src/rtsp/ffmpeg/bsf.cpp
./src/rtsp/ffmpeg/channel_layout.cpp
./src/rtsp/ffmpeg/codec_desc.cpp
./src/rtsp/ffmpeg/color_utils.cpp
./src/rtsp/ffmpeg/cpu.cpp
./src/rtsp/ffmpeg/crc.cpp
./src/rtsp/ffmpeg/cuvid.cpp
./src/rtsp/ffmpeg/dct.cpp
./src/rtsp/ffmpeg/dict.cpp
./src/rtsp/ffmpeg/error.cpp
./src/rtsp/ffmpeg/eval.cpp
./src/rtsp/ffmpeg/faandct.cpp
./src/rtsp/ffmpeg/fdctdsp.cpp
./src/rtsp/ffmpeg/fifo.cpp
./src/rtsp/ffmpeg/file_open.cpp
./src/rtsp/ffmpeg/format.cpp
./src/rtsp/ffmpeg/frame_thread_encoder.cpp
./src/rtsp/ffmpeg/h264_mp4toannexb_bsf.cpp
./src/rtsp/ffmpeg/hwcontext.cpp
./src/rtsp/ffmpeg/id3v1.cpp
./src/rtsp/ffmpeg/id3v2.cpp
./src/rtsp/ffmpeg/imgutils.cpp
./src/rtsp/ffmpeg/intmath.cpp
./src/rtsp/ffmpeg/isom.cpp
./src/rtsp/ffmpeg/libfdk-aacdec.cpp
./src/rtsp/ffmpeg/libfdk-aacenc.cpp
./src/rtsp/ffmpeg/live.cpp
./src/rtsp/ffmpeg/livethread.cpp
./src/rtsp/ffmpeg/log.cpp
./src/rtsp/ffmpeg/mathematics.cpp
./src/rtsp/ffmpeg/mem.cpp
./src/rtsp/ffmpeg/metadata.cpp
./src/rtsp/ffmpeg/mov_chan.cpp
./src/rtsp/ffmpeg/movenc.cpp
./src/rtsp/ffmpeg/movenccenc.cpp
./src/rtsp/ffmpeg/movenchint.cpp
./src/rtsp/ffmpeg/mpeg12data.cpp
./src/rtsp/ffmpeg/mpeg4_unpack_bframes_bsf.cpp
./src/rtsp/ffmpeg/mpeg4audio.cpp
./src/rtsp/ffmpeg/mpegaudiodata.cpp
./src/rtsp/ffmpeg/mpegpicture.cpp
./src/rtsp/ffmpeg/mpegutils.cpp
./src/rtsp/ffmpeg/mpegvideodsp.cpp
./src/rtsp/ffmpeg/mpegvideoencdsp.cpp
./src/rtsp/ffmpeg/mux.cpp
./src/rtsp/ffmpeg/muxframe.cpp
./src/rtsp/ffmpeg/network.cpp
./src/rtsp/ffmpeg/opt.cpp
./src/rtsp/ffmpeg/options.cpp
./src/rtsp/ffmpeg/options_avformat.cpp
./src/rtsp/ffmpeg/os_support.cpp
./src/rtsp/ffmpeg/parseutils.cpp
./src/rtsp/ffmpeg/pixdesc.cpp
./src/rtsp/ffmpeg/profiles.cpp
./src/rtsp/ffmpeg/protocols.cpp
./src/rtsp/ffmpeg/pthread.cpp
./src/rtsp/ffmpeg/pthread_frame.cpp
./src/rtsp/ffmpeg/pthread_slice.cpp
./src/rtsp/ffmpeg/random_seed.cpp
./src/rtsp/ffmpeg/ratecontrol.cpp
./src/rtsp/ffmpeg/rational.cpp
./src/rtsp/ffmpeg/raw.cpp
./src/rtsp/ffmpeg/rawutils.cpp
./src/rtsp/ffmpeg/rdft.cpp
./src/rtsp/ffmpeg/reverse.cpp
./src/rtsp/ffmpeg/riff.cpp
./src/rtsp/ffmpeg/riffenc.cpp
./src/rtsp/ffmpeg/rl.cpp
./src/rtsp/ffmpeg/samplefmt.cpp
./src/rtsp/ffmpeg/sha.cpp
./src/rtsp/ffmpeg/stereo3d.cpp
./src/rtsp/ffmpeg/timecode.cpp
./src/rtsp/ffmpeg/utils.cpp
./src/rtsp/ffmpeg/utils_avformat.cpp
./src/rtsp/ffmpeg/utils_avutil.cpp
./src/rtsp/ffmpeg/vpcc.cpp


)

add_executable(main  mainhttp.cpp  ${SOURCES})
target_compile_definitions(main PRIVATE ${media_defines})
target_include_directories(main PUBLIC ./src/base/include ./src/http_parser ./src/json/include ./src/net/include ./src/http/include ./src/libuv/include ./src/libuv/src  ./src/signal/include ./src/rtsp/ffmpeg )
target_link_libraries(main  PUBLIC uv_a )


add_library(httpStream STATIC ${SOURCES})
target_compile_definitions(httpStream PRIVATE ${media_defines})
target_include_directories(httpStream PUBLIC ./src/base/include ./src/http_parser ./src/json/include ./src/net/include ./src/http/include ./src/libuv/include ./src/libuv/src  ./src/signal/include ./src/rtsp/ffmpeg )
target_link_libraries(httpStream  PUBLIC uv_a )



if(UNIX)
  # Now for some gibbering horrors from beyond the stars...
  foreach(x ${uv_libraries})
    set(LIBS "${LIBS} -l${x}")
  endforeach(x)
  file(STRINGS configure.ac configure_ac REGEX ^AC_INIT)
  string(REGEX MATCH [0-9]+[.][0-9]+[.][0-9]+ PACKAGE_VERSION "${configure_ac}")
  string(REGEX MATCH ^[0-9]+ UV_VERSION_MAJOR "${PACKAGE_VERSION}")
  # The version in the filename is mirroring the behaviour of autotools.
  set_target_properties(uv PROPERTIES VERSION ${UV_VERSION_MAJOR}.0.0
              SOVERSION ${UV_VERSION_MAJOR})
  set(includedir ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
  set(libdir ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
  set(prefix ${CMAKE_INSTALL_PREFIX})
  configure_file(libuv.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libuv.pc @ONLY)

  install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libuv.pc
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
  install(TARGETS uv LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(TARGETS uv_a ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

if(WIN32)
  install(DIRECTORY include/ DESTINATION include)
  install(FILES LICENSE DESTINATION .)
  install(TARGETS uv uv_a
          RUNTIME DESTINATION lib/$<CONFIG>
          ARCHIVE DESTINATION lib/$<CONFIG>)
endif()
