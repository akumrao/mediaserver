

#ifndef base_Platform_H
#define base_Platform_H


#include "base/base.h"

#include <string>
#include <cstdint>


namespace base {


//
/// Cross-platform utilities
//

/// Returns the current executable path.
Base_API std::string getExePath();

/// Return the current working directory.
Base_API std::string getCwd();

/// Returns the current amount of free memory.
Base_API uint64_t getFreeMemory();

/// Returns the current amount of used memory.
Base_API uint64_t getTotalMemory();

/// Returns the number of CPU cores.
Base_API int numCpuCores();

/// Pause the current thread for the given ms duration.
Base_API void sleep(int ms);

/// Pause the current thread until enter is pressed.
Base_API void pause();

/// Return the system hostname.
Base_API std::string getHostname();

/// Return an environment variable or the default value.
Base_API std::string getEnv(const std::string& name, const std::string& defaultValue = "");

/// Return an environment variable boolean or the default value.
/// The variable must be `1` or `true` for this function to return true.
Base_API bool getEnvBool(const std::string& name);





} // namespace base


#endif // base_Platform_H

