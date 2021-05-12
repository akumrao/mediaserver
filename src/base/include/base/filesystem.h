/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */



#ifndef base_FileSystem_H
#define base_FileSystem_H


#include "base/base.h"
#include <string>
#include <vector>
#include <cstdint>


namespace base {
namespace fs {


/// The platform specific path split separator:
/// "/" on unix and '\\' on windows.
Base_API extern const char* separator;

/// The platform specific path split delimiter:
/// '/' on unix and '\\' on windows.
Base_API extern const char delimiter;

/// Returns the file name and extension part of the given path.
Base_API std::string filename(const std::string& path);

/// Returns the file name sans extension.
Base_API std::string basename(const std::string& path);

/// Returns the directory part of the path.
Base_API std::string dirname(const std::string& path);

/// Returns the file extension part of the path.
Base_API std::string extname(const std::string& path, bool includeDot = false);

/// Returns true if the file or directory exists.
Base_API bool exists(const std::string& path);

/// Returns true if the directory exists on the system.
Base_API bool isdir(const std::string& path);

/// Returns the size in bytes of the given file, or -1 if file doesn't exist.
Base_API std::int64_t filesize(const std::string& path);

/// Returns a list of all files and folders in the directory.
Base_API void readdir(const std::string& path, std::vector<std::string>& res);
Base_API void readdir_filter(const std::string& path, std::vector<std::string>& res, const std::string& filter);

/// Creates a directory.
Base_API void mkdir(const std::string& path, int mode = 0755);

/// Creates a directory recursively.
Base_API void mkdirr(const std::string& path, int mode = 0755);

/// Creates a directory.
Base_API void rmdir(const std::string& path);

/// Deletes a file.
Base_API void unlink(const std::string& path);

/// Renames or moves the given file to the target path.
Base_API void rename(const std::string& path, const std::string& target);

/// Adds the trailing directory separator to the given path string.
/// If the last character is already a separator nothing will be done.
Base_API void addsep(std::string& path);

/// Appends the given node to the path.
/// If the given path has no trailing separator one will be appended.
Base_API void addnode(std::string& path, const std::string& node);

/// Normalizes a path for the current opearting system.
/// Currently this function only converts directory separators to native style.
Base_API std::string normalize(const std::string& path);

/// Transcodes the path to into windows native format if using windows
/// and if LibSourcey was compiled with Unicode support (base_UNICODE),
/// otherwise the path string is returned unchanged.
Base_API std::string transcode(const std::string& path);

/// Saves the given data buffer to the output file path.
/// Returns true on success, or if whiny is set then an
/// exception will be thrown on error.
Base_API bool savefile(const std::string& path, const char* data, 
                        size_t size, bool whiny = false);

// TODO: Implement more libuv fs_* types


} // namespace fs
} // namespace base


#endif

