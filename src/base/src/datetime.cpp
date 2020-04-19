/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#include "base/datetime.h"
//#include "base/numeric.h"
#include "base/platform.h"
#include "base/time.h"
#include "base/util.h"

#include <cctype>
#include <cmath>
#include <ctime>
#if defined(_WIN32_WCE)
#include "wce_time.h"
#endif
#if !defined(WIN32)
#include <sys/time.h>
#endif


using std::endl;


namespace base {


//
// Date Time
//


inline double DateTime::toJulianDay(Timestamp::UtcTimeVal utcTime)
{
    double utcDays = double(utcTime) / 864000000000.0;
    return utcDays + 2299160.5; // first day of Gregorian reform (Oct 15 1582)
}


inline Timestamp::UtcTimeVal DateTime::toUtcTime(double julianDay)
{
    return Timestamp::UtcTimeVal((julianDay - 2299160.5) * 864000000000.0);
}


DateTime::DateTime()
{
    Timestamp now;
    _utcTime = now.utcTime();
    computeGregorian(julianDay());
    computeDaytime();
}


DateTime::DateTime(const Timestamp& timestamp)
    : _utcTime(timestamp.utcTime())
{
    computeGregorian(julianDay());
    computeDaytime();
}


DateTime::DateTime(int year, int month, int day, int hour, int minute,
                   int second, int millisecond, int microsecond)
    : _year(year)
    , _month(month)
    , _day(day)
    , _hour(hour)
    , _minute(minute)
    , _second(second)
    , _millisecond(millisecond)
    , _microsecond(microsecond)
{
    assert(year >= 0 && year <= 9999);
    assert(month >= 1 && month <= 12);
    assert(day >= 1 && day <= daysOfMonth(year, month));
    assert(hour >= 0 && hour <= 23);
    assert(minute >= 0 && minute <= 59);
    assert(second >= 0 && second <= 59);
    assert(millisecond >= 0 && millisecond <= 999);
    assert(microsecond >= 0 && microsecond <= 999);

    _utcTime = toUtcTime(toJulianDay(year, month, day)) +
               10 * (hour * Timespan::HOURS + minute * Timespan::MINUTES +
                     second * Timespan::SECONDS +
                     millisecond * Timespan::MILLISECONDS + microsecond);
}


DateTime::DateTime(double julianDay)
    : _utcTime(toUtcTime(julianDay))
{
    computeGregorian(julianDay);
}


DateTime::DateTime(Timestamp::UtcTimeVal utcTime, Timestamp::TimeDiff diff)
    : _utcTime(utcTime + diff * 10)
{
    computeGregorian(julianDay());
    computeDaytime();
}


DateTime::DateTime(const DateTime& dateTime)
    : _utcTime(dateTime._utcTime)
    , _year(dateTime._year)
    , _month(dateTime._month)
    , _day(dateTime._day)
    , _hour(dateTime._hour)
    , _minute(dateTime._minute)
    , _second(dateTime._second)
    , _millisecond(dateTime._millisecond)
    , _microsecond(dateTime._microsecond)
{
}


DateTime::~DateTime()
{
}


DateTime& DateTime::operator=(const DateTime& dateTime)
{
    if (&dateTime != this) {
        _utcTime = dateTime._utcTime;
        _year = dateTime._year;
        _month = dateTime._month;
        _day = dateTime._day;
        _hour = dateTime._hour;
        _minute = dateTime._minute;
        _second = dateTime._second;
        _millisecond = dateTime._millisecond;
        _microsecond = dateTime._microsecond;
    }
    return *this;
}


DateTime& DateTime::operator=(const Timestamp& timestamp)
{
    _utcTime = timestamp.utcTime();
    computeGregorian(julianDay());
    computeDaytime();
    return *this;
}


DateTime& DateTime::operator=(double julianDay)
{
    _utcTime = toUtcTime(julianDay);
    computeGregorian(julianDay);
    return *this;
}


DateTime& DateTime::assign(int year, int month, int day, int hour, int minute,
                           int second, int millisecond, int microsecond)
{
    assert(year >= 0 && year <= 9999);
    assert(month >= 1 && month <= 12);
    assert(day >= 1 && day <= daysOfMonth(year, month));
    assert(hour >= 0 && hour <= 23);
    assert(minute >= 0 && minute <= 59);
    assert(second >= 0 && second <= 59);
    assert(millisecond >= 0 && millisecond <= 999);
    assert(microsecond >= 0 && microsecond <= 999);

    _utcTime = toUtcTime(toJulianDay(year, month, day)) +
               10 * (hour * Timespan::HOURS + minute * Timespan::MINUTES +
                     second * Timespan::SECONDS +
                     millisecond * Timespan::MILLISECONDS + microsecond);
    _year = year;
    _month = month;
    _day = day;
    _hour = hour;
    _minute = minute;
    _second = second;
    _millisecond = millisecond;
    _microsecond = microsecond;

    return *this;
}


void DateTime::swap(DateTime& dateTime)
{
    std::swap(_utcTime, dateTime._utcTime);
    std::swap(_year, dateTime._year);
    std::swap(_month, dateTime._month);
    std::swap(_day, dateTime._day);
    std::swap(_hour, dateTime._hour);
    std::swap(_minute, dateTime._minute);
    std::swap(_second, dateTime._second);
    std::swap(_millisecond, dateTime._millisecond);
    std::swap(_microsecond, dateTime._microsecond);
}


int DateTime::dayOfWeek() const
{
    return int((std::floor(julianDay() + 1.5))) % 7;
}


int DateTime::dayOfYear() const
{
    int doy = 0;
    for (int month = 1; month < _month; ++month)
        doy += daysOfMonth(_year, month);
    doy += _day;
    return doy;
}


int DateTime::daysOfMonth(int year, int month)
{
    assert(month >= 1 && month <= 12);

    static int daysOfMonthTable[] = {0,  31, 28, 31, 30, 31, 30,
                                     31, 31, 30, 31, 30, 31};

    if (month == 2 && isLeapYear(year))
        return 29;
    else
        return daysOfMonthTable[month];
}


bool DateTime::isValid(int year, int month, int day, int hour, int minute,
                       int second, int millisecond, int microsecond)
{
    return (year >= 0 && year <= 9999) && (month >= 1 && month <= 12) &&
           (day >= 1 && day <= daysOfMonth(year, month)) &&
           (hour >= 0 && hour <= 23) && (minute >= 0 && minute <= 59) &&
           (second >= 0 && second <= 59) &&
           (millisecond >= 0 && millisecond <= 999) &&
           (microsecond >= 0 && microsecond <= 999);
}


int DateTime::week(int firstDayOfWeek) const
{
    assert(firstDayOfWeek >= 0 && firstDayOfWeek <= 6);

    // find the first firstDayOfWeek.
    int baseDay = 1;
    while (DateTime(_year, 1, baseDay).dayOfWeek() != firstDayOfWeek)
        ++baseDay;

    int doy = dayOfYear();
    int offs = baseDay <= 4 ? 0 : 1;
    if (doy < baseDay)
        return offs;
    else
        return (doy - baseDay) / 7 + 1 + offs;
}


double DateTime::julianDay() const
{
    return toJulianDay(_utcTime);
}


DateTime DateTime::operator+(const Timespan& span) const
{
    return DateTime(_utcTime, span.totalMicroseconds());
}


DateTime DateTime::operator-(const Timespan& span) const
{
    return DateTime(_utcTime, -span.totalMicroseconds());
}


Timespan DateTime::operator-(const DateTime& dateTime) const
{
    return Timespan((_utcTime - dateTime._utcTime) / 10);
}


DateTime& DateTime::operator+=(const Timespan& span)
{
    _utcTime += span.totalMicroseconds() * 10;
    computeGregorian(julianDay());
    computeDaytime();
    return *this;
}


DateTime& DateTime::operator-=(const Timespan& span)
{
    _utcTime -= span.totalMicroseconds() * 10;
    computeGregorian(julianDay());
    computeDaytime();
    return *this;
}


void DateTime::makeUTC(int tzd)
{
    operator-=(Timespan(((Timestamp::TimeDiff)tzd) * Timespan::SECONDS));
}


void DateTime::makeLocal(int tzd)
{
    operator+=(Timespan(((Timestamp::TimeDiff)tzd) * Timespan::SECONDS));
}


double DateTime::toJulianDay(int year, int month, int day, int hour, int minute,
                             int second, int millisecond, int microsecond)
{
    // lookup table for (153*month - 457)/5 - note that 3 <= month <= 14.
    static int lookup[] = {-91, -60, -30, 0,   31,  61,  92, 122,
                           153, 184, 214, 245, 275, 306, 337};

    // day to double
    double dday = double(day) +
        ((double((hour * 60 + minute) * 60 + second) * 1000 + millisecond) *
             1000 + microsecond) / 86400000000.0;
    if (month < 3) {
        month += 12;
        --year;
    }
    double dyear = double(year);
    return dday + lookup[month] + 365 * year + std::floor(dyear / 4) -
           std::floor(dyear / 100) + std::floor(dyear / 400) + 1721118.5;
}


void DateTime::checkLimit(short& lower, short& higher, short limit)
{
    if (lower >= limit) {
        higher += short(lower / limit);
        lower = short(lower % limit);
    }
}


void DateTime::normalize()
{
    checkLimit(_microsecond, _millisecond, 1000);
    checkLimit(_millisecond, _second, 1000);
    checkLimit(_second, _minute, 60);
    checkLimit(_minute, _hour, 60);
    checkLimit(_hour, _day, 24);

    if (_day > daysOfMonth(_year, _month)) {
        _day -= daysOfMonth(_year, _month);
        if (++_month > 12) {
            ++_year;
            _month -= 12;
        }
    }
}


void DateTime::computeGregorian(double julianDay)
{
    double z = std::floor(julianDay - 1721118.5);
    double r = julianDay - 1721118.5 - z;
    double g = z - 0.25;
    double a = std::floor(g / 36524.25);
    double b = a - std::floor(a / 4);
    _year = short(std::floor((b + g) / 365.25));
    double c = b + z - std::floor(365.25 * _year);
    _month = short(std::floor((5 * c + 456) / 153));
    double dday = c - std::floor((153.0 * _month - 457) / 5) + r;
    _day = short(dday);
    if (_month > 12) {
        ++_year;
        _month -= 12;
    }
    r *= 24;
    _hour = short(std::floor(r));
    r -= std::floor(r);
    r *= 60;
    _minute = short(std::floor(r));
    r -= std::floor(r);
    r *= 60;
    _second = short(std::floor(r));
    r -= std::floor(r);
    r *= 1000;
    _millisecond = short(std::floor(r));
    r -= std::floor(r);
    r *= 1000;
    _microsecond = short(r + 0.5);

    normalize();

    assert(_month >= 1 && _month <= 12);
    assert(_day >= 1 && _day <= daysOfMonth(_year, _month));
    assert(_hour >= 0 && _hour <= 23);
    assert(_minute >= 0 && _minute <= 59);
    assert(_second >= 0 && _second <= 59);
    assert(_millisecond >= 0 && _millisecond <= 999);
    assert(_microsecond >= 0 && _microsecond <= 999);
}


void DateTime::computeDaytime()
{
    Timespan span(_utcTime / 10);

    // Due to double rounding issues, the previous call to computeGregorian()
    // may have crossed into the next or previous day. We need to correct that.
    // _hour = span.hours();
    int hour = span.hours();
    if (hour == 23 && _hour == 0) {
        _day--;
        if (_day == 0) {
            _month--;
            if (_month == 0) {
                _month = 12;
                _year--;
            }
            _day = daysOfMonth(_year, _month);
        }
    }
    else if (hour == 0 && _hour == 23) {
        _day++;
        if (_day > daysOfMonth(_year, _month)) {
            _month++;
            if (_month > 12) {
                _month = 1;
                _year++;
            }
            _day = 1;
        }
    }
    _hour = hour;
    _minute = span.minutes();
    _second = span.seconds();
    _millisecond = span.milliseconds();
    _microsecond = span.microseconds();
}


//
// LocalDateTime
//


LocalDateTime::LocalDateTime()
{
    determineTzd(true);
}


LocalDateTime::LocalDateTime(int year, int month, int day, int hour, int minute,
                             int second, int millisecond, int microsecond)
    : _dateTime(year, month, day, hour, minute, second, millisecond, microsecond)
{
    determineTzd();
}


LocalDateTime::LocalDateTime(int tzd, int year, int month, int day, int hour,
                             int minute, int second, int millisecond, int microsecond)
    : _dateTime(year, month, day, hour, minute, second, millisecond, microsecond)
    , _tzd(tzd)
{
}


LocalDateTime::LocalDateTime(double julianDay)
    : _dateTime(julianDay)
{
    determineTzd(true);
}


LocalDateTime::LocalDateTime(int tzd, double julianDay)
    : _dateTime(julianDay)
    , _tzd(tzd)
{
    adjustForTzd();
}


LocalDateTime::LocalDateTime(const DateTime& dateTime)
    : _dateTime(dateTime)
{
    determineTzd(true);
}


LocalDateTime::LocalDateTime(int tzd, const DateTime& dateTime)
    : _dateTime(dateTime)
    , _tzd(tzd)
{
    adjustForTzd();
}


LocalDateTime::LocalDateTime(int tzd, const DateTime& dateTime, bool adjust)
    : _dateTime(dateTime)
    , _tzd(tzd)
{
    if (adjust)
        adjustForTzd();
}


LocalDateTime::LocalDateTime(const LocalDateTime& dateTime)
    : _dateTime(dateTime._dateTime)
    , _tzd(dateTime._tzd)
{
}


LocalDateTime::LocalDateTime(Timestamp::UtcTimeVal utcTime,
                             Timestamp::TimeDiff diff, int tzd)
    : _dateTime(utcTime, diff)
    , _tzd(tzd)
{
    adjustForTzd();
}


LocalDateTime::~LocalDateTime()
{
}


LocalDateTime& LocalDateTime::operator=(const LocalDateTime& dateTime)
{
    if (&dateTime != this) {
        _dateTime = dateTime._dateTime;
        _tzd = dateTime._tzd;
    }
    return *this;
}


LocalDateTime& LocalDateTime::operator=(const Timestamp& timestamp)
{
    if (timestamp != this->timestamp()) {
        _dateTime = timestamp;
        determineTzd(true);
    }
    return *this;
}


LocalDateTime& LocalDateTime::operator=(double julianDay)
{
    _dateTime = julianDay;
    determineTzd(true);
    return *this;
}


LocalDateTime& LocalDateTime::assign(int year, int month, int day, int hour,
                                     int minute, int second, int millisecond,
                                     int microseconds)
{
    _dateTime.assign(year, month, day, hour, minute, second, millisecond, microseconds);
    determineTzd(false);
    return *this;
}


LocalDateTime& LocalDateTime::assign(int tzd, int year, int month, int day,
                                     int hour, int minute, int second,
                                     int millisecond, int microseconds)
{
    _dateTime.assign(year, month, day, hour, minute, second, millisecond, microseconds);
    _tzd = tzd;
    return *this;
}


LocalDateTime& LocalDateTime::assign(int tzd, double julianDay)
{
    _tzd = tzd;
    _dateTime = julianDay;
    adjustForTzd();
    return *this;
}


void LocalDateTime::swap(LocalDateTime& dateTime)
{
    _dateTime.swap(dateTime._dateTime);
    std::swap(_tzd, dateTime._tzd);
}


DateTime LocalDateTime::utc() const
{
    return DateTime(_dateTime.utcTime(),
                    -((Timestamp::TimeDiff)_tzd) * Timespan::SECONDS);
}


bool LocalDateTime::operator==(const LocalDateTime& dateTime) const
{
    return utcTime() == dateTime.utcTime();
}


bool LocalDateTime::operator!=(const LocalDateTime& dateTime) const
{
    return utcTime() != dateTime.utcTime();
}


bool LocalDateTime::operator<(const LocalDateTime& dateTime) const
{
    return utcTime() < dateTime.utcTime();
}


bool LocalDateTime::operator<=(const LocalDateTime& dateTime) const
{
    return utcTime() <= dateTime.utcTime();
}


bool LocalDateTime::operator>(const LocalDateTime& dateTime) const
{
    return utcTime() > dateTime.utcTime();
}


bool LocalDateTime::operator>=(const LocalDateTime& dateTime) const
{
    return utcTime() >= dateTime.utcTime();
}


LocalDateTime LocalDateTime::operator+(const Timespan& span) const
{
    // First calculate the adjusted UTC time, then calculate the
    // locally adjusted time by constructing a new LocalDateTime.
    DateTime tmp(utcTime(), span.totalMicroseconds());
    return LocalDateTime(tmp);
}


LocalDateTime LocalDateTime::operator-(const Timespan& span) const
{
    // First calculate the adjusted UTC time, then calculate the
    // locally adjusted time by constructing a new LocalDateTime.
    DateTime tmp(utcTime(), -span.totalMicroseconds());
    return LocalDateTime(tmp);
}


Timespan LocalDateTime::operator-(const LocalDateTime& dateTime) const
{
    return Timespan((utcTime() - dateTime.utcTime()) / 10);
}


LocalDateTime& LocalDateTime::operator+=(const Timespan& span)
{
    // Use the same trick as in operator+. Create a UTC time, adjust
    // it for the span, and convert back to LocalDateTime. This will
    // recalculate the tzd correctly in the case where the addition
    // crosses a DST boundary.
    *this = DateTime(utcTime(), span.totalMicroseconds());
    return *this;
}


LocalDateTime& LocalDateTime::operator-=(const Timespan& span)
{
    // Use the same trick as in operator-. Create a UTC time, adjust
    // it for the span, and convert back to LocalDateTime. This will
    // recalculate the tzd correctly in the case where the subtraction
    // crosses a DST boundary.
    *this = DateTime(utcTime(), -span.totalMicroseconds());
    return *this;
}


void LocalDateTime::determineTzd(bool adjust)
{
    if (adjust) {
        std::time_t epochTime = _dateTime.timestamp().epochTime();
#if defined(_WIN32)
#if defined(_WIN32_WCE)
        std::tm* broken = wceex_localtime(&epochTime);
#else
        std::tm* broken = std::localtime(&epochTime);
#endif
        if (!broken)
            throw std::runtime_error("System error: Cannot get local time");
        _tzd = (Timezone::utcOffset() + ((broken->tm_isdst == 1) ? 3600 : 0));
#else
        std::tm broken;
        if (!localtime_r(&epochTime, &broken))
            throw std::runtime_error("System error: Cannot get local time");
        _tzd = (Timezone::utcOffset() + ((broken.tm_isdst == 1) ? 3600 : 0));
#endif
        adjustForTzd();
    } else {
        int dst;
        dstOffset(dst);
        _tzd = (Timezone::utcOffset() + dst);
    }
}


std::time_t LocalDateTime::dstOffset(int& dstOffset) const
{
    std::time_t local;
    std::tm broken;

    broken.tm_year = (_dateTime.year() - 1900);
    broken.tm_mon = (_dateTime.month() - 1);
    broken.tm_mday = _dateTime.day();
    broken.tm_hour = _dateTime.hour();
    broken.tm_min = _dateTime.minute();
    broken.tm_sec = _dateTime.second();
    broken.tm_isdst = -1;
#if defined(_WIN32_WCE)
    local = wceex_mktime(&broken);
#else
    local = std::mktime(&broken);
#endif

    dstOffset = (broken.tm_isdst == 1) ? 3600 : 0;
    return local;
}


//
// Timezone
//


int Timezone::tzd()
{
    return utcOffset() + dst();
}


#if defined(_WIN32)


//
// Timezone: Win
//


int Timezone::utcOffset()
{
    TIME_ZONE_INFORMATION tzInfo;
    DWORD dstFlag = GetTimeZoneInformation(&tzInfo);
    (void)dstFlag;
    return -tzInfo.Bias * 60;
}


int Timezone::dst()
{
    TIME_ZONE_INFORMATION tzInfo;
    DWORD dstFlag = GetTimeZoneInformation(&tzInfo);
    (void)dstFlag;
    return dstFlag == TIME_ZONE_ID_DAYLIGHT ? -tzInfo.DaylightBias * 60 : 0;
}


bool Timezone::isDst(const Timestamp& timestamp)
{
    std::time_t time = timestamp.epochTime();
    struct std::tm* tms = std::localtime(&time);
    if (!tms)
        throw std::runtime_error(
            "System error: cannot get local time DST flag");
    return tms->tm_isdst > 0;
}


std::string Timezone::name()
{
    std::string result;
    TIME_ZONE_INFORMATION tzInfo;
    DWORD dstFlag = GetTimeZoneInformation(&tzInfo);
    WCHAR* ptr = dstFlag == TIME_ZONE_ID_DAYLIGHT ? tzInfo.DaylightName
                                                  : tzInfo.StandardName;
    // #if defined(UNICODE)
    //     UnicodeConverter::toUTF8(ptr, result);
    // #else
    char buffer[256];
    DWORD rc = WideCharToMultiByte(CP_ACP, 0, ptr, -1, buffer, sizeof(buffer),
                                   nullptr, nullptr);
    if (rc)
        result = buffer;
    // #endif
    return result;
}


std::string Timezone::standardName()
{
    std::string result;
    TIME_ZONE_INFORMATION tzInfo;
    DWORD dstFlag = GetTimeZoneInformation(&tzInfo);
    WCHAR* ptr = tzInfo.StandardName;
    (void)dstFlag;
    // #if defined(UNICODE)
    //     UnicodeConverter::toUTF8(ptr, result);
    // #else
    char buffer[256];
    DWORD rc = WideCharToMultiByte(CP_ACP, 0, ptr, -1, buffer, sizeof(buffer),
                                   nullptr, nullptr);
    if (rc)
        result = buffer;
    // #endif
    return result;
}


std::string Timezone::dstName()
{
    std::string result;
    TIME_ZONE_INFORMATION tzInfo;
    DWORD dstFlag = GetTimeZoneInformation(&tzInfo);
    WCHAR* ptr = tzInfo.DaylightName;
    (void)dstFlag;
    // #if defined(UNICODE)
    //     UnicodeConverter::toUTF8(ptr, result);
    // #else
    char buffer[256];
    DWORD rc = WideCharToMultiByte(CP_ACP, 0, ptr, -1, buffer, sizeof(buffer),
                                   nullptr, nullptr);
    if (rc)
        result = buffer;
    // #endif
    return result;
}


#else


//
// Timezone: Unix
//

class Base_API TZInfo
{
public:
    TZInfo() { tzset(); }

