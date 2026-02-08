#include "utils.h"
#include <thread>

#ifdef _WIN32
    #include <windows.h>
    #include <sysinfoapi.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <sys/utsname.h>
    #include <sys/sysinfo.h>
    #include <sys/types.h>
#elif defined(__APPLE___)
    #include <unistd.h>
    #include <sys/utsname.h>
    #include <sys/types.h>
    #include <sys/sysctl.h>
#endif

namespace sys_utils
{
#ifdef _WIN32
    // Windows-специфичные реализации
    SystemInfo collect_system_info()
    {
        SystemInfo info;

        // Определяем ОС
        info.os_type = OSType::Windows;

        // Архитектура
        SYSTEM_INFO sys_info;
        GetSystemInfo(&sys_info);
        info.architecture = (sys_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ? Architecture::x64 : Architecture::x86;

        // Определяем количество ядер
        info.cpu_cores = std::thread::hardware_concurrency();
        if (info.cpu_cores == 0)
        {
            info.cpu_cores = 1;
        }

        // ОЗУ
        MEMORYSTATUSEX memory_status;
        memory_status.dwLength = sizeof(memory_status);
        GlobalMemoryStatusEx(&memory_status);
        // Переводим в МБайт
        info.total_ram_mb = memory_status.ullTotalPhys / (1024 * 1024);

        return info;
    }

#elif defined(__linux__)
    // Linux-специфичные реализации
    SystemInfo collect_system_info()
    {
        SystemInfo info;

        // Определяем ОС
        info.os_type = OSType::Linux;

        // Архитектура
        struct utsname uname_data;
        if (uname(&uname_data) == 0)
        {
            std::string arch(uname_data.machine);
            if (arch.find("x86_64") != std::string::npos)
            {
                info.architecture = Architecture::x64;
            }
            else if (arch.find("i386") != std::string::npos || arch.find("i686") != std::string::npos)
            {
                info.architecture = Architecture::x86;
            }
            else if (arch.find("arm") != std::string::npos || arch.find("aarch64") != std::string::npos)
            {
                info.architecture = arch.find("64") != std::string::npos ? Architecture::ARM64 : Architecture::ARM;
            }
            else
            {
                info.architecture = Architecture::Unknown;
            }
        }
        else
        {
            info.architecture = Architecture::Unknown;
        }

        // Определяем количество ядер
        info.cpu_cores = std::thread::hardware_concurrency();
        if (info.cpu_cores == 0)
        {
            info.cpu_cores = 1;
        }

        // ОЗУ
        struct sysinfo mem_info;
        if (sysinfo(&mem_info) == 0)
        {

            // Переводим в МБайт
            info.total_ram_mb = (mem_info.totalram * mem_info.mem_unit) / (1024 * 1024);
        }
        else
        {
            info.total_ram_mb = 0;
        }
        return info;
    }

#elif defined(__APPLE__)
    // macOS-специфичные реализации
    SystemInfo collect_system_info()
    {
        SystemInfo info;

        // Определяем ОС
        info.os_type = OSType::MacOS;

// Архитектура
#if defined(__aarch64__) || defined(__arm64__)
        info.architecture = Architecture::ARM64;
#else
        info.architecture = Architecture::x64;
#endif

        // Определяем количество ядер
        info.cpu_cores = std::thread::hardware_concurrency();
        if (info.cpu_cores == 0)
        {
            info.cpu_cores = 1;
        }

        // Получаем информацию о памяти через sysctl
        uint64_t mem_size;
        size_t len = sizeof(mem_size);
        sysctlbyname("hw.memsize", &mem_size, &len, NULL, 0);
        // Переводим в МБайт
        info.total_ram_mb = mem_size / (1024 * 1024);

        return info;
    }

#endif
} // namespace sys_utils

std::string to_string(OSType os)
{
    switch (os)
    {
    case OSType::Windows:
        return "Windows";
    case OSType::Linux:
        return "Linux";
    case OSType::MacOS:
        return "MacOS";
    default:
        return "Unknown";
    }
}

std::string to_string(Architecture arch)
{
    switch (arch)
    {
    case Architecture::x86:
        return "x86";
    case Architecture::x64:
        return "x64";
    case Architecture::ARM:
        return "ARM";
    case Architecture::ARM64:
        return "ARM64";
    default:
        return "Unknown";
    }
}

// Реализация метода to_string для SystemInfo
std::string SystemInfo::to_string() const
{
    std::string result;
    result += "OS: " + ::to_string(os_type) + "\n";
    result += "Architecture: " + ::to_string(architecture) + " " + "\n";
    result += "CPU Cores: " + std::to_string(cpu_cores) + "\n";
    result += "RAM: " + std::to_string(total_ram_mb) + " MB\n";
    return result;
}
