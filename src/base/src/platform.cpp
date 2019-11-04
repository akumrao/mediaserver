

#include "base/platform.h"
#include "base/error.h"

#include "uv.h"

#include <stdlib.h>
#include <string.h>

#ifdef base_WIN
#include <windows.h>
#include <winsock2.h>
#else
#include <unistd.h>
#endif


#define PATHMAX 1024


namespace base {


std::string getExePath()
{
    char buf[PATHMAX];
    size_t size = PATHMAX;
    if (uv_exepath(buf, &size) != 0)
        throw std::runtime_error(
            "System error: Cannot resolve executable path");
    return std::string(buf, size);
}


std::string getCwd()
{
    char buf[PATHMAX];
    size_t size = PATHMAX;
    if (uv_cwd(buf, &size) != 0)
        throw std::runtime_error(
            "System error: Cannot resolve working directory");
    return std::string(buf);
}


uint64_t getFreeMemory()
{
    return uv_get_free_memory();
}


uint64_t getTotalMemory()
{
    return uv_get_total_memory();
}


int numCpuCores()
{
    uv_cpu_info_t *info;
    int cpu_count;
    uv_cpu_info(&info, &cpu_count);
    uv_free_cpu_info(info, cpu_count);
    return cpu_count;
}


void sleep(int ms)
{
#ifdef base_WIN
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}


void pause()
{
    std::puts("Press enter to continue...");
    std::getchar();
}


std::string getHostname()
{
    char name[256];
    gethostname(name, 256);
    return name;
}


std::string getEnv(const std::string& name, const std::string& defaultValue)
{
    const char* value = getenv(name.c_str());
    if (value)
        return value;
    return defaultValue;
}


bool getEnvBool(const std::string& name)
{
    const char* value = getenv(name.c_str());
    return value && (
        strcmp(value, "1") == 0 ||
        strcmp(value, "true") == 0);
}




} // namespace base


/// @\}
