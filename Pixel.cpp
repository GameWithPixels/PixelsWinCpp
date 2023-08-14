#include "pch.h"
#include "Systemic/Pixels/Pixel.h"

#include "Systemic/BluetoothLE/Peripheral.h"
#include "Systemic/BluetoothLE/Characteristic.h"
#include "Systemic/BluetoothLE/Service.h"
#include "Systemic/Pixels/Helpers.h"
#include "Systemic/Pixels/PixelBleUuids.h"

using namespace Systemic::BluetoothLE;

namespace Systemic::Pixels
{
    Pixel::Pixel(const ScannedPixel& scannedPixel, std::shared_ptr<PixelDelegate> delegate)
        : _data(scannedPixel.data)
        , _peripheral(Peripheral::create(scannedPixel.data.address, [this](ConnectionEvent ev, ConnectionEventReason /*reason*/)
            {
                std::lock_guard lock{ _mutex };

                switch (ev)
                {
                case ConnectionEvent::Connecting:
                    _status = PixelStatus::Connecting;
                    break;
                case ConnectionEvent::Disconnecting:
                    _status = PixelStatus::Disconnecting;
                    break;
                case ConnectionEvent::Disconnected:
                case ConnectionEvent::FailedToConnect:
                    _status = PixelStatus::Disconnected;
                    break;
                case ConnectionEvent::Connected:
                case ConnectionEvent::Ready:
                    // Nothing
                    break;
                }
            }))
        , _delegate(delegate)
    {
    }

            std::future<Pixel::ConnectResult> Pixel::connectAsync()
            {
                auto result = ConnectResult::Success;

                try
                {
                    const auto connectStatus = co_await _peripheral->connectAsync({ PixelBleUuids::service });

                    if (connectStatus == BleRequestStatus::Success)
                    {
                        PixelStatus prevStatus{};
                        if (updateStatus(PixelStatus::Connecting, PixelStatus::Identifying, &prevStatus))
                        {
                            result = co_await internalSetupAsync();

                            if (result == ConnectResult::Success)
                            {
                                updateStatus(PixelStatus::Identifying, PixelStatus::Ready);
                            }
                            else
                            {
                                disconnect();
                            }
                        }
                        else if (_status == PixelStatus::Identifying)
                        {
                            std::promise<PixelStatus> statusPromise{};

                            StatusCallback callback{ [&statusPromise](auto status)
                                {
                                    statusPromise.set_value(status);
                                } };

                            const auto cbIndex = _internalStatusCbs.add(callback);

                            co_await statusPromise.get_future();

                            _internalStatusCbs.remove(cbIndex);
                        }
                    }
                    else
                    {
                        result = ConnectResult::ConnectionFailed;
                    }

                    if (result == ConnectResult::Success && _status != PixelStatus::Ready) // No lock needed
                    {
                        result = ConnectResult::Cancelled;
                    }

                    co_return result;
                }
                catch (...)
                {
                    // This is a safeguard in case the above code throws an exception (which shouldn't happen)
                    try
                    {
                        disconnect();
                    }
                    catch (...)
                    {
                    }
                    throw;
                }
            }

            void Pixel::disconnect()
            {
                _peripheral->disconnect();
            }

            std::future<bool> Pixel::turnOffAsync()
            {
                return sendMessageAsync(Messages::MessageType::Sleep, true); // withoutAck
            }

            //
            // Private methods
            //

            bool Pixel::updateStatus(PixelStatus expectedStatus, PixelStatus newStatus, PixelStatus* outLastStatus /*= nullptr*/)
            {
                bool update = false;
                {
                    std::lock_guard lock{ _mutex };

                    if (outLastStatus)
                    {
                        *outLastStatus = _status;
                    }

                    update = _status == expectedStatus;
                    if (update)
                    {
                        _status = newStatus;
                    }
                }

                if (update)
                {
                    std::vector<StatusCallback> callbacks = _internalStatusCbs.get();

                    for (const auto& cb : callbacks)
                    {
                        if (cb)
                        {
                            cb(newStatus);
                        }
                    }

                    if (_delegate)
                    {
                        _delegate->onStatusChanged(shared_from_this(), newStatus);
                    }
                }

                return update;
            }

            std::future<Pixel::ConnectResult> Pixel::internalSetupAsync()
            {
                ConnectResult result = ConnectResult::Success;

                auto service = _peripheral->getDiscoveredService(PixelBleUuids::service);
                if (service)
                {
                    auto notify = service->getCharacteristic(PixelBleUuids::notifyCharacteristic);
                    auto write = service->getCharacteristic(PixelBleUuids::writeCharacteristic);
                    if (notify && write)
                    {
                        const auto status = co_await notify->subscribeAsync([this](auto data)
                            {
                                onValueChanged(data);
                            });

                        if (status == BleRequestStatus::Success)
                        {
                            {
                                std::lock_guard lock{ _mutex };

                                _notifyCharacteristic = notify;
                                _writeCharacteristic = write;
                            }

                            const auto iAmADie = std::static_pointer_cast<const Messages::IAmADie>(
                                co_await sendAndWaitForResponseAsync(
                                    Messages::MessageType::WhoAreYou,
                                    Messages::MessageType::IAmADie,
                                    std::chrono::seconds(2))
                            );
                            if (!iAmADie)
                            {
                                result = ConnectResult::IdentificationTimeout;
                            }
                            else if (iAmADie->pixelId != _data.pixelId)
                            {
                                result = ConnectResult::IdentificationMismatch;
                            }
                        }
                        else
                        {
                            result = ConnectResult::SubscriptionError;
                        }
                    }
                }

                co_return result;
            }

