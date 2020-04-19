/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef base_Time_H
#define base_Time_H


#include "base/base.h"
#include <string>
#include <ctime>
#include <cstdint>


namespace base {


/// Classes and functions for handling time
namespace time {


/// Constants for calculating time.
static const int64_t kNumMillisecsPerSec = INT64_C(1000);
static const int64_t kNumMicrosecsPerSec = INT64_C(1000000);
static const int64_t kNumNanosecsPerSec = INT64_C(1000000000);

static const int64_t kNumMicrosecsPerMillisec = kNumMicrosecsPerSec / kNumMillisecsPerSec;
static const int64_t kNumNanosecsPerMillisec = kNumNanosecsPerSec / kNumMillisecsPerSec;
static const int64_t kNumNanosecsPerMicrosec = kNumNanosecsPerSec / kNumMicrosecsPerSec;

/// The date/time format defined in the ISO 8601 standard.
/// This is the default format used throughout the library for consistency.
///
/// Examples:
///   2005-01-01T12:00:00+01:00
///   2005-01-01T11:00:00Z
static const char* ISO8601Format = "%Y-%m-%dT%H:%M:%SZ";

/// Return the number of UTC milliseconds since epoch.
Base_API std::time_t now();

/// Return the current process time in decimal seconds.
Base_API double clockSecs();

/// Cross-platform time formatting.
Base_API std::string print(const std::tm& dt, const char* fmt = ISO8601Format);

/// Print the current local time using the given format.
Base_API std::string printLocal(const char* fmt = ISO8601Format);

/// Print the current UTC time using the given format.
Base_API std::string printUTC(const char* fmt = ISO8601Format);

/// Convert the given time value to local time.
/// Uses thread-safe native functions.
Base_API std::tm toLocal(const std::time_t& time);

/// Convert the given time value to UTC time.
/// Uses thread-safe native functions.
Base_API std::tm toUTC(const std::time_t& time);

/// Return a local ISO8601 formatted date time string.
Base_API std::string getLocal();

/// Return a UTC ISO8601 formatted date time string.
Base_API std::string getUTC();

/// Return the current high-resolution real time in nanoseconds.
Base_API uint64_t hrtime();

#if 0
/// Retrieves the number of milliseconds that have elapsed since the system was started, up to 49.7 days.
uint64_t ticks();

/// Return the current real time in milliseconds.
uint64_t getTimeMS();
#endif


} // namespace time
} // namespace base


#endif // base_Time_H

