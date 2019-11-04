// Name of library
#define base_NAME "mediaserver"

// Define the library version
#define base_VERSION "1.1.4"
#define base_VERSION_MAJOR 1
#define base_VERSION_MINOR 1
#define base_VERSION_PATCH 4

// Define the source path
#define base_SOURCE_DIR "/data/mozilla/mediaserver/src"

// Define the build path
#define base_BUILD_DIR "/data/mozilla/mediaserver/build"

// Define the installation path
#define base_INSTALL_DIR "/usr/local"

// Building as shared library (.so or .dll)
#define base_SHARED_LIBRARY

// Disable logging
#define base_ENABLE_LOGGING

// Attempt to recover from internal exceptions
// Exceptions thrown inside thread and stream context will be caught, logged and
// handled via the event loop in an attempt to prevent crashes.
#define base_EXCEPTION_RECOVERY



// LibUV library
#define HAVE_LIBUV

// OpenSSL library
#define HAVE_OPENSSL
/* #undef OPENSSL_IS_BORINGSSL */

// FFmpeg video library
#define HAVE_FFMPEG
#define HAVE_FFMPEG_AVCODEC
#define HAVE_FFMPEG_AVFORMAT
#define HAVE_FFMPEG_AVUTIL
#define HAVE_FFMPEG_AVFILTER
#define HAVE_FFMPEG_AVDEVICE
#define HAVE_FFMPEG_AVRESAMPLE
#define HAVE_FFMPEG_SWRESAMPLE
#define HAVE_FFMPEG_SWSCALE
#define HAVE_FFMPEG_POSTPROC

// OpenCV library
/* #undef HAVE_OPENCV */

// RtAudio library
/* #undef HAVE_RTAUDIO */

// Define to 1 if you have the <inttypes.h> header file.
/* #undef HAVE_INTTYPES_H */

// Enable macros for format specifiers in <inttypes.h>
#define __STDC_FORMAT_MACROS

// Define to 1 if your processor stores words with the most significant byte
// first (like Motorola and SPARC, unlike Intel and VAX).
/* #undef WORDS_BIGENDIAN */


//
/// Platform and compiler definitions
//

#ifdef _WIN32 // Windows (x64 and x86)
#define base_WIN
#endif
#if __unix__ // Unix
#define base_UNIX
#endif
#if __posix__ // POSIX
#define base_POSIX
#endif
#if __linux__ // Linux
#define base_LINUX
#endif
#if __APPLE__ // Mac OS
#define base_APPLE
#endif
#if __GNUC__ // GCC compiler
#define base_GNUC
#endif
#if defined(__MINGW32__) || defined(__MINGW64__) // MinGW
#define base_MINGW
#endif


//
/// Windows specific
//

#ifdef base_WIN

// Verify that we're building with the multithreaded
// versions of the runtime libraries
#if defined(_MSC_VER) && !defined(_MT)
#error Must compile with /MD, /MDd, /MT or /MTd
#endif

// Check debug/release settings consistency
#if defined(NDEBUG) && defined(_DEBUG)
#error Inconsistent build settings (check for /MD[d])
#endif

// Unicode Support
#if defined(UNICODE)
#define base_UNICODE
#endif

// Disable unnecessary warnings
#if defined(_MSC_VER)
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4251) // ... needs to have dll-interface warning
#pragma warning(disable : 4355) // 'this' : used in base member initializer list
#pragma warning(disable : 4996) // VC++ 8.0 deprecation warnings
#pragma warning(disable : 4351) // new behavior: elements of array '...' will be default initialized
#pragma warning(disable : 4675) // resolved overload was found by argument-dependent lookup
#pragma warning(disable : 4100) // MSVS 'unreferenced formal parameter' warnings showing false positives
#pragma warning(disable : 4706) // assignment within conditional expression
#endif

#endif // base_WIN
