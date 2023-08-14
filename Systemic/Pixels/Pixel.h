/**
 * @file
 * @brief Definition of the Pixel class.
 */

#pragma once

#include <functional>
#include <memory>
#include <algorithm>
#include <limits>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <future>
#include "Systemic/Internal/GuardedList.h"
#include "ScannedPixel.h"
#include "MessageSerialization.h"

namespace Systemic::BluetoothLE
{
    class Peripheral;
    class Characteristic;
}

namespace Systemic::Pixels
{
    /// The different possible connection statuses of a Pixel.
    enum class PixelStatus
    {
        Disconnected,
        Connecting,
        Identifying,
        Ready,
        Disconnecting,
    };

    class Pixel;

#pragma warning(push)
#pragma warning(disable : 4100) // unreferenced formal parameter

    /// Interface for a class that handles Pixel events.
    struct PixelDelegate
    {
        /// Called when the Pixel status changes.
        virtual void onStatusChanged(std::shared_ptr<Pixel> pixel, PixelStatus status) {}
        // TODO virtual void onNameChanged(std::shared_ptr<Pixel> pixel, std::wstring name) {}

        /// Called when the Pixel firmware date changes.
        virtual void onFirmwareDateChanged(std::shared_ptr<Pixel> pixel, std::chrono::system_clock::time_point firmwareDate) {}

        /// Called when the Pixel measured RSSI changes.
        virtual void onRssiChanged(std::shared_ptr<Pixel> pixel, int rssi) {}

        /// Called when the Pixel battery level (in percent) changes.
        virtual void onBatteryLevelChanged(std::shared_ptr<Pixel> pixel, int batteryLevel) {}

        /// Called when the Pixel charging state changes.
        virtual void onChargingStateChanged(std::shared_ptr<Pixel> pixel, bool isCharging) {}

        /// Called when the Pixel roll state changes.
        virtual void onRollStateChanged(std::shared_ptr<Pixel> pixel, PixelRollState state, int face) {}

        /// Called just after the Pixel was rolled.
        virtual void onRolled(std::shared_ptr<Pixel> pixel, int face) {}

        /// Called when the Pixel instance received a message from the actual die.
        virtual void onMessageReceived(std::shared_ptr<Pixel> pixel, std::shared_ptr<const Messages::PixelMessage> message) {}
    };

#pragma warning(pop)

    /**
     * @brief Represents a Pixels die.
     *
     * The delegate object passed to the constructor is used to notify of events
     * such as connection status changes or rolls.
     *
     * Call the connectAsync() method to initiate a connection with the physical die.
     *
     * This class is thread safe.
     */
    class Pixel : public std::enable_shared_from_this<Pixel>, public PixelInfo
    {
        using StatusCallback = std::function<void(PixelStatus)>;
        using MessageCallback = std::function<void(std::shared_ptr<const Messages::PixelMessage>)>;

        // Constant data
        const std::shared_ptr<Systemic::BluetoothLE::Peripheral> _peripheral;
        const std::shared_ptr<PixelDelegate> _delegate;

        // Mutable data
        ScannedPixelData _data;
        PixelStatus _status{};
        std::shared_ptr<Systemic::BluetoothLE::Characteristic> _notifyCharacteristic{};
        std::shared_ptr<Systemic::BluetoothLE::Characteristic> _writeCharacteristic{};

        // Mutex for modifying the above data
        std::recursive_mutex _mutex{};

        // Internal lists of status and message notifications
        GuardedList<StatusCallback> _internalStatusCbs;
        GuardedList<MessageCallback> _internalMsgCbs;

    public:
        /// List of possible Pixel connection results.
        enum class ConnectResult
        {
            /// Connection has succeeded.
            Success,

            /// Connection with the actual device failed.
            ConnectionFailed,

            /// Connection was canceled (usually because of a call to Pixel.disconnect()).
            Cancelled,

            /// Connection failed because the die Pixel id is not the expected one.
            IdentificationMismatch,

            /// Connection failed because the die didn't identify itself in time.
            IdentificationTimeout,

            /// Connection failed because subscribing to the Bluetooth characteristic failed.
            SubscriptionError,
        };

        /**
         * @brief Initializes a new instance of Pixel for the device with the given Bluetooth address.
         * @param address The expected Bluetooth address of the Pixels die.
         * @param delegate The object to receive event notifications from this instance.
         * @return A Pixel instance in a shared pointer.
         * @note The delegate virtual methods should not block the thread and complete
         *       their operations quickly.
         */
        static std::shared_ptr<Pixel> create(
            bluetooth_address_t address,
            std::shared_ptr<PixelDelegate> delegate = nullptr)
        {
            return std::shared_ptr<Pixel>(new Pixel{ ScannedPixel{ ScannedPixelData{ address } }, delegate });
        }

