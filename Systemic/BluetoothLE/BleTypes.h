/**
 * @file
 * @brief Common types used across the Systemic::BluetoothLE namespace.
 */

#pragma once

#include <cstdint>

//! \defgroup BluetoothLE
//! @brief A collection of C++ classes that provides a simplified access to Bluetooth Low Energy peripherals.
//!
//! @see Systemic::BluetoothLE namespace.

/**
 * @brief A collection of C++ classes that provides a simplified access to Bluetooth Low Energy peripherals.
 *
 * @note Some knowledge with Bluetooth Low Energy semantics is recommended for reading this documentation.
 *
 * WinRT APIs are used to access Bluetooth. It allows communicating with devices without needing
 * to first add them in Windows' Bluetooth devices manager.
 *
 * Requires at least Windows 10 version 1709 (Fall Creators Update).
 *
 * The Scanner class enables scanning for Bluetooth Low Energy peripherals.
 * It stores and notifies of discovered peripherals with ScannedPeripheral objects.
 *
 * The Peripheral class implements the most used BLE operation for communicating with BLE peripherals.
 * After a successful connection, services and characteristics are discovered and made accessible through
 * the Service and Characteristic classes.
 *
 * Below is a diagram of the main classes of this library:
 * @image html native-winrt.svg "Classes diagram"
 *
 * @ingroup BluetoothLE
 */
namespace Systemic::BluetoothLE
{
    /// Type for a Bluetooth address.
    using bluetooth_address_t = std::uint64_t;

    /// Peripheral requests statuses.
    enum class BleRequestStatus
    {
        /// The request completed successfully.
        Success,

        /// The request completed with a non-specific error.
        Error,

        /// The request is still in progress.
        InProgress,

        /// The request was canceled.
        Canceled,

        /// The request was aborted because the peripheral got disconnected.
        Disconnected, // TODO

        /// The request did not run because the given peripheral is not valid.
        InvalidPeripheral,

        /// The request did not run because the operation is not valid or permitted.
        InvalidCall,

        /// The request did not run because some of its parameters are invalid.
        InvalidParameters,

        /// The request failed because of the operation is not supported by the peripheral.
        NotSupported,

        /// The request failed because of BLE protocol error.
        ProtocolError,

        /// The request failed because it was denied access.
        AccessDenied,

        /// The request failed because the Bluetooth radio is off.
        AdapterOff, // TODO

        /// The request did not succeed after the timeout period.
        Timeout,
    };
}
