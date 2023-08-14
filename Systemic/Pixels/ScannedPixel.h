/**
 * @file
 * @brief Definition of the ScannedPixel class.
 */

#pragma once

#include <string>
#include "PixelInfo.h"

namespace Systemic::Pixels
{
    /**
     * @brief Data for ScannedPixel class.
     * @note: All the fields are written atomically
     */
    struct ScannedPixelData
    {
        /// Type for a system Date.
        using Date = PixelInfo::Date;

        /// The Bluetooth address for the Pixel.
        bluetooth_address_t address{};

        /// The unique Pixel id of the device.
        pixel_id_t pixelId{};

        /// The Pixel name.
        std::wstring name{};

        /// The number of LEDs of the Pixel.
        int ledCount{};

        /// The Pixel design and color.
        PixelDesignAndColor designAndColor{};

        /// The firmware build date of the Pixel.
        Date firmwareDate{};

        /// The last RSSI value measured by this Pixel.
        int rssi{};

        /// The Pixel battery level (percentage).
        int batteryLevel{};

        /// Whether the Pixel battery is charging or not.
        /// Set to 'true' if fully charged but still on charger.
        bool isCharging{};

        /// The Pixel roll state.
        PixelRollState rollState{};

        /// The Pixel face value that is currently facing up.
        int currentFace{};
    };

    /// Data periodically emitted by a Pixel when not connected to a device.
    class ScannedPixel : public PixelInfo
    {
    public:
        /// Initializes a new instance of ScannedPixel with the given data.
        explicit ScannedPixel(const ScannedPixelData& data)
            : data(data)
        {
        }

        /// Default virtual destructor.
        virtual ~ScannedPixel() = default;

        /// The raw data of the instance.
        ScannedPixelData data{};

        virtual bluetooth_address_t systemId() const override
        {
            return data.address;
        }

        virtual bluetooth_address_t address() const override
        {
            return data.address;
        }

        virtual pixel_id_t pixelId() const override
        {
            return data.pixelId;
        }

        virtual const std::wstring& name() const override
        {
            return data.name;
        }

        virtual int ledCount() const override
        {
            return data.ledCount;
        }

        virtual PixelDesignAndColor designAndColor() const override
        {
            return data.designAndColor;
        }

        virtual Date firmwareDate() const override
        {
            return data.firmwareDate;
        }

        virtual int rssi() const override
        {
            return data.rssi;
        }

        virtual int batteryLevel() const override
        {
            return data.batteryLevel;
        }

        virtual bool isCharging() const override
        {
            return data.isCharging;
        }

        virtual PixelRollState rollState() const override
        {
            return data.rollState;
        }

        virtual int currentFace() const override
        {
            return data.currentFace;
        }
    };
}