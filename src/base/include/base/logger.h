/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#ifndef base_Logger_H
#define base_Logger_H


#include "base/base.h"
#include "base/error.h"
#include "base/singleton.h"
#include "base/thread.h"

#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <mutex>
#include <string.h>

//#define _REMOTELOG
#if defined(__ANDROID__)    
#include <android/log.h>
#include "net/UdpSocket.h"
#endif

#if defined(_REMOTELOG) 
#include "net/UdpSocket.h"
#endif


namespace base {


enum class Level
{
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Fatal = 5,
    Remote = 6,
};


inline Level getLevelFromString(const char* level)
{
    if (strcmp(level, "trace") == 0)
        return Level::Trace;
    if (strcmp(level, "debug") == 0)
        return Level::Debug;
    if (strcmp(level, "info") == 0)
        return Level::Info;
    if (strcmp(level, "warn") == 0)
        return Level::Warn;
    if (strcmp(level, "error") == 0)
        return Level::Error;
    if (strcmp(level, "fatal") == 0)
        return Level::Fatal;
     if (strcmp(level, "remote") == 0)
        return Level::Remote;
    return Level::Trace;
}


inline const char* getStringFromLevel(Level level)
{
    switch (level) {
        case Level::Trace:
            return "trace";
        case Level::Debug:
            return "debug";
        case Level::Info:
            return "info";
        case Level::Warn:
            return "warn";
        case Level::Error:
            return "error";
        case Level::Fatal:
            return "fatal";
        case Level::Remote:
            return "remote";
    }
    return "debug";
}


struct Base_API LogStream;
class Base_API LogChannel;


//
// Default Log Writer
//


/// Log output stream writer.
class Base_API LogWriter
{
public:
    LogWriter();
    virtual ~LogWriter();

    /// Writes the given log message stream.
    virtual void write(LogStream* stream);
};


//
// Asynchronous Log Writer
//


/// Thread based log output stream writer.
class Base_API AsyncLogWriter : public LogWriter, public Thread
{
public:
    AsyncLogWriter();
    virtual ~AsyncLogWriter();

    /// Queues the given log message stream.
    virtual void write(LogStream* stream) override;

    /// Flushes queued messages.
    void flush();

    /// Writes queued messages asynchronously.
    void run() override;

    /// Clears all queued messages.
    void clear();

protected:
    bool writeNext();

    std::deque<LogStream*> _pending;
    mutable std::mutex _mutex;
};


//
// Logger
//


/// Logger class.
class Base_API Logger
{
public:
    Logger();
    ~Logger();

    /// Returns the default logger singleton.
    /// Logger instances may be created separately as needed.
    static Logger& instance();

    /// Sets the default logger singleton instance.
    static void setInstance(Logger* logger, bool freeExisting = true);

    /// Destroys the default logger singleton instance.
    static void destroy();

    /// Adds the given log channel.
    void add(LogChannel* channel);

    /// Removes the given log channel by name,
    /// and optionally frees the pointer.
    void remove(const std::string& name, bool freePointer = true);

    /// Returns the specified log channel.
    /// Throws an exception if the channel doesn't exist.
    LogChannel* get(const std::string& name, bool whiny = true) const;

    /// Sets the default log to the specified log channel.
    void setDefault(const std::string& name);

    /// Sets the log writer instance.
    void setWriter(LogWriter* writer);

    /// Returns the default log channel, or the nullptr channel
    /// if no default channel has been set.
    LogChannel* getDefault() const;

    /// Writes the given message to the default log channel.
    /// The message will be copied.
    void write(const LogStream& stream);

    /// Writes the given message to the default log channel.
    /// The stream pointer will be deleted when appropriate.
    void write(LogStream* stream);

    /// Sends to the default log using the given class instance.
    /// Recommend using write(LogStream&) to avoid copying data.
    // LogStream& send(const char* level = "debug", const char* realm = "",
    //                 const void* ptr = nullptr,
    //                 const char* channel = nullptr) const;

protected:
    /// NonCopyable and NonMovable
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    typedef std::map<std::string, LogChannel*> LogChannelMap;

    friend class Singleton<Logger>;
    friend class Thread;