    int timeZone()
    {
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(POCO_ANDROID) // no timezone global var
        std::time_t now = std::time(nullptr);
        struct std::tm t;
        gmtime_r(&now, &t);
        std::time_t utc = std::mktime(&t);
        return now - utc;
#elif defined(__CYGWIN__)
        return -_timezone;
#else
        return -timezone;
#endif
    }

    const char* name(bool dst) { return tzname[dst ? 1 : 0]; }
};


static TZInfo tzInfo;


int Timezone::utcOffset()
{
    return tzInfo.timeZone();
}


int Timezone::dst()
{
    std::time_t now = std::time(nullptr);
    struct std::tm t;
    if (!localtime_r(&now, &t))
        throw std::runtime_error("System error: cannot get local time DST offset");
    return t.tm_isdst == 1 ? 3600 : 0;
}


bool Timezone::isDst(const Timestamp& timestamp)
{
    std::time_t time = timestamp.epochTime();
    struct std::tm* tms = std::localtime(&time);
    if (!tms)
        throw std::runtime_error("System error: cannot get local time DST flag");
    return tms->tm_isdst > 0;
}


std::string Timezone::name()
{
    return std::string(tzInfo.name(dst() != 0));
}


std::string Timezone::standardName()
{
    return std::string(tzInfo.name(false));
}


std::string Timezone::dstName()
{
    return std::string(tzInfo.name(true));
}


#endif


//
// DateTimeFormat
//


const std::string DateTimeFormat::ISO8601_FORMAT("%Y-%m-%dT%H:%M:%S%z");
const std::string DateTimeFormat::ISO8601_FRAC_FORMAT("%Y-%m-%dT%H:%M:%s%z");
const std::string DateTimeFormat::RFC822_FORMAT("%w, %e %b %y %H:%M:%S %Z");
const std::string DateTimeFormat::RFC1123_FORMAT("%w, %e %b %Y %H:%M:%S %Z");
const std::string DateTimeFormat::HTTP_FORMAT("%w, %d %b %Y %H:%M:%S %Z");
const std::string DateTimeFormat::RFC850_FORMAT("%W, %e-%b-%y %H:%M:%S %Z");
const std::string DateTimeFormat::RFC1036_FORMAT("%W, %e %b %y %H:%M:%S %Z");
const std::string DateTimeFormat::ASCTIME_FORMAT("%w %b %f %H:%M:%S %Y");
const std::string DateTimeFormat::SORTABLE_FORMAT("%Y-%m-%d %H:%M:%S");


const std::string DateTimeFormat::WEEKDAY_NAMES[] = {
    "Sunday",   "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"};


const std::string DateTimeFormat::MONTH_NAMES[] = {
    "January", "February", "March",     "April",   "May",      "June",
    "July",    "August",   "September", "October", "November", "December"};


//
// DateTimeFormatter
//
/*

void DateTimeFormatter::append(std::string& str, const LocalDateTime& dateTime,
                               const std::string& fmt)
{
    DateTimeFormatter::append(str, dateTime.utc(), fmt, dateTime.tzd());
}


void DateTimeFormatter::append(std::string& str, const DateTime& dateTime,
                               const std::string& fmt, int timeZoneDifferential)
{
   
}


void DateTimeFormatter::append(std::string& str, const Timespan& timespan,
                               const std::string& fmt)
{
    
}

void DateTimeFormatter::tzdISO(std::string& str, int timeZoneDifferential)
{
   
}


void DateTimeFormatter::tzdRFC(std::string& str, int timeZoneDifferential)
{

}

*/
//
// DateTimeParser
//


#define SKIP_JUNK()                                                            \
    while (it != end && !::isdigit(*it))                                       \
    ++it


#define SKIP_DIGITS()                                                          \
    while (it != end && ::isdigit(*it))                                        \
    ++it


#define PARSE_NUMBER(var)                                                      \
    while (it != end && ::isdigit(*it))                                        \
    var = var * 10 + ((*it++) - '0')


#define PARSE_NUMBER_N(var, n)                                                 \
    {                                                                          \
        int i = 0;                                                             \
        while (i++ < n && it != end && ::isdigit(*it))                         \
            var = var * 10 + ((*it++) - '0');                                  \
    }


#define PARSE_FRACTIONAL_N(var, n)                                             \
    {                                                                          \
        int i = 0;                                                             \
        while (i < n && it != end && ::isdigit(*it)) {                         \
            var = var * 10 + ((*it++) - '0');                                  \
            i++;                                                               \
        }                                                                      \
        while (i++ < n)                                                        \
            var *= 10;                                                         \
    }


void DateTimeParser::parse(const std::string& fmt, const std::string& str,
                           DateTime& dateTime, int& timeZoneDifferential)
{
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    int millis = 0;
    int micros = 0;
    int tzd = 0;

    std::string::const_iterator it = str.begin();
    std::string::const_iterator end = str.end();
    std::string::const_iterator itf = fmt.begin();
    std::string::const_iterator endf = fmt.end();

    while (itf != endf && it != end) {
        if (*itf == '%') {
            if (++itf != endf) {
                switch (*itf) {
                    case 'w':
                    case 'W':
                        while (it != end && ::isspace(*it))
                            ++it;
                        while (it != end && ::isalpha(*it))
                            ++it;
                        break;
                    case 'b':
                    case 'B':
                        month = parseMonth(it, end);
                        break;
                    case 'd':
                    case 'e':
                    case 'f':
                        SKIP_JUNK();
                        PARSE_NUMBER_N(day, 2);
                        break;
                    case 'm':
                    case 'n':
                    case 'o':
                        SKIP_JUNK();
                        PARSE_NUMBER_N(month, 2);
                        break;
                    case 'y':
                        SKIP_JUNK();
                        PARSE_NUMBER_N(year, 2);
                        if (year >= 69)
                            year += 1900;
                        else
                            year += 2000;
                        break;
                    case 'Y':
                        SKIP_JUNK();
                        PARSE_NUMBER_N(year, 4);
                        break;
                    case 'r':
                        SKIP_JUNK();
                        PARSE_NUMBER(year);
                        if (year < 1000) {
                            if (year >= 69)
                                year += 1900;
                            else
                                year += 2000;
                        }
                        break;
                    case 'H':
                    case 'h':
                        SKIP_JUNK();
                        PARSE_NUMBER_N(hour, 2);
                        break;
                    case 'a':
                    case 'A':
                        hour = parseAMPM(it, end, hour);
                        break;
                    case 'M':
                        SKIP_JUNK();
                        PARSE_NUMBER_N(minute, 2);
                        break;
                    case 'S':
                        SKIP_JUNK();
                        PARSE_NUMBER_N(second, 2);
                        break;
                    case 's':
                        SKIP_JUNK();
                        PARSE_NUMBER_N(second, 2);
                        if (it != end && (*it == '.' || *it == ',')) {
                            ++it;
                            PARSE_FRACTIONAL_N(millis, 3);
                            PARSE_FRACTIONAL_N(micros, 3);
                            SKIP_DIGITS();
                        }
                        break;
                    case 'i':
                        SKIP_JUNK();
                        PARSE_NUMBER_N(millis, 3);
                        break;
                    case 'c':
                        SKIP_JUNK();
                        PARSE_NUMBER_N(millis, 1);
                        millis *= 100;
                        break;
                    case 'F':
                        SKIP_JUNK();
                        PARSE_FRACTIONAL_N(millis, 3);
                        PARSE_FRACTIONAL_N(micros, 3);
                        SKIP_DIGITS();
                        break;
                    case 'z':
                    case 'Z':
                        tzd = parseTZD(it, end);
                        break;
                }
                ++itf;
            }
        } else
            ++itf;
    }
    if (month == 0)
        month = 1;
    if (day == 0)
        day = 1;
    if (DateTime::isValid(year, month, day, hour, minute, second, millis, micros))
        dateTime.assign(year, month, day, hour, minute, second, millis, micros);
    else
        throw std::runtime_error("Syntax error: date/time component out of range");
    timeZoneDifferential = tzd;
}


DateTime DateTimeParser::parse(const std::string& fmt, const std::string& str,
                               int& timeZoneDifferential)
{
    DateTime result;
    parse(fmt, str, result, timeZoneDifferential);
    return result;
}


bool DateTimeParser::tryParse(const std::string& fmt, const std::string& str,
                              DateTime& dateTime, int& timeZoneDifferential)
{
    try {
        parse(fmt, str, dateTime, timeZoneDifferential);
    } catch (std::exception&) {
        return false;
    }
    return true;
}


void DateTimeParser::parse(const std::string& str, DateTime& dateTime,
                           int& timeZoneDifferential)
{
    if (!tryParse(str, dateTime, timeZoneDifferential))
        throw std::runtime_error("Syntax error: Unsupported or invalid date/time format");
}


DateTime DateTimeParser::parse(const std::string& str,
                               int& timeZoneDifferential)
{
    DateTime result;
    if (tryParse(str, result, timeZoneDifferential))
        return result;
    else
        throw std::runtime_error("Syntax error: Unsupported or invalid date/time format");
}


bool DateTimeParser::tryParse(const std::string& str, DateTime& dateTime,
                              int& timeZoneDifferential)
{
    if (str.length() < 4)
        return false;

    if (str[3] == ',')
        return tryParse("%w, %e %b %r %H:%M:%S %Z", str, dateTime,
                        timeZoneDifferential);
    else if (str[3] == ' ')
        return tryParse(DateTimeFormat::ASCTIME_FORMAT, str, dateTime,
                        timeZoneDifferential);
    else if (str.find(',') < 10)
        return tryParse("%W, %e %b %r %H:%M:%S %Z", str, dateTime,
                        timeZoneDifferential);
    else if (::isdigit(str[0])) {
        if (str.find(' ') != std::string::npos || str.length() == 10)
            return tryParse(DateTimeFormat::SORTABLE_FORMAT, str, dateTime,
                            timeZoneDifferential);
        else if (str.find('.') != std::string::npos ||
                 str.find(',') != std::string::npos)
            return tryParse(DateTimeFormat::ISO8601_FRAC_FORMAT, str, dateTime,
                            timeZoneDifferential);
        else
            return tryParse(DateTimeFormat::ISO8601_FORMAT, str, dateTime,
                            timeZoneDifferential);
    } else
        return false;
}


int DateTimeParser::parseTZD(std::string::const_iterator& it,
                             const std::string::const_iterator& end)
{
    struct Zone
    {
        const char* designator;
        int timeZoneDifferential;
    };

    static Zone zones[] = {{"Z", 0},
                           {"UT", 0},
                           {"GMT", 0},
                           {"BST", 1 * 3600},
                           {"IST", 1 * 3600},
                           {"WET", 0},
                           {"WEST", 1 * 3600},
                           {"CET", 1 * 3600},
                           {"CEST", 2 * 3600},
                           {"EET", 2 * 3600},
                           {"EEST", 3 * 3600},
                           {"MSK", 3 * 3600},
                           {"MSD", 4 * 3600},
                           {"NST", -3 * 3600 - 1800},
                           {"NDT", -2 * 3600 - 1800},
                           {"AST", -4 * 3600},
                           {"ADT", -3 * 3600},
                           {"EST", -5 * 3600},
                           {"EDT", -4 * 3600},
                           {"CST", -6 * 3600},
                           {"CDT", -5 * 3600},
                           {"MST", -7 * 3600},
                           {"MDT", -6 * 3600},
                           {"PST", -8 * 3600},
                           {"PDT", -7 * 3600},
                           {"AKST", -9 * 3600},
                           {"AKDT", -8 * 3600},
                           {"HST", -10 * 3600},
                           {"AEST", 10 * 3600},
                           {"AEDT", 11 * 3600},
                           {"ACST", 9 * 3600 + 1800},
                           {"ACDT", 10 * 3600 + 1800},
                           {"AWST", 8 * 3600},
                           {"AWDT", 9 * 3600}};

    int tzd = 0;
    while (it != end && ::isspace(*it))
        ++it;
    if (it != end) {
        if (::isalpha(*it)) {
            std::string designator;
            designator += *it++;
            if (it != end && ::isalpha(*it))
                designator += *it++;
            if (it != end && ::isalpha(*it))
                designator += *it++;
            if (it != end && ::isalpha(*it))
                designator += *it++;
            for (unsigned i = 0; i < sizeof(zones) / sizeof(Zone); ++i) {
                if (designator == zones[i].designator) {
                    tzd = zones[i].timeZoneDifferential;
                    break;
                }
            }
        }
        if (it != end && (*it == '+' || *it == '-')) {
            int sign = *it == '+' ? 1 : -1;
            ++it;
            int hours = 0;
            PARSE_NUMBER_N(hours, 2);
            if (it != end && *it == ':')
                ++it;
            int minutes = 0;
            PARSE_NUMBER_N(minutes, 2);
            tzd += sign * (hours * 3600 + minutes * 60);
        }
    }
    return tzd;
}


int DateTimeParser::parseMonth(std::string::const_iterator& it,
                               const std::string::const_iterator& end)
{
    std::string month;
    while (it != end && (::isspace(*it) || ::ispunct(*it)))
        ++it;
    bool isFirst = true;
    while (it != end && ::isalpha(*it)) {
        char ch = (*it++);
        if (isFirst) {
            month += static_cast<char>(::toupper(ch));
            isFirst = false;
        } else
            month += static_cast<char>(::tolower(ch));
    }
    if (month.length() < 3)
        throw std::runtime_error("Syntax error: Month name must be at least "
                                 "three characters long: " + month);
    for (int i = 0; i < 12; ++i) {
        if (DateTimeFormat::MONTH_NAMES[i].find(month) == 0)
            return i + 1;
    }
    throw std::runtime_error("Syntax error: Not a valid month name: " + month);
}


int DateTimeParser::parseDayOfWeek(std::string::const_iterator& it,
                                   const std::string::const_iterator& end)
{
    std::string dow;
    while (it != end && (::isspace(*it) || ::ispunct(*it)))
        ++it;
    bool isFirst = true;
    while (it != end && ::isalpha(*it)) {
        char ch = (*it++);
        if (isFirst) {
            dow += static_cast<char>(::toupper(ch));
            isFirst = false;
        } else
            dow += static_cast<char>(::tolower(ch));
    }
    if (dow.length() < 3)
        throw std::runtime_error("Syntax error: Weekday name must be at least "
                                 "three characters long: " +
                                 dow);
    for (int i = 0; i < 7; ++i) {
        if (DateTimeFormat::WEEKDAY_NAMES[i].find(dow) == 0)
            return i;
    }
    throw std::runtime_error("Syntax error: Not a valid weekday name: " + dow);
}


int DateTimeParser::parseAMPM(std::string::const_iterator& it,
                              const std::string::const_iterator& end, int hour)
{
    std::string ampm;
    while (it != end && (::isspace(*it) || ::ispunct(*it)))
        ++it;
    while (it != end && ::isalpha(*it)) {
        char ch = (*it++);
        ampm += static_cast<char>(::toupper(ch));
    }
    if (ampm == "AM") {
        if (hour == 12)
            return 0;
        else
            return hour;
    } else if (ampm == "PM") {
        if (hour < 12)
            return hour + 12;
        else
            return hour;
    } else
        throw std::runtime_error(
            "Syntax error: Not a valid AM/PM designator: " + ampm);
}


//
// Timestamp
//


Timestamp::Timestamp()
{
    update();
}


Timestamp::Timestamp(TimeVal tv)
{
    _ts = tv;
}


Timestamp::Timestamp(const Timestamp& other)
{
    _ts = other._ts;
}


Timestamp::~Timestamp()
{
}


Timestamp& Timestamp::operator=(const Timestamp& other)
{
    _ts = other._ts;
    return *this;
}


Timestamp& Timestamp::operator=(TimeVal tv)
{
    _ts = tv;
    return *this;
}


void Timestamp::swap(Timestamp& timestamp)
{
    std::swap(_ts, timestamp._ts);
}


Timestamp Timestamp::fromEpochTime(std::time_t t)
{
    return Timestamp(TimeVal(t) * resolution());
}


Timestamp Timestamp::fromUtcTime(UtcTimeVal val)
{
    val -= (TimeDiff(0x01b21dd2) << 32) + 0x13814000;
    val /= 10;
    return Timestamp(val);
}


void Timestamp::update()
{
#if defined(WIN32)

    FILETIME ft;
#if defined(_WIN32_WCE)
    GetSystemTimeAsFileTimeWithMillisecondResolution(&ft);
#else
    GetSystemTimeAsFileTime(&ft);
#endif

    ULARGE_INTEGER epoch; // UNIX epoch (1970-01-01 00:00:00) expressed in
                          // Windows NT FILETIME
    epoch.LowPart = 0xD53E8000;
    epoch.HighPart = 0x019DB1DE;

    ULARGE_INTEGER ts;
    ts.LowPart = ft.dwLowDateTime;
    ts.HighPart = ft.dwHighDateTime;
    ts.QuadPart -= epoch.QuadPart;
    _ts = ts.QuadPart / 10;

#else

    struct timeval tv;
    if (gettimeofday(&tv, nullptr))
        throw std::runtime_error("System error: Cannot get time of day");
    _ts = TimeVal(tv.tv_sec) * resolution() + tv.tv_usec;

#endif
}


//
// Timespan
//


const Timespan::TimeDiff Timespan::MILLISECONDS = 1000;
const Timespan::TimeDiff Timespan::SECONDS = 1000 * Timespan::MILLISECONDS;
const Timespan::TimeDiff Timespan::MINUTES = 60 * Timespan::SECONDS;
const Timespan::TimeDiff Timespan::HOURS = 60 * Timespan::MINUTES;
const Timespan::TimeDiff Timespan::DAYS = 24 * Timespan::HOURS;


Timespan::Timespan()
    : _span(0)
{
}


Timespan::Timespan(TimeDiff microseconds)
    : _span(microseconds)
{
}


Timespan::Timespan(long seconds, long microseconds)
    : _span(TimeDiff(seconds) * SECONDS + microseconds)
{
}


Timespan::Timespan(int days, int hours, int minutes, int seconds, int microseconds)
    : _span(TimeDiff(microseconds) + 
            TimeDiff(seconds) * SECONDS +
            TimeDiff(minutes) * MINUTES + 
            TimeDiff(hours) * HOURS +
            TimeDiff(days) * DAYS)
{
}


Timespan::Timespan(const Timespan& timespan)
    : _span(timespan._span)
{
}


Timespan::~Timespan()
{
}


Timespan& Timespan::operator=(const Timespan& timespan)
{
    _span = timespan._span;
    return *this;
}


Timespan& Timespan::operator=(TimeDiff microseconds)
{
    _span = microseconds;
    return *this;
}


Timespan& Timespan::assign(int days, int hours, int minutes, int seconds,
                           int microseconds)
{
    _span = TimeDiff(microseconds) + TimeDiff(seconds) * SECONDS +
            TimeDiff(minutes) * MINUTES + TimeDiff(hours) * HOURS +
            TimeDiff(days) * DAYS;
    return *this;
}


Timespan& Timespan::assign(long seconds, long microseconds)
{
    _span = TimeDiff(seconds) * SECONDS + TimeDiff(microseconds);
    return *this;
}


void Timespan::swap(Timespan& timespan)
{
    std::swap(_span, timespan._span);
}


Timespan Timespan::operator+(const Timespan& d) const
{
    return Timespan(_span + d._span);
}


Timespan Timespan::operator-(const Timespan& d) const
{
    return Timespan(_span - d._span);
}


Timespan& Timespan::operator+=(const Timespan& d)
{
    _span += d._span;
    return *this;
}


Timespan& Timespan::operator-=(const Timespan& d)
{
    _span -= d._span;
    return *this;
}


Timespan Timespan::operator+(TimeDiff microseconds) const
{
    return Timespan(_span + microseconds);
}


Timespan Timespan::operator-(TimeDiff microseconds) const
{
    return Timespan(_span - microseconds);
}


Timespan& Timespan::operator+=(TimeDiff microseconds)
{
    _span += microseconds;
    return *this;
}


Timespan& Timespan::operator-=(TimeDiff microseconds)
{
    _span -= microseconds;
    return *this;
}


//
// Stopwatch
//


Stopwatch::Stopwatch()
    : _elapsed(0)
    , _running(false)
{
}


Stopwatch::~Stopwatch()
{
}


void Stopwatch::start()
{
    if (!_running) {
        _start.update();
        _running = true;
    }
}


void Stopwatch::stop()
{
    if (_running) {
        Timestamp current;
        _elapsed += current - _start;
        _running = false;
    }
}


int Stopwatch::elapsedSeconds() const
{
    return int(elapsed() / resolution());
}


int Stopwatch::elapsedMilliseconds() const
{
    return int(elapsed() / (resolution() / 1000));
}


Timestamp::TimeVal Stopwatch::resolution()
{
    return Timestamp::resolution();
}


Timestamp::TimeDiff Stopwatch::elapsed() const
{
    if (_running) {
        Timestamp current;
        return _elapsed + (current - _start);
    } else {
        return _elapsed;
    }
}


void Stopwatch::reset()
{
    _elapsed = 0;
    _running = false;
}


void Stopwatch::restart()
{
    _elapsed = 0;
    _start.update();
    _running = true;
}


} // namespace base


/// @\}


//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
