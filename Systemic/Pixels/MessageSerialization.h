/**
 * @file
 * @brief Pixel messages serialization functions.
 */

#pragma once

#include <memory>
#include <vector>
#include <type_traits>

#include "Messages.h"

namespace Systemic::Pixels::Messages::Serialization
{
    /**
     * @brief Serialize a PixelMessage instance to binary data so it can be send to a Pixels die.
     * @tparam T The type of PixelMessage.
     * @param message The PixelMessage instance.
     * @param outData The binary data serialized from the given message.
     */
    template <typename T, std::enable_if_t<std::is_base_of_v<Systemic::Pixels::Messages::PixelMessage, T>, int> = 0>
    inline void serializeMessage(const T& message, std::vector<uint8_t>& outData)
    {
        //static_assert(std::is_base_of_v<Message, T>);
        outData.resize(sizeof(T));
        memcpy(outData.data(), &message, sizeof(T));
    }

    /**
     * @brief Deserialize some binary data received from a Pixels die to a PixelMessage.
     * @tparam T The expected type of PixelMessage.
     * @param data The binary data.
     * @return The PixelMessage deserialized from the given binary data.
     * @note It's usually best to call the non template overload.
     */
    template <typename T, std::enable_if_t<std::is_base_of_v<Systemic::Pixels::Messages::PixelMessage, T>, int> = 0>
    inline std::shared_ptr<const T> deserializeMessage(const std::vector<uint8_t>& data)
    {
        if (data.size() == sizeof(T))
        {
            auto msg = std::shared_ptr<T>(new T{});
            memcpy(msg.get(), data.data(), data.size());
            return msg;
        }
        return nullptr;
    }

    /**
     * @brief Deserialize some binary data received from a Pixels die to a PixelMessage.
     * @param data The binary data.
     * @return The PixelMessage deserialized from the given binary data.
     */
    inline std::shared_ptr<const PixelMessage> deserializeMessage(const std::vector<uint8_t>& data)
    {
        if (!data.empty())
        {
            const auto type = static_cast<MessageType>(data[0]);
            switch (type)
            {
            case MessageType::IAmADie:
                return deserializeMessage<IAmADie>(data);
            case MessageType::RollState:
                return deserializeMessage<RollState>(data);
            case MessageType::Blink:
                return deserializeMessage<Blink>(data);
            case MessageType::BatteryLevel:
                return deserializeMessage<BatteryLevel>(data);
            case MessageType::RequestRssi:
                return deserializeMessage<RequestRssi>(data);
            case MessageType::Rssi:
                return deserializeMessage<Rssi>(data);
            }

            if (data.size() == 1 && type != MessageType::None)
            {
                return std::shared_ptr<PixelMessage>(new PixelMessage{ type });
            }
        }
        return nullptr;
    }
}