            void Pixel::onValueChanged(const std::vector<uint8_t>& data)
            {
                const auto msg = Messages::Serialization::deserializeMessage(data);
                if (msg)
                {
                    processMessage(*msg);

                    std::vector<MessageCallback> callbacks = _internalMsgCbs.get();

                    for (const auto& cb : callbacks)
                    {
                        if (cb)
                        {
                            cb(msg);
                        }
                    }

                    if (_delegate)
                    {
                        _delegate->onMessageReceived(shared_from_this(), msg);
                    }
                }
            }

            void Pixel::processMessage(const Messages::PixelMessage& message)
            {
                switch (message.type)
                {
                case Messages::MessageType::IAmADie:
                {
                    const auto& iAmADie = static_cast<const Messages::IAmADie&>(message);
                    if (!_data.pixelId || iAmADie.pixelId == _data.pixelId)
                    {
                        // Update read only properties (atomic writes, no lock)
                        _data.ledCount = iAmADie.ledCount;
                        _data.designAndColor = iAmADie.designAndColor;
                        _data.pixelId = iAmADie.pixelId;

                        // Update notifiable properties

                        // Skip sending roll state to delegate as we didn't get the data
                        // from an actual roll event
                        _data.rollState = iAmADie.rollState;
                        _data.currentFace = iAmADie.currentFaceIndex + 1;

                        const auto firmwareDate = Helpers::getFirmwareDate(iAmADie.buildTimestamp);
                        const bool dateChanged = _data.firmwareDate != firmwareDate;
                        _data.firmwareDate = firmwareDate;
                        if (_delegate && dateChanged)
                        {
                            _delegate->onFirmwareDateChanged(shared_from_this(), firmwareDate);
                        }

                        const auto level = iAmADie.batteryLevelPercent;
                        const bool levelChanged = _data.batteryLevel != level;
                        _data.batteryLevel = iAmADie.batteryLevelPercent;
                        if (_delegate && levelChanged)
                        {
                            _delegate->onBatteryLevelChanged(shared_from_this(), level);
                        }

                        const bool isCharging = Helpers::isPixelChargingOrDone(iAmADie.batteryState);
                        const bool chargingChanged = _data.isCharging != isCharging;
                        _data.isCharging = isCharging;
                        if (_delegate && chargingChanged)
                        {
                            _delegate->onChargingStateChanged(shared_from_this(), isCharging);
                        }
                    }
                    break;
                }

                case Messages::MessageType::RollState:
                {
                    const auto& roll = static_cast<const Messages::RollState&>(message);

                    // Update properties
                    _data.rollState = roll.state;
                    _data.currentFace = roll.faceIndex + 1;

                    if (_delegate)
                    {
                        // Always notify delegate of roll events
                        _delegate->onRollStateChanged(shared_from_this(), roll.state, roll.faceIndex + 1);

                        if (roll.state == PixelRollState::OnFace)
                        {
                            _delegate->onRolled(shared_from_this(), roll.faceIndex + 1);
                        }
                    }
                    break;
                }

                case Messages::MessageType::BatteryLevel:
                {
                    const auto& batteryLevel = static_cast<const Messages::BatteryLevel&>(message);

                    const bool levelChanged = _data.batteryLevel != batteryLevel.levelPercent;
                    const bool isCharging = Helpers::isPixelChargingOrDone(batteryLevel.state);
                    const bool chargingChanged = _data.isCharging != isCharging;

                    _data.batteryLevel = batteryLevel.levelPercent;
                    if (_delegate && levelChanged)
                    {
                        _delegate->onBatteryLevelChanged(shared_from_this(), batteryLevel.levelPercent);
                    }

                    _data.isCharging = isCharging;
                    if (_delegate && chargingChanged)
                    {
                        _delegate->onChargingStateChanged(shared_from_this(), isCharging);
                    }
                    break;
                }

                case Messages::MessageType::Rssi:
                {
                    const auto& rssi = static_cast<const Messages::Rssi&>(message);

                    const bool rssiChanged = _data.rssi != rssi.value;

                    _data.rssi = rssi.value;
                    if (_delegate && rssiChanged)
                    {
                        _delegate->onRssiChanged(shared_from_this(), rssi.value);
                    }
                    break;
                }
                }
            }

            std::future<bool> Pixel::sendMessageAsync(const std::vector<uint8_t>& data, bool withoutAck /*= false*/)
            {
                const auto result = co_await _writeCharacteristic->writeAsync(data, withoutAck);
                co_return result == BleRequestStatus::Success;
            }
}
