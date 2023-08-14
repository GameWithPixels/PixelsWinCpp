/**
 * @file
 * @brief Global functions for accessing Bluetooth adapter state.
 */

#pragma once

namespace Systemic::BluetoothLE
{
    /// Bluetooth adapter states.
    enum class BleAdapterState
    {
        /// The system doesn't have a compatible Bluetooth adapter.
        Unsupported,

        /// The radio of the default Bluetooth adapter is in a uncontrollable state.
        Unavailable,

        /// The radio of the default Bluetooth adapter is disabled or powered off.
        Disabled,

        /// The radio of the default Bluetooth adapter is enabled and ready for use..
        Enabled
    };

    /**
     * @brief Returns the default Bluetooth adapter state.
     *
     * @return The adapter state
     */
    std::future<BleAdapterState> getAdapterStateAsync();

    /**
     * @brief Subscribe to the default Bluetooth adapter radio state events.
     *
     * @param onStateChanged Called when the radio state changes
     * @return A future indicating whether the operation was successful.
     */
    std::future<bool> subscribeAdapterStateChangedAsync(const std::function<void(BleAdapterState)>& onStateChanged);
}