        /**
         * @brief Initializes a new instance of Pixel for the device corresponding to the given
         *        scanned Pixel data.
         * @param scannedPixel The scanned Pixel data identifying the die.
         * @param delegate The object to receive event notifications from this instance.
         * @return A Pixel instance in a shared pointer.
         * @note The delegate virtual methods should not block the thread and complete
         *       their operations quickly.
         */
        static std::shared_ptr<Pixel> create(
            const ScannedPixel& scannedPixel,
            std::shared_ptr<PixelDelegate> delegate = nullptr)
        {
            return std::shared_ptr<Pixel>(new Pixel{ scannedPixel, delegate });
        }

        /**
         * @brief Default virtual destructor.
         */
        virtual ~Pixel() = default;

        /**
         * @brief Gets the last known connection status of the Pixel.
         * @return The last known connection status of the Pixel.
         */
        PixelStatus status() const
        {
            return _status;
        }

        /**
         * @brief Indicates whether the Pixel status is "ready".
         * @return Whether the Pixel status is "ready".
         */
        bool isReady() const
        {
            return _status == PixelStatus::Ready;
        }

        virtual bluetooth_address_t systemId() const override
        {
            return _data.address;
        }

        virtual bluetooth_address_t address() const override
        {
            return _data.address;
        }

        virtual pixel_id_t pixelId() const override
        {
            return _data.pixelId;
        }

        virtual const std::wstring& name() const override
        {
            return _data.name;
        }

        virtual int ledCount() const override
        {
            return _data.ledCount;
        }

        virtual PixelDesignAndColor designAndColor() const override
        {
            return _data.designAndColor;
        }

        virtual Date firmwareDate() const override
        {
            return _data.firmwareDate;
        }

        virtual int rssi() const override
        {
            return _data.rssi;
        }

        virtual int batteryLevel() const override
        {
            return _data.batteryLevel;
        }

        virtual bool isCharging() const override
        {
            return _data.isCharging;
        }

        virtual PixelRollState rollState() const override
        {
            return _data.rollState;
        }

        virtual int currentFace() const override
        {
            return _data.currentFace;
        }

        /**
         * @brief Asynchronously tries to connect to the die.
         * @note The request times out after 7 to 20s if device is not reachable.
         * @return A future with the result of the operation.
         */
        std::future<ConnectResult> connectAsync();

        /**
         * @brief Immediately disconnects from the die.
         */
        void disconnect();

        /**
         * @brief Sends a message to the Pixel.
         * @param type Type of message to send.
         * @param withoutAck Whether to request a confirmation that the message was received.
         * @return A future with a boolean indicating whether the operation succeeded.
         */
        std::future<bool> sendMessageAsync(Messages::MessageType type, bool withoutAck = false)
        {
            std::vector<uint8_t> data{ static_cast<uint8_t>(type) };
            return sendMessageAsync(data, withoutAck);
        }

        /**
         * @brief Sends a message to the Pixel.
         * @tparam T Type of the message.
         * @param message Message to send.
         * @param withoutAck Whether to request a confirmation that the message was received.
         * @return A future with a boolean indicating whether the operation succeeded.
         */
        template <typename T, std::enable_if_t<std::is_base_of_v<Messages::PixelMessage, T>, int> = 0>
        std::future<bool> sendMessageAsync(const T& message, bool withoutAck = false)
        {
            std::vector<uint8_t> data{};
            Messages::Serialization::serializeMessage(message, data);
            return sendMessageAsync(data, withoutAck);
        }

