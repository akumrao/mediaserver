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
    src/fs-poll.cpp
    src/idna.cpp
    src/inet.cpp
    src/random.cpp
    src/strscpy.cpp
    src/threadpool.cpp
    src/timer.cpp
    src/uv-common.cpp
    src/uv-data-getter-setters.cpp
    src/version.cpp)

set(uv_test_sources
    test/blackhole-server.cpp
    test/echo-server.cpp
    test/run-tests.cpp
    test/runner.cpp
    test/test-active.cpp
    test/test-async-null-cb.cpp
    test/test-async.cpp
    test/test-barrier.cpp
    test/test-callback-order.cpp
    test/test-callback-stack.cpp
    test/test-close-fd.cpp
    test/test-close-order.cpp
    test/test-condvar.cpp
    test/test-connect-unspecified.cpp
    test/test-connection-fail.cpp
    test/test-cwd-and-chdir.cpp
    test/test-default-loop-close.cpp
    test/test-delayed-accept.cpp
    test/test-dlerror.cpp
    test/test-eintr-handling.cpp
    test/test-embed.cpp
    test/test-emfile.cpp
    test/test-env-vars.cpp
    test/test-error.cpp
    test/test-fail-always.cpp
    test/test-fork.cpp
    test/test-fs-copyfile.cpp
    test/test-fs-event.cpp
    test/test-fs-poll.cpp
    test/test-fs.cpp
    test/test-fs-readdir.cpp
    test/test-fs-fd-hash.cpp
    test/test-fs-open-flags.cpp
    test/test-get-currentexe.cpp
    test/test-get-loadavg.cpp
    test/test-get-memory.cpp
    test/test-get-passwd.cpp
    test/test-getaddrinfo.cpp
    test/test-gethostname.cpp
    test/test-getnameinfo.cpp
    test/test-getsockname.cpp
    test/test-getters-setters.cpp
    test/test-gettimeofday.cpp
    test/test-handle-fileno.cpp
    test/test-homedir.cpp
    test/test-hrtime.cpp
    test/test-idle.cpp
    test/test-idna.cpp
    test/test-ip4-addr.cpp
    test/test-ip6-addr.cpp
    test/test-ipc-heavy-traffic-deadlock-bug.cpp
    test/test-ipc-send-recv.cpp
    test/test-ipc.cpp
    test/test-loop-alive.cpp
    test/test-loop-close.cpp
    test/test-loop-configure.cpp
    test/test-loop-handles.cpp
    test/test-loop-stop.cpp
    test/test-loop-time.cpp
    test/test-multiple-listen.cpp
    test/test-mutexes.cpp
    test/test-osx-select.cpp
    test/test-pass-always.cpp
    test/test-ping-pong.cpp
    test/test-pipe-bind-error.cpp
    test/test-pipe-close-stdout-read-stdin.cpp
    test/test-pipe-connect-error.cpp
    test/test-pipe-connect-multiple.cpp
    test/test-pipe-connect-prepare.cpp
    test/test-pipe-getsockname.cpp
    test/test-pipe-pending-instances.cpp
    test/test-pipe-sendmsg.cpp
    test/test-pipe-server-close.cpp
    test/test-pipe-set-fchmod.cpp
    test/test-pipe-set-non-blocking.cpp
    test/test-platform-output.cpp
    test/test-poll-close-doesnt-corrupt-stack.cpp
    test/test-poll-close.cpp
    test/test-poll-closesocket.cpp
    test/test-poll-oob.cpp
    test/test-poll.cpp
    test/test-process-priority.cpp
    test/test-process-title-threadsafe.cpp
    test/test-process-title.cpp
    test/test-queue-foreach-delete.cpp
    test/test-random.cpp
    test/test-ref.cpp
    test/test-run-nowait.cpp
    test/test-run-once.cpp
    test/test-semaphore.cpp
    test/test-shutdown-close.cpp
    test/test-shutdown-eof.cpp
    test/test-shutdown-twice.cpp
    test/test-signal-multiple-loops.cpp
    test/test-signal-pending-on-close.cpp
    test/test-signal.cpp
    test/test-socket-buffer-size.cpp
    test/test-spawn.cpp
    test/test-stdio-over-pipes.cpp
    test/test-strscpy.cpp
    test/test-tcp-alloc-cb-fail.cpp
    test/test-tcp-bind-error.cpp
    test/test-tcp-bind6-error.cpp
    test/test-tcp-close-accept.cpp
    test/test-tcp-close-while-connecting.cpp
    test/test-tcp-close.cpp
    test/test-tcp-close-reset.cpp
    test/test-tcp-connect-error-after-write.cpp
    test/test-tcp-connect-error.cpp
    test/test-tcp-connect-timeout.cpp
    test/test-tcp-connect6-error.cpp
    test/test-tcp-create-socket-early.cpp
    test/test-tcp-flags.cpp
    test/test-tcp-oob.cpp
    test/test-tcp-open.cpp
    test/test-tcp-read-stop.cpp
    test/test-tcp-shutdown-after-write.cpp
    test/test-tcp-try-write.cpp
    test/test-tcp-try-write-error.cpp
    test/test-tcp-unexpected-read.cpp
    test/test-tcp-write-after-connect.cpp
    test/test-tcp-write-fail.cpp
    test/test-tcp-write-queue-order.cpp
    test/test-tcp-write-to-half-open-connection.cpp
    test/test-tcp-writealot.cpp
    test/test-thread-equal.cpp
    test/test-thread.cpp
    test/test-threadpool-cancel.cpp
    test/test-threadpool.cpp
    test/test-timer-again.cpp
    test/test-timer-from-check.cpp
    test/test-timer.cpp
    test/test-tmpdir.cpp
    test/test-tty-duplicate-key.cpp
    test/test-tty.cpp
    test/test-udp-alloc-cb-fail.cpp
    test/test-udp-bind.cpp
    test/test-udp-connect.cpp
    test/test-udp-create-socket-early.cpp
    test/test-udp-dgram-too-big.cpp
    test/test-udp-ipv6.cpp
    test/test-udp-multicast-interface.cpp
    test/test-udp-multicast-interface6.cpp
    test/test-udp-multicast-join.cpp
    test/test-udp-multicast-join6.cpp
    test/test-udp-multicast-ttl.cpp
    test/test-udp-open.cpp
    test/test-udp-options.cpp
    test/test-udp-send-and-recv.cpp
    test/test-udp-send-hang-loop.cpp
    test/test-udp-send-immediate.cpp
    test/test-udp-send-unreachable.cpp
    test/test-udp-try-send.cpp
    test/test-uname.cpp
    test/test-walk-handles.cpp
    test/test-watcher-cross-stop.cpp)