    mutable std::mutex _mutex;
    LogChannelMap _channels;
    LogChannel* _defaultChannel;
    LogWriter* _writer;
};


//
// Recursive logging for veradic arguments
//


template <typename T>
void logArgs(std::ostream& o, T&& t)
{
    o << t; // << ' '; // << std::endl;
}

template<typename T, typename... Args>
void logArgs(std::ostream& o, T&& t, Args&&... args) // recursive variadic function
{
    logArgs(o, std::forward<T>(t));
    logArgs(o, std::forward<Args>(args)...);
}


//
// Log Stream
//


#ifdef base_ENABLE_LOGGING

struct LogStream
{
    Level level;
    int line;
    std::string realm;
    std::ostringstream message;
    std::time_t ts;
    LogChannel* channel;
    bool flushed;

    LogStream(Level level, std::string realm, int line, const char* channel = nullptr);
    LogStream(const LogStream& that);
    ~LogStream();

    /// Recursively log veradic arguments.
    template<typename... Args>
    void write(Args... args)
    {
        logArgs(message, args...);
        flush();
    }

    /// Write data to the log message.
    template <typename T> LogStream& operator<<(const T& data)
    {
        message << data;
        return *this;
    }

    LogStream& operator<<(const Level data)
    {
        level = data;
        return *this;
    }

    LogStream& operator<<(LogChannel* data)
    {
        channel = data;
        return *this;
    }

    /// Handle std::endl manipulators.
    /// This method flushes the log message and queues it for write.
    LogStream& operator<<(std::ostream& (*f)(std::ostream&))
    {
        flush();
        return *this;
    }

    void flush()
    {
        if (!flushed) {
            flushed = true;
            message << std::endl;
            Logger::instance().write(new LogStream(*this));
        }
    }
};

#else

struct LogStream
{
    LogStream(Level level, std::string realm, int line, const char* channel = nullptr) {};
    LogStream(const LogStream& that) {}

    template<typename... Args>
    void write(Args... args)
    {
    }

    template <typename T> LogStream& operator<<(const T&)
    {
        return *this;
    }

    LogStream& operator<<(std::ostream& (*f)(std::ostream&))
    {
        return *this;
    }
};

#endif


//
// Log Channel
//


class Base_API LogChannel
{
public:
    LogChannel(std::string name, Level level = Level::Debug,
               std::string timeFormat = "%H:%M:%S");
    virtual ~LogChannel() = default;

    virtual void write(const LogStream& stream);
    virtual void write(std::string message, Level level = Level::Debug,
                       std::string realm = "");
    virtual void format(const LogStream& stream, std::ostream& ost);

    std::string name() const { return _name; }
    Level level() const { return _level; }
    std::string timeFormat() const { return _timeFormat; }

    void setLevel(Level level) { _level = level; }
    void setTimeFormat(std::string format) { _timeFormat = std::move(format); }
    void setFilter(std::string filter) { _filter = std::move(filter); }

protected:
    std::string _name;
    Level _level;
    std::string _timeFormat;
    std::string _filter;
};


/// Null log channel.
///
/// Redifine the base `LogChannel` as `NullChannel` so it can be logically used
/// as a disabled log channel.
typedef LogChannel NullChannel;


//
// Console Channel
//


class Base_API ConsoleChannel : public LogChannel
{
public:
    ConsoleChannel(std::string name, Level level = Level::Debug,
                   std::string timeFormat = "%Y-%m-%d %H:%M:%S");
    virtual ~ConsoleChannel()= default;
    
    virtual void write(const LogStream& stream) override;

};



#if defined(__ANDROID__)   || defined(_REMOTELOG) 

class Base_API RemoteChannel : public ConsoleChannel
{
public:
    RemoteChannel(std::string name, Level level = Level::Debug, std::string ip="127.0.0.1", int port=6000,  
                   std::string timeFormat = "%Y-%m-%d %H:%M:%S");
    virtual ~RemoteChannel();
    
    virtual void write(const LogStream& stream) override;
    
   // net::UdpSocket *udpClient;
};


#endif
//
// File Channel
//


class Base_API FileChannel : public LogChannel
{
public:
    FileChannel(std::string name, std::string path,
                Level level = Level::Debug,
                std::string timeFormat = "%H:%M:%S");
    virtual ~FileChannel();

    virtual void write(const LogStream& stream) override;