        /**
         * @brief Sends a message to the Pixel and wait for a specific reply.
         * @tparam Rep Duration arithmetic type representing the number of ticks.
         * @tparam Period Duration type representing the tick period.
         * @param type Type of message to send.
         * @param responseType Type of the response to expect.
         * @param timeout Timeout before aborting waiting for the response.
         * @return A message object or nullptr in a shared pointer.
         */
        template <class Rep, class Period>
        std::future<std::shared_ptr<const Messages::PixelMessage>> sendAndWaitForResponseAsync(
            Messages::MessageType type,
            Messages::MessageType responseType,
            std::chrono::duration<Rep, Period> timeout = std::chrono::seconds(5))
        {
            std::promise<std::shared_ptr<const Messages::PixelMessage>> responsePromise{};

            MessageCallback callback{ [&responseType, &responsePromise](auto msg)
                {
                    if (msg->type == responseType)
                    {
                        responsePromise.set_value(msg);
                    }
                } };

            const auto cbIndex = _internalMsgCbs.add(callback);

            const auto sendResult = co_await sendMessageAsync(type);

            std::shared_ptr<const Messages::PixelMessage> response{};
            if (sendResult)
            {
                auto respFuture = responsePromise.get_future();
                if (respFuture.wait_for(timeout) == std::future_status::ready)
                {
                    response = respFuture.get();
                }
            }

            _internalMsgCbs.remove(cbIndex);

            co_return response;
        }

        /**
         * @brief Sends a message to the Pixel and wait for a specific reply.
         * @param type Type of message to send.
         * @param responseType Type of the response to expect.
         * @return A message object or nullptr in a shared pointer.
         */
        std::future<std::shared_ptr<const Messages::PixelMessage>> sendAndWaitForResponseAsync(
            Messages::MessageType type,
            Messages::MessageType responseType)
        {
            return sendAndWaitForResponseAsync(type, responseType, std::chrono::seconds(5));
        }

        /**
         * @brief Requests the Pixel to regularly send its measured RSSI value.
         * @tparam Rep Duration arithmetic type representing the number of ticks.
         * @tparam Period Duration type representing the tick period.
         * @param activate Whether to turn or turn off this feature.
         * @param minInterval The minimum time interval in seconds between two RSSI updates.
         * @return A future with a boolean indicating whether the operation succeeded.
         */
        template <class Rep, class Period>
        std::future<bool> reportRssiAsync(
            bool activate,
            std::chrono::duration<Rep, Period> minInterval)
        {
            Messages::RequestRssi msg{};
            msg.requestMode = activate ? Messages::TelemetryRequestMode::Automatic : Messages::TelemetryRequestMode::Off;
            down_cast(msg.minInterval, std::chrono::milliseconds{ minInterval }.count());
            return sendMessageAsync(msg);
        }

        /**
         * @brief Requests the Pixel to regularly send its measured RSSI value.
         * @param activate Whether to turn or turn off this feature.
         * @return A future with a boolean indicating whether the operation succeeded.
         */
        std::future<bool> reportRssiAsync(bool activate = true)
        {
            return reportRssiAsync(activate, std::chrono::seconds(5));
        }

        /**
         * @brief Requests the Pixel to turn itself off.
         * @return A future with a boolean indicating whether the operation succeeded.
         */
        std::future<bool> turnOffAsync();

        /**
         * @brief Requests the Pixel to blink.
         * @tparam Rep Duration arithmetic type representing the number of ticks.
         * @tparam Period Duration type representing the tick period.
         * @param duration Total duration of the animation.
         * @param rgbColor Blink color.
         * @param count Number of blinks.
         * @param fade Amount of in and out fading, 0: sharp transition, 1: maximum fading.
         * @return A future with a boolean indicating whether the operation succeeded.
         */
        template <class Rep, class Period>
        std::future<bool> blinkAsync(
            std::chrono::duration<Rep, Period> duration,
            uint32_t rgbColor,
            int count = 1,
            float fade = 1)
        {
            Messages::Blink msg{};
            down_cast(msg.count, count);
            down_cast(msg.duration, std::chrono::milliseconds{ duration }.count());
            msg.color = rgbColor;
            msg.faceMask = 0xFFFF;
            down_cast(msg.fade, 255 * fade);
            msg.loop = 0;
            // TODO return sendAndWaitForResponseAsync(msg, Messages::MessageType::BlinkAck);
            return sendMessageAsync(msg);
        }

    private:
        Pixel(const ScannedPixel& scannedPixel, std::shared_ptr<PixelDelegate> delegate);
        bool updateStatus(PixelStatus expectedStatus, PixelStatus newStatus, PixelStatus* outLastStatus = nullptr);
        std::future<ConnectResult> internalSetupAsync();
        void onValueChanged(const std::vector<uint8_t>& data);
        void processMessage(const Messages::PixelMessage& message);
        std::future<bool> sendMessageAsync(const std::vector<uint8_t>& data, bool withoutAck = false);

        template <typename T1, typename T2>
        static T1 down_cast(T1& dst, T2 src)
        {
            return dst = static_cast<T1>(std::min<T2>(src, std::numeric_limits<T1>::max()));
        }
    };
}