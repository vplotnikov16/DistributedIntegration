#pragma once

#include <cstdint>
#include <string>

enum class OSType : uint8_t
{
    Windows,
    Linux,
    MacOS,
    Unknown
};

enum class Architecture : uint8_t
{
    x86,
    x64,
    ARM,
    ARM64,
    Unknown
};

// Функции для преобразования enum в строку
std::string to_string(OSType os);
std::string to_string(Architecture arch);


struct SystemInfo
{
    // ==== Основная информация о системе ====
    OSType os_type = OSType::Unknown;
    Architecture architecture = Architecture::Unknown;

    // ==== Основная информация о процессоре ====
    uint32_t cpu_cores = 0;

    // ==== ОЗУ ====
    uint64_t total_ram_mb = 0;

    // Метод для удобного вывода
    std::string to_string() const;
};