    void setPath(const std::string& path);
    std::string path() const;

protected:
    virtual void open();
    virtual void close();

protected:
    std::ofstream _fstream;
    std::string _path;
};


//
// Rotating File Channel
//


class Base_API RotatingFileChannel : public LogChannel
{
public:
    RotatingFileChannel(std::string name,
                        std::string dir,
                        Level level = Level::Debug,
                        std::string extension = "log",
                        int rotationInterval = 12 * 3600, // 12 hours
                        std::string timeFormat = "%H:%M:%S");
    virtual ~RotatingFileChannel();

    virtual void write(const LogStream& stream) override;
    virtual void rotate();

    std::string dir() const { return _dir; }
    std::string filename() const { return _filename; }
    int rotationInterval() const { return _rotationInterval; }

    void setDir(std::string dir) { _dir = std::move(dir); }
    void setExtension(std::string ext) { _extension = std::move(ext); }
    void setRotationInterval(int interval) { _rotationInterval = interval; }

protected:
    std::ofstream* _fstream;
    std::string _dir;
    std::string _filename;
    std::string _extension;
    int _rotationInterval; ///< The log rotation interval in seconds
    time_t _rotatedAt;     ///< The time the log was last rotated
};





constexpr const char* str_end(const char *str)
{
    return *str ? str_end(str + 1) : str;
}

constexpr bool str_slant(const char *str)
{
    return *str == '/' || *str == '\\' ? true : (*str ? str_slant(str + 1) : false);
}

constexpr const char* r_slant(const char* str)
{
    return *str == '/' || *str == '\\' ? (str + 1) : r_slant(str - 1);
}

constexpr const char* _fileName(const char* str)
{
    return str_slant(str) ? r_slant(str_end(str)) : str;
}


//
// Logging macros
//


// Macro for logging class functions
#ifndef __CLASS_FUNCTION__
#if _MSC_VER
#define __CLASS_FUNCTION__ __FUNCTION__
#else
inline std::string _methodName(const std::string& fsig)
{
    size_t colons = fsig.find("::");
    size_t sbeg = fsig.substr(0, colons).rfind(" ") + 1;
    size_t send = fsig.rfind("(") - sbeg;
    return fsig.substr(sbeg, send) + "()";
}
#define __CLASS_FUNCTION__ _methodName(__PRETTY_FUNCTION__)
#endif
#endif


#define STrace LogStream(Level::Trace, _fileName(__FILE__), __LINE__)
#define SDebug LogStream(Level::Debug, _fileName(__FILE__), __LINE__)
#define SInfo  LogStream(Level::Info, _fileName(__FILE__), __LINE__)
#define SWarn  LogStream(Level::Warn, _fileName(__FILE__), __LINE__)
#define SError LogStream(Level::Error, _fileName(__FILE__), __LINE__)
#define SRTrace LogStream(Level::Remote, _fileName(__FILE__), __LINE__)

#define LTrace(...) { LogStream(Level::Trace, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }
#define LDebug(...) { LogStream(Level::Debug, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }
#define LInfo(...)  { LogStream(Level::Info, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }
#define LWarn(...)  { LogStream(Level::Warn, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }
#define LError(...) { LogStream(Level::Error, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }
#define RTrace(...) { LogStream(Level::Remote, _fileName(__FILE__), __LINE__).write(__VA_ARGS__); }


// #define TraceS(self) LogStream(Level::Trace, _fileName(__FILE__), __LINE__, self)
// #define DebugS(self) LogStream(Level::Debug, _fileName(__FILE__), __LINE__, self)
// #define InfoS(self)  LogStream(Level::Info, _fileName(__FILE__), __LINE__, self)
// #define WarnS(self)  LogStream(Level::Warn, _fileName(__FILE__), __LINE__, self)
// #define ErrorS(self) LogStream(Level::Error, _fileName(__FILE__), __LINE__, self)

// #define TraceN(self) LogStream(Level::Trace, self->className(), __LINE__, self)
// #define DebugN(self) LogStream(Level::Debug, self->className(), __LINE__, self)
// #define InfoN(self)  LogStream(Level::Info, self->className(), __LINE__, self)
// #define WarnN(self)  LogStream(Level::Warn, self->className(), __LINE__, self)
// #define ErrorN(self) LogStream(Level::Error, self->className(), __LINE__, self)


} // namespace base


#endif


