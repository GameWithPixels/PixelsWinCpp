/**
 * @file
 * @brief Definition of the PixelScanner class.
 */

#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <mutex>

namespace Systemic::BluetoothLE
{
    class Scanner;
    class ScannedPeripheral;
}

namespace Systemic::Pixels
{
    class ScannedPixel;

    /**
     * @brief Represents a Bluetooth scanner for Pixels dice.
     *
     * This class is thread safe.
     */
    class PixelScanner
    {
    public:
        /// Signature of a scanned Pixel listener.
        using ScannedPixelListener = std::function<void(const std::shared_ptr<const ScannedPixel>&)>;

    private:
        // Listener given by user
        const ScannedPixelListener _listener;
        // Bluetooth scanner instance
        std::shared_ptr<Systemic::BluetoothLE::Scanner> _scanner{};
        // List of scanned pixels
        std::vector<std::shared_ptr<const ScannedPixel>> _scannedPixels{};

        // Mutex used to modify list of scanned Pixels
        std::recursive_mutex _mutex{};

    public:
        /**
         * @brief Initializes a new instance of PixelScanner with the given listener.
         * @param listener A function to be called upon each Pixel advertisement packet
                           received by the scanner.
         * @note The given listener should not block the thread and completes its operation quickly.
         */
        explicit PixelScanner(const ScannedPixelListener& listener);

        /// Default destructor.
        ~PixelScanner();

        /// Indicates whether a scan for Pixels dice is currently running.
        bool isScanning()
        {
            return _scanner != nullptr;
        }

        /**
         * @brief Copy the list of scanned Pixels to the given std::vector.
         *
         * @param outScannedPixels The std::vector to which the scanned Pixels are copied (appended).
         */
        void copyScannedPixels(std::vector<std::shared_ptr<const ScannedPixel>>& outScannedPixels)
        {
            std::lock_guard lock{ _mutex };

            outScannedPixels.reserve(outScannedPixels.size() + _scannedPixels.size());
            for (auto& p : _scannedPixels)
            {
                outScannedPixels.emplace_back(p);
            }
        }

        /// Starts a Bluetooth scan for Pixels.
        void start();

        /// Stops scanning for Pixels.
        void stop();

        /// Clear the list of scanned Pixels.
        void clear();
    };
}