if(WIN32)
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
       src/win/async.cpp
       src/win/core.cpp
       src/win/detect-wakeup.cpp
       src/win/dl.cpp
       src/win/error.cpp
       src/win/fs.cpp
       src/win/fs-event.cpp
       src/win/getaddrinfo.cpp
       src/win/getnameinfo.cpp
       src/win/handle.cpp
       src/win/loop-watcher.cpp
       src/win/pipe.cpp
       src/win/thread.cpp
       src/win/poll.cpp
       src/win/process.cpp
       src/win/process-stdio.cpp
       src/win/signal.cpp
       src/win/snprintf.cpp
       src/win/stream.cpp
       src/win/tcp.cpp
       src/win/tty.cpp
       src/win/udp.cpp
       src/win/util.cpp
       src/win/winapi.cpp
       src/win/winsock.cpp)
  list(APPEND uv_test_libraries ws2_32)
  list(APPEND uv_test_sources src/win/snprintf.cpp test/runner-win.cpp)
else()
  list(APPEND uv_defines _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE)
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
    # Android has pthread as part of its c library, not as a separate
    # libpthread.so.
    list(APPEND uv_libraries pthread)
  endif()
  list(APPEND uv_sources
       src/unix/async.cpp
       src/unix/core.cpp
       src/unix/dl.cpp
       src/unix/fs.cpp
       src/unix/getaddrinfo.cpp
       src/unix/getnameinfo.cpp
       src/unix/loop-watcher.cpp
       src/unix/loop.cpp
       src/unix/pipe.cpp
       src/unix/poll.cpp
       src/unix/process.cpp
       src/unix/random-devurandom.cpp
       src/unix/signal.cpp
       src/unix/stream.cpp
       src/unix/tcp.cpp
       src/unix/thread.cpp
       src/unix/tty.cpp
       src/unix/udp.cpp)
  list(APPEND uv_test_sources test/runner-unix.cpp)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
  list(APPEND uv_defines
       _ALL_SOURCE
       _LINUX_SOURCE_COMPAT
       _THREAD_SAFE
       _XOPEN_SOURCE=500)
  list(APPEND uv_libraries perfstat)
  list(APPEND uv_sources src/unix/aix.cpp)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  list(APPEND uv_libs dl)
  list(APPEND uv_sources
       src/unix/android-ifaddrs.cpp
       src/unix/linux-core.cpp
       src/unix/linux-inotify.cpp
       src/unix/linux-syscalls.cpp
       #src/unix/procfs-exepath.cpp
       src/unix/pthread-fixes.cpp
       #src/unix/sysinfo-loadavg.cpp
)
endif()

