#pragma once

#include <boost/asio.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <sstream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include "logger.h"

// Кроссплатформенная поддержка htonl/ntohl
#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <arpa/inet.h>
#endif

/**
 * @file net_utils.h
 * @brief В данном модуле представлены утилиты для сетевого взаимодействия с сериализацией данных
 *
 * Предоставляет функции для отправки и получения данных через TCP сокеты
 * с использованием библиотеки Cereal для кроссплатформенной сериализации.
 */

using tcp = boost::asio::ip::tcp;

std::string get_remote_address(tcp::socket &socket)
{
    try
    {
        if (socket.is_open())
        {
            return socket.remote_endpoint().address().to_string();
        }
    }
    catch (const boost::system::system_error &)
    {
        // Игнорируем ошибку
    }
    return "unknown";
}

namespace net_utils
{
    /**
     * @brief Отправляет сериализованные данные через TCP сокет
     *
     * Функция сериализует данные в бинарный формат с помощью Cereal,
     * затем отправляет размер данных (4 байта) и сами данные.
     *
     * @tparam T Тип данных для отправки (должен поддерживать сериализацию Cereal)
     * @param socket Открытый TCP сокет Boost.Asio
     * @param data Данные для отправки
     * 
     * @code
     * tcp::socket socket(io_context);
     * SystemInfo info = collect_system_info();
     * net_utils::send_data(socket, info);
     * @endcode
     */
    template <typename T>
    void send_data(tcp::socket &socket, const T &data)
    {
        try
        {
            // Сериализация данных в строку
            std::ostringstream oss;
            {
                cereal::BinaryOutputArchive archive(oss);
                archive(data);
            }
            std::string serialized = oss.str();

            // Отправка размера данных в сетевом порядке байт
            uint32_t size = static_cast<uint32_t>(serialized.size());
            uint32_t network_size = htonl(size); // Host to network byte order

            LOG_DEBUG("Sending data: {} bytes to {}",
                      size,
                      get_remote_address(socket));

            boost::asio::write(
                socket,
                boost::asio::buffer(&network_size, sizeof(network_size)));

            // Отправка самих данных
            boost::asio::write(
                socket,
                boost::asio::buffer(serialized.data(), serialized.size()));

            LOG_TRACE("Data sent successfully");
        }
        catch (const boost::system::system_error &e)
        {
            LOG_ERROR("Network error: {}", e.what());
            throw std::runtime_error(
                std::string("Network error while sending data: ") + e.what());
        }
        catch (const cereal::Exception &e)
        {
            LOG_ERROR("Serialization error: {}", e.what());
            throw std::runtime_error(
                std::string("Serialization error: ") + e.what());
        }
    }

    /**
     * @brief Получает и десериализует данные из TCP сокета
     *
     * Функция сначала получает размер данных (4 байта), затем получает
     * сами данные и десериализует их с помощью Cereal.
     *
     * @tparam T Тип данных для получения (должен поддерживать сериализацию Cereal)
     * @param socket Открытый TCP сокет Boost.Asio
     * @return Десериализованные данные типа T
     * @throws boost::system::system_error Если произошла ошибка сети
     * @throws cereal::Exception Если произошла ошибка десериализации
     * @throws std::runtime_error Если размер данных некорректен
     *
     * @code
     * tcp::socket socket(io_context);
     * SystemInfo info = net_utils::receive_data<SystemInfo>(socket);
     * @endcode
     */
    template <typename T>
    T receive_data(boost::asio::ip::tcp::socket &socket)
    {
        try
        {
            // Получение размера данных
            uint32_t network_size;
            boost::asio::read(
                socket,
                boost::asio::buffer(&network_size, sizeof(network_size)));
            // Сетевой порядок байт
            uint32_t size = ntohl(network_size);

            LOG_DEBUG("Receiving data: {} bytes from {}",
                      size,
                      socket.remote_endpoint().address().to_string());

            // Валидация размера
            constexpr uint32_t MAX_PACKET_SIZE = 100 * 1024 * 1024; // 100 MB
            if (size == 0 || size > MAX_PACKET_SIZE)
            {
                LOG_ERROR("Invalid packet size: {} bytes", size);
                throw std::runtime_error(
                    "Invalid packet size: " + std::to_string(size));
            }

            // Получение данных
            std::string buffer(size, '\0');
            boost::asio::read(
                socket,
                boost::asio::buffer(buffer.data(), size));

            // Десериализация напрямую
            std::istringstream iss(std::move(buffer));
            T data;
            {
                cereal::BinaryInputArchive archive(iss);
                archive(data);
            }

            LOG_TRACE("Data received and deserialized successfully");
            return data;
        }
        catch (const boost::system::system_error &e)
        {
            LOG_ERROR("Network error: {}", e.what());
            throw std::runtime_error(
                std::string("Network error while receiving data: ") + e.what());
        }
        catch (const cereal::Exception &e)
        {
            LOG_ERROR("Deserialization error: {}", e.what());
            throw std::runtime_error(
                std::string("Deserialization error: ") + e.what());
        }
    }

} // namespace net_utils
