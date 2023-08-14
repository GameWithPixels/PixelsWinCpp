/**
 * @file
 * @brief Pixel's Bluetooth UUIDs.
 */

#pragma once

#include <winrt/Windows.Foundation.h>

/// Bluetooth UUIDs related to Pixels peripherals.
namespace Systemic::Pixels::PixelBleUuids
{
    /// Pixel dice service UUID.
    extern const winrt::guid service;

    /// Pixel dice write characteristic UUID.
    extern const winrt::guid notifyCharacteristic;

    /// Pixel dice write characteristic UUID.
    extern const winrt::guid writeCharacteristic;
}
