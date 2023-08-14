/**
 * @file
 * @brief Common types used across the Systemic::Pixel namespace.
 */

#pragma once

#include <cstdint>

//! \defgroup Pixels
//! @brief A collection of C++ classes and types to scan for and connect to Pixels dice.
//!
//! @see Systemic::Pixels namespace.

/**
 * @brief A collection of C++ classes and types to scan for and connect to Pixels dice.
 *
 * @ingroup Pixels
 */
namespace Systemic::Pixels
{
    /// Type for a Pixel Id.
    using pixel_id_t = uint32_t;

    /// Available combinations of Pixel designs and colors.
    enum class PixelDesignAndColor : uint8_t
    {
        Unknown,
        Generic,
        V3Orange,
        V4BlackClear,
        V4WhiteClear,
        V5Grey,
        V5White,
        V5Black,
        V5Gold,
        OnyxBlack,
        HematiteGrey,
        MidnightGalaxy,
        AuroraSky,
    };

    /// Pixel roll states.
    enum class PixelRollState : uint8_t
    {
        /// The Pixel roll state could not be determined.
        Unknown,

        /// The Pixel is resting in a position with a face up.
        OnFace,

        /// The Pixel is being handled.
        Handling,

        /// The Pixel is rolling.
        Rolling,

        /// The Pixel is resting in a crooked position.
        Crooked,
    };

    /// The different possible battery charging states.
    enum class PixelBatteryState : uint8_t
    {
        /// Battery looks fine, nothing is happening.
        Ok,

        /// Battery level is low, notify user they should recharge.
        Low,

        /// Battery is currently recharging.
        Charging,

        /// Battery is full and finished charging.
        Done,

        /// Coil voltage is bad, die is probably positioned incorrectly.
        /// Note that currently this state is triggered during transition between charging and not charging...
        BadCharging,

        /// Charge state doesn't make sense (charging but no coil voltage detected for instance).
        Error,
    };

    /// The different types of dice.
    enum class DieType
    {
        D20,
        D12,
        D10,
        D8,
        D6,
        D6Pipped,
        D6Fudge,
        D4,
    };
}
