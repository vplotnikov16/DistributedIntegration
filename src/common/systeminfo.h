#pragma once

#include <cstdint>
#include <string>

#include <cereal/cereal.hpp>

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

/**
 * @struct SystemInfo
 * @brief Структура с информацией о системе клиента
 */
struct SystemInfo
{
    // ==== Основная информация о системе ====
    OSType os_type = OSType::Unknown;
    Architecture architecture = Architecture::Unknown;

    // ==== Основная информация о процессоре ====
    uint32_t cpu_cores = 0;

    // ==== ОЗУ ====
    uint64_t total_ram_mb = 0;

    /**
     * @brief Преобразует информацию о системе в строку для вывода
     * @return Строка с форматированной информацией
     */
    std::string to_string() const;

    /**
     * @brief Метод сериализации для Cereal
     * @tparam Archive Тип архива Cereal
     */
    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(
            CEREAL_NVP(os_type),
            CEREAL_NVP(architecture),
            CEREAL_NVP(cpu_cores),
            CEREAL_NVP(total_ram_mb)
        );
    }
};
