/**
 * @file
 * @brief List of Pixel messages and their respective classes.
 */

#pragma once

#include <cstdint>
#include "PixelTypes.h"

namespace Systemic::Pixels::Messages
{
    /**
     * @brief Lists all the Pixel dice message types.
     * The value is used for the first byte of data in a Pixel message to identify it's type.
     * These message identifiers have to match up with the ones on the firmware.
     */
    enum class MessageType : uint8_t
    {
        None = 0,
        WhoAreYou,
        IAmADie,
        RollState,
        Telemetry,
        BulkSetup,
        BulkSetupAck,
        BulkData,
        BulkDataAck,
        TransferAnimSet,
        TransferAnimSetAck,
        TransferAnimSetFinished,
        TransferSettings,
        TransferSettingsAck,
        TransferSettingsFinished,
        TransferTestAnimSet,
        TransferTestAnimSetAck,
        TransferTestAnimSetFinished,
        DebugLog,
        PlayAnim,
        PlayAnimEvent,
        StopAnim,
        RemoteAction,
        RequestRollState,
        RequestAnimSet,
        RequestSettings,
        RequestTelemetry,
        ProgramDefaultAnimSet,
        ProgramDefaultAnimSetFinished,
        Blink,
        BlinkAck,
        RequestDefaultAnimSetColor,
        DefaultAnimSetColor,
        RequestBatteryLevel,
        BatteryLevel,
        RequestRssi,
        Rssi,
        Calibrate,
        CalibrateFace,
        NotifyUser,
        NotifyUserAck,
        TestHardware,
        TestLedLoopback,
        LedLoopback,
        SetTopLevelState,
        ProgramDefaultParameters,
        ProgramDefaultParametersFinished,
        SetDesignAndColor,
        SetDesignAndColorAck,
        SetCurrentBehavior,
        SetCurrentBehaviorAck,
        SetName,
        SetNameAck,
        Sleep,
        ExitValidation,
        TransferInstantAnimSet,
        TransferInstantAnimSetAck,
        TransferInstantAnimSetFinished,
        PlayInstantAnim,
        StopAllAnims,
        RequestTemperature,
        Temperature,
        EnableCharging,
        DisableCharging,
        Discharge,
        BlinkId,
        BlinkIdAck,
    };

#pragma pack(push, 1)

    /*
     * Base type for all Pixel messages.
     * May also represent any message with no data.
     */
    struct PixelMessage
    {
        /// Message type.
        const MessageType type;

        /// Initializes a new instance of PixelMessage with the given type.
        PixelMessage(MessageType msgType) : type(msgType) {}
    };

    /// Message send by a Pixel after receiving a "WhoAmI" message.
    struct IAmADie
        : public PixelMessage
    {
        /// Number of LEDs.
        uint8_t ledCount{};

        /// Die look.
        PixelDesignAndColor designAndColor{};

    private:
        uint8_t _padding{};
    public:

        /// Hash of the uploaded profile.
        uint32_t dataSetHash{};

        /// The Pixel id.
        uint32_t pixelId{};

        /// Amount of available flash.
        uint16_t availableFlash{};

        /// UNIX timestamp in seconds for the date of the firmware.
        uint32_t buildTimestamp{};

        // Roll state

        /// Current roll state.
        PixelRollState rollState{};

        /// Face index (if applicable), starts at 0.
        uint8_t currentFaceIndex{};

        // Battery level

        /// The battery charge level in percent.
        uint8_t batteryLevelPercent{};

        /// The charging state of the battery.
        PixelBatteryState batteryState{};

        /// Initializes a new instance of IAmADie.
        IAmADie() : PixelMessage(MessageType::IAmADie) {}
    };

    /// Message send by a Pixel to notify of its rolling state.
    struct RollState
        : public PixelMessage
    {
        /// Current roll state.
        PixelRollState state{};

        /// Index of the face facing up (if applicable).
        uint8_t faceIndex{};

        /// Initializes a new instance of RollState.
        RollState() : PixelMessage(MessageType::RollState) {}
    };

    /// Message send to a Pixel to have it blink its LEDs.
    struct Blink
        : public PixelMessage
    {
        /// Number of flashes.
        uint8_t count{};

        /// Total duration in milliseconds.
        uint16_t duration{};

        /// Color to blink.
        uint32_t color{};

        /// Select which faces to light up.
        uint32_t faceMask{};

        /// Amount of in and out fading, 0: sharp transition, 255: max fading.
        uint8_t fade{};

        /// Whether to indefinitely loop the animation.
        uint8_t loop{};

        /// Initializes a new instance of Blink.
        Blink() : PixelMessage(MessageType::Blink) {}
    };

    /// Message send by a Pixel to notify of its battery level and state.
    struct BatteryLevel
        : public PixelMessage
    {
        /// The battery charge level in percent.
        uint8_t levelPercent{};

        /// The charging state of the battery.
        PixelBatteryState state{};

        /// Initializes a new instance of BatteryLevel.
        BatteryLevel() : PixelMessage(MessageType::BatteryLevel) {}
    };

    /// Available modes for telemetry requests.
    enum class TelemetryRequestMode : uint8_t
    {
        /// Request Pixel to stop automatically sending telemetry updates.
        Off,

        /// Request Pixel to immediately send a single telemetry update.
        Once,

        /// Request Pixel to automatically send telemetry updates.
        Automatic,
    };

    /// Message send to a Pixel to configure RSSI reporting.
    struct RequestRssi
        : public PixelMessage
    {
        /// Telemetry mode used for sending the RSSI update(s).
        TelemetryRequestMode requestMode{};

        /// Minimum interval in milliseconds between two updates (0 for no cap on rate).
        uint16_t minInterval{};

        /// Initializes a new instance of RequestRssi.
        RequestRssi() : PixelMessage(MessageType::RequestRssi) {}
    };

