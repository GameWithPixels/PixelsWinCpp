/**
 * @file
 * @brief Definition of the PixelInfo class.
 */

#pragma once

#include <string>
#include <chrono>
#include "Systemic/BluetoothLE/BleTypes.h"
#include "PixelTypes.h"

namespace Systemic::Pixels
{
    /// Type for a Bluetooth address.
    using bluetooth_address_t = Systemic::BluetoothLE::bluetooth_address_t;

    /// Common accessible values between Pixel advertised data and a connected Pixel.
    class PixelInfo
    {
    protected:
        PixelInfo() = default;
        virtual ~PixelInfo() = default;

    public:
        /// Type for a system Date.
        using Date = std::chrono::system_clock::time_point;

        /**
         * @brief Gets the unique id assigned by the OS to Pixel Bluetooth peripheral.
         * @note This member exist only to keep similar interface than other implementations,
         *       the Bluetooth address is returned.
         * @return The unique id for the Pixel.
         */
        virtual bluetooth_address_t systemId() const = 0;

        /**
         * @brief Gets the Bluetooth address of the Pixel.
         * @return The Bluetooth address of the Pixel.
         */
        virtual bluetooth_address_t address() const = 0;

        /**
         * @brief Gets the unique Pixel id of the die.
         * @return The unique Pixel id of the die.
        */
        virtual pixel_id_t pixelId() const = 0;

        /**
         * @brief Gets the Pixel name.
         * @return The Pixel name.
         */
        virtual const std::wstring& name() const = 0;

        /**
         * @brief Gets the number of LEDs of the Pixel.
         * @return The number of LEDs of the Pixel.
         */
        virtual int ledCount() const = 0;

        /**
         * @brief Gets the design and color of the Pixel.
         * @return The design and color of the Pixel.
         */
        virtual PixelDesignAndColor designAndColor() const = 0;

        /**
         * @brief Gets the firmware build date of the Pixel.
         * @return The firmware build date of the Pixel.
         */
        virtual Date firmwareDate() const = 0;

        /**
         * @brief Gets the last RSSI value measured by the Pixel.
         * @return The last RSSI value measured by the Pixel.
         */
        virtual int rssi() const = 0;

        /**
         * @brief Gets the Pixel battery level (percentage).
         * @return The Pixel battery level (percentage).
         */
        virtual int batteryLevel() const = 0;

        /**
         * @brief Indicates whether the Pixel battery is charging or not.
         *        Set to 'true' if fully charged but still on charger.
         * @return Whether the Pixel battery is charging or not.
         */
        virtual bool isCharging() const = 0;

        /**
         * @brief Gets the Pixel roll state.
         * @return The Pixel roll state.
         */
        virtual PixelRollState rollState() const = 0;

        /**
         * @brief Gets the Pixel face value that is currently facing up.
         * @return The Pixel face value that is currently facing up.
         */
        virtual int currentFace() const = 0;

        /**
         * @brief Gets die type of the Pixel.
         * @return The Pixel type.
         */
        DieType dieType();

        /**
         * @brief Gets the number of faces of the Pixel.
         * @return The number of faces of the Pixel.
         */
        int dieFaceCount();
    };
}
