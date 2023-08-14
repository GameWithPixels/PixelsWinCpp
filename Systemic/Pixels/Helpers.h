/**
 * @file
 * @brief Some helper functions for Pixels.
 */

#pragma once

#include <cstdint>
#include <chrono>
#include "PixelTypes.h"

namespace Systemic::Pixels::Helpers
{
    /**
     * @brief Returns the die type based on the number of LEDs.
     * @param ledCount Number of LEDs on Pixels die.
     * @return The die type for the given number of LEDs.
     */
    inline DieType getDieType(int ledCount)
    {
        // For now we infer the die type from the number of LEDs, but eventually
        // that value will be part of identification data.
        switch (ledCount)
        {
        case 4:
            return DieType::D4;
        case 6:
            return DieType::D6;
        case 8:
            return DieType::D8;
        case 10:
            return DieType::D10;
        case 12:
            return DieType::D12;
        case 0: // Defaults unknown die to D20
        case 20:
            return DieType::D20;
        case 21:
            return DieType::D6Pipped;
        default:
            // Fudge has 6 LEDs actually, but let's use it as our default for now
            return DieType::D6Fudge;
        }
    }

    /**
     * @brief Returns the number of faces based on the die type.
     * @param dieType The Pixels die type.
     * @return The number of faces for the given die type.
     */
    inline int getFaceCount(DieType dieType)
    {
        switch (dieType)
        {
        default:
        case DieType::D20:
            return 20;
        case DieType::D12:
            return 12;
        case DieType::D10:
            return 10;
        case DieType::D8:
            return 8;
        case DieType::D6:
        case DieType::D6Pipped:
        case DieType::D6Fudge:
            return 6;
        case DieType::D4:
            return 4;
        }
    }

    /**
     * @brief
     * @param batteryState
     * @return
     */
    inline bool isPixelChargingOrDone(PixelBatteryState batteryState)
    {
        return batteryState == PixelBatteryState::Charging || batteryState == PixelBatteryState::Done;
    }

    /**
     * @brief Converts a UNIX timestamp in seconds to a `time_point`.
     * Use this function to get the date of a Pixels firmware from its timestamp.
     * @param buildTimestamp UNIX timestamp in seconds.
     * @return The `time_point` value for the given timestamp.
     */
    inline std::chrono::system_clock::time_point getFirmwareDate(uint32_t buildTimestamp)
    {
        return std::chrono::system_clock::time_point{ std::chrono::seconds{ buildTimestamp } };
    }
}