    /// Message send by a Pixel to notify of its measured RSSI.
    struct Rssi
        : public PixelMessage
    {
        /// The RSSI value, in dBm.
        int8_t value{};

        /// Initializes a new instance of Rssi.
        Rssi() : PixelMessage(MessageType::Rssi) {}
    };

#pragma pack(pop)

    /**
     * @brief Returns the name of a Pixel message.
     * @param type The message type to get the name for.
     * @return The name of the message represented by given message type.
    */
    inline const char* getMessageName(MessageType type)
    {
        switch (type)
        {
        case MessageType::None:
            return "None";
        case MessageType::WhoAreYou:
            return "WhoAreYou";
        case MessageType::IAmADie:
            return "IAmADie";
        case MessageType::RollState:
            return "RollState";
        case MessageType::Telemetry:
            return "Telemetry";
        case MessageType::BulkSetup:
            return "BulkSetup";
        case MessageType::BulkSetupAck:
            return "BulkSetupAck";
        case MessageType::BulkData:
            return "BulkData";
        case MessageType::BulkDataAck:
            return "BulkDataAck";
        case MessageType::TransferAnimSet:
            return "TransferAnimSet";
        case MessageType::TransferAnimSetAck:
            return "TransferAnimSetAck";
        case MessageType::TransferAnimSetFinished:
            return "TransferAnimSetFinished";
        case MessageType::TransferSettings:
            return "TransferSettings";
        case MessageType::TransferSettingsAck:
            return "TransferSettingsAck";
        case MessageType::TransferSettingsFinished:
            return "TransferSettingsFinished";
        case MessageType::TransferTestAnimSet:
            return "TransferTestAnimSet";
        case MessageType::TransferTestAnimSetAck:
            return "TransferTestAnimSetAck";
        case MessageType::TransferTestAnimSetFinished:
            return "TransferTestAnimSetFinished";
        case MessageType::DebugLog:
            return "DebugLog";
        case MessageType::PlayAnim:
            return "PlayAnim";
        case MessageType::PlayAnimEvent:
            return "PlayAnimEvent";
        case MessageType::StopAnim:
            return "StopAnim";
        case MessageType::RemoteAction:
            return "RemoteAction";
        case MessageType::RequestRollState:
            return "RequestRollState";
        case MessageType::RequestAnimSet:
            return "RequestAnimSet";
        case MessageType::RequestSettings:
            return "RequestSettings";
        case MessageType::RequestTelemetry:
            return "RequestTelemetry";
        case MessageType::ProgramDefaultAnimSet:
            return "ProgramDefaultAnimSet";
        case MessageType::ProgramDefaultAnimSetFinished:
            return "ProgramDefaultAnimSetFinished";
        case MessageType::Blink:
            return "Blink";
        case MessageType::BlinkAck:
            return "BlinkAck";
        case MessageType::RequestDefaultAnimSetColor:
            return "RequestDefaultAnimSetColor";
        case MessageType::DefaultAnimSetColor:
            return "DefaultAnimSetColor";
        case MessageType::RequestBatteryLevel:
            return "RequestBatteryLevel";
        case MessageType::BatteryLevel:
            return "BatteryLevel";
        case MessageType::RequestRssi:
            return "RequestRssi";
        case MessageType::Rssi:
            return "Rssi";
        case MessageType::Calibrate:
            return "Calibrate";
        case MessageType::CalibrateFace:
            return "CalibrateFace";
        case MessageType::NotifyUser:
            return "NotifyUser";
        case MessageType::NotifyUserAck:
            return "NotifyUserAck";
        case MessageType::TestHardware:
            return "TestHardware";
        case MessageType::TestLedLoopback:
            return "TestLedLoopback";
        case MessageType::LedLoopback:
            return "LedLoopback";
        case MessageType::SetTopLevelState:
            return "SetTopLevelState";
        case MessageType::ProgramDefaultParameters:
            return "ProgramDefaultParameters";
        case MessageType::ProgramDefaultParametersFinished:
            return "ProgramDefaultParametersFinished";
        case MessageType::SetDesignAndColor:
            return "SetDesignAndColor";
        case MessageType::SetDesignAndColorAck:
            return "SetDesignAndColorAck";
        case MessageType::SetCurrentBehavior:
            return "SetCurrentBehavior";
        case MessageType::SetCurrentBehaviorAck:
            return "SetCurrentBehaviorAck";
        case MessageType::SetName:
            return "SetName";
        case MessageType::SetNameAck:
            return "SetNameAck";
        case MessageType::Sleep:
            return "Sleep";
        case MessageType::ExitValidation:
            return "ExitValidation";
        case MessageType::TransferInstantAnimSet:
            return "TransferInstantAnimSet";
        case MessageType::TransferInstantAnimSetAck:
            return "TransferInstantAnimSetAck";
        case MessageType::TransferInstantAnimSetFinished:
            return "TransferInstantAnimSetFinished";
        case MessageType::PlayInstantAnim:
            return "PlayInstantAnim";
        case MessageType::StopAllAnims:
            return "StopAllAnims";
        case MessageType::RequestTemperature:
            return "RequestTemperature";
        case MessageType::Temperature:
            return "Temperature";
        case MessageType::EnableCharging:
            return "EnableCharging";
        case MessageType::DisableCharging:
            return "DisableCharging";
        case MessageType::Discharge:
            return "Discharge";
        case MessageType::BlinkId:
            return "BlinkId";
        case MessageType::BlinkIdAck:
            return "BlinkIdAck";
        }
        return "";
    }
};
