# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

set(CMAKE_CXX_STANDARD 14)	

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(socketio)

message( ${CMAKE_CURRENT_SOURCE_DIR} )
message( ${CMAKE_CURRENT_BINARY_DIR} )
message( ${CMAKE_SYSTEM_NAME} )


set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(./client/src/base/include ./client/src/http_parser ./client/src/crypto/include ./client/src/json/include ./client/src/net/include ./client/src/http/include ./client/src/libuv/include ./client/src/libuv/src ./client/src/libuv/src/unix ./client/src/signal/include )

file(GLOB SOURCES "S*.cpp"  "./client/src/libuv/src/*.cpp" "./client/src/base/src/*.cpp" "./client/src/net/src/*.cpp"  "./client/src/http/src/*.cpp" "./client/src/http_parser/*.cpp" "./client/src/json/src/*.cpp" "./client/src/signal/src/*.cpp" )

list(APPEND SOURCES
       ./client/src/libuv/src/unix/async.cpp
       ./client/src/libuv/src/unix/core.cpp
       ./client/src/libuv/src/unix/dl.cpp
       ./client/src/libuv/src/unix/fs.cpp
       ./client/src/libuv/src/unix/getaddrinfo.cpp
       ./client/src/libuv/src/unix/getnameinfo.cpp
       ./client/src/libuv/src/unix/loop-watcher.cpp
       ./client/src/libuv/src/unix/loop.cpp
       ./client/src/libuv/src/unix/pipe.cpp
       ./client/src/libuv/src/unix/poll.cpp
       ./client/src/libuv/src/unix/process.cpp
       ./client/src/libuv/src/unix/random-devurandom.cpp
       ./client/src/libuv/src/unix/signal.cpp
       ./client/src/libuv/src/unix/stream.cpp
       ./client/src/libuv/src/unix/tcp.cpp
       ./client/src/libuv/src/unix/thread.cpp
       ./client/src/libuv/src/unix/tty.cpp
       ./client/src/libuv/src/unix/udp.cpp)



 if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND uv_defines _GNU_SOURCE _POSIX_C_SOURCE=200112)
  list(APPEND SOURCES
       ./client/src/libuv/src/unix/linux-core.cpp
       ./client/src/libuv/src/unix/linux-inotify.cpp
       ./client/src/libuv/src/unix/linux-syscalls.cpp
       ./client/src/libuv/src/unix/proctitle.cpp
       ./client/src/libuv/src/unix/random-getrandom.cpp
       ./client/src/libuv/src/unix/random-sysctl.cpp)
  endif()


if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|NetBSD|OpenBSD")
  list(APPEND SOURCES ./client/src/libuv/src/unix/bsd-ifaddrs.cpp ./client/src/libuv/src/unix/kqueue.cpp)
endif()

if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "Android|Linux|OS/390")
  list(APPEND SOURCES ./client/src/libuv/src/unix/proctitle.cpp)
endif()


if(APPLE OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
  list(APPEND SOURCES ./client/src/libuv/src/unix/random-getentropy.cpp)
endif()

if(APPLE)
  list(APPEND uv_defines _DARWIN_UNLIMITED_SELECT=1 _DARWIN_USE_64_BIT_INODE=1)
  list(APPEND SOURCES
       ./client/src/libuv/src/unix/darwin-proctitle.cpp
       ./client/src/libuv/src/unix/darwin.cpp
       ./client/src/libuv/src/unix/fsevents.cpp)
endif()


#
#file(GLOB SOURCES "*.cpp")

set (CMAKE_CXX_STANDARD 14)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        socketio

        # Sets the library as a shared library.
        STATIC

        # Provides a relative path to your source file(s).
        ${SOURCES})

target_compile_definitions(socketio
                           PUBLIC ${uv_defines} )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

add_executable(main  main.cpp)

target_link_libraries(main  ${CMAKE_CURRENT_BINARY_DIR}/libsocketio.a)
target_link_libraries(main dl pthread ssl crypto )

#target_link_libraries (testme LINK_PUBLIC ${SOURCES})