if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "Android|Linux|OS/390")
  list(APPEND uv_sources src/unix/proctitle.cpp)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD")
  list(APPEND uv_sources src/unix/freebsd.cpp)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|NetBSD|OpenBSD")
  list(APPEND uv_sources src/unix/posix-hrtime.cpp src/unix/bsd-proctitle.cpp)
  list(APPEND uv_libraries kvm)
endif()

if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|NetBSD|OpenBSD")
  list(APPEND uv_sources src/unix/bsd-ifaddrs.cpp src/unix/kqueue.cpp)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  list(APPEND uv_sources src/unix/random-getrandom.cpp)
endif()

if(APPLE OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
  list(APPEND uv_sources src/unix/random-getentropy.cpp)
endif()

if(APPLE)
  list(APPEND uv_defines _DARWIN_UNLIMITED_SELECT=1 _DARWIN_USE_64_BIT_INODE=1)
  list(APPEND uv_sources
       src/unix/darwin-proctitle.cpp
       src/unix/darwin.cpp
       src/unix/fsevents.cpp)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND uv_defines _GNU_SOURCE _POSIX_C_SOURCE=200112)
  list(APPEND uv_libraries dl rt)
  list(APPEND uv_sources
       src/unix/linux-core.cpp
       src/unix/linux-inotify.cpp
       src/unix/linux-syscalls.cpp
       #src/unix/procfs-exepath.cpp
       src/unix/random-getrandom.cpp
       src/unix/random-sysctl.cpp
       #src/unix/sysinfo-loadavg.cpp
)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
  list(APPEND uv_sources src/unix/netbsd.cpp)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
  list(APPEND uv_sources src/unix/openbsd.cpp)
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
       src/unix/pthread-fixes.cpp
       src/unix/pthread-barrier.cpp
       src/unix/os390.cpp
       src/unix/os390-syscalls.cpp)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  list(APPEND uv_defines __EXTENSIONS__ _XOPEN_SOURCE=500)
  list(APPEND uv_libraries kstat nsl sendfile socket)
  list(APPEND uv_sources src/unix/no-proctitle.cpp src/unix/sunos.cpp)
endif()

if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|Linux|NetBSD|OpenBSD")
  list(APPEND uv_test_libraries util)
endif()

add_library(uv SHARED ${uv_sources})
target_compile_definitions(uv
                           INTERFACE USING_UV_SHARED=1
                           PRIVATE ${uv_defines} BUILDING_UV_SHARED=1)
target_compile_options(uv PRIVATE ${uv_cflags})
target_include_directories(uv PUBLIC include PRIVATE src)
target_link_libraries(uv ${uv_libraries})

add_library(uv_a STATIC ${uv_sources})
target_compile_definitions(uv_a PRIVATE ${uv_defines})
target_compile_options(uv_a PRIVATE ${uv_cflags})
target_include_directories(uv_a PUBLIC include PRIVATE src)
target_link_libraries(uv_a ${uv_libraries})



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
