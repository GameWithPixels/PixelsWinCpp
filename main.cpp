#include "pch.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>

#include "Systemic/Pixels/PixelScanner.h"
#include "Systemic/Pixels/Pixel.h"

using namespace Systemic::Pixels;
using namespace Systemic::Pixels::Messages;

using namespace std::chrono_literals;
using std::to_string;

std::string to_string(PixelStatus status)
{
    switch (status)
    {
    case PixelStatus::Disconnected:
        return "disconnected";
    case PixelStatus::Connecting:
        return "connecting";
    case PixelStatus::Identifying:
        return "identifying";
    case PixelStatus::Ready:
        return "ready";
    case PixelStatus::Disconnecting:
        return "disconnecting";
    default:
        return "";
    }
}

std::string serializeTimePoint(const std::chrono::system_clock::time_point& time, const std::string& format);
void printPixelMessage(std::shared_ptr<const PixelMessage> message);

// Pixel delegate that output Pixel events to the console
struct MyDelegate : PixelDelegate
{
    virtual void onStatusChanged(std::shared_ptr<Pixel> pixel, PixelStatus status) override
    {
        std::cout << "\nStatus changed to " + to_string(status);
    }

    virtual void onFirmwareDateChanged(std::shared_ptr<Pixel> pixel, std::chrono::system_clock::time_point firmwareDate) override
    {
        std::cout << "\nFirmware date changed to " + serializeTimePoint(firmwareDate, "%Y-%m-%d %H:%M:%S");
    }

    virtual void onRssiChanged(std::shared_ptr<Pixel> pixel, int rssi) override
    {
        std::cout << "\nRSSI changed to " + to_string(rssi) + "dBm";
    }

    virtual void onBatteryLevelChanged(std::shared_ptr<Pixel> pixel, int batteryLevel) override
    {
        std::cout << "\nBattery level changed to " + to_string(batteryLevel) + "%";
    }

    virtual void onChargingStateChanged(std::shared_ptr<Pixel> pixel, bool isCharging) override
    {
        std::cout << std::string{ "\nBattery is" } + (isCharging ? "" : " not") + " charging";
    }

    virtual void onRollStateChanged(std::shared_ptr<Pixel> pixel, PixelRollState state, int face) override
    {
        std::cout << "\nRoll state changed to " + to_string((int)state) + " with face " + to_string(face) + " up";
    }

    virtual void onRolled(std::shared_ptr<Pixel> pixel, int face) override
    {
        std::cout << "\nRolled on face " + to_string(face);
    }

    //virtual void onMessageReceived(std::shared_ptr<Pixel> pixel, std::shared_ptr<const PixelMessage> message) override
    //{
    //	printPixelMessage(message);
    //}
};

// Future that connects to a Pixels die and make it blink
std::future<void> connectAndBlink(std::shared_ptr<Pixel> pixel)
{
    std::cout << "\nConnecting...";

    // Attempts to connect
    auto result = co_await pixel->connectAsync();
    // Check result
    if (result != Pixel::ConnectResult::Success)
    {
        // If it failed, try a second time
        result = co_await pixel->connectAsync();
    }

    if (result == Pixel::ConnectResult::Success)
    {
        // We're connected!
        std::cout << "\nConnected and ready to use!";

        // Get RSSI notifications from die
        co_await pixel->reportRssiAsync();

        // Make die blink
        co_await pixel->blinkAsync(3s, 0xFF0000, 3);

        // Let's roll!
        std::cout << "\nRoll die to see results...";
    }
    else
    {
        // Couldn't connect
        std::cout << "\nConnection error: " + to_string((int)result);
    }
}

// Program entry point
int main()
{
    winrt::init_apartment();

    // Shared states
    std::mutex mutex{};
    std::shared_ptr<Pixel> pixel{};
    std::shared_ptr<MyDelegate> delegate{};
    std::thread connectThread{};

    // Print instructions
    std::cout << "Scanning for Pixels dice...\n";
    std::cout << "Once a Pixel is found, it will attempt to connect to it.\n";
    std::cout << "Press any key to exit.\n";

    // Prepare scanner instance
    PixelScanner scanner{ [&mutex, &pixel, &delegate, &connectThread](auto scannedPixel)
        {
            // We get notified each time the scanner gets advertisement data from a die

            // Print scanned Pixel info
            std::wcout << L"\nScanned Pixel: %ls" + scannedPixel->name();
            std::stringstream ss{}; // Use stream to output text all at once so it's not cut by calls from another thread
            ss << "\nId: " << std::hex << scannedPixel->pixelId() << std::dec;
            ss << "\nRSSI: " << scannedPixel->rssi();
            ss << "\nFirmware: " << serializeTimePoint(scannedPixel->firmwareDate(), "%Y-%m-%d %H:%M:%S");
            ss << "\nRoll state: " << (int)scannedPixel->rollState();
            ss << "\nCurrent face: " << (int)scannedPixel->currentFace();
            ss << "\nBattery level: " << (int)scannedPixel->batteryLevel() << "%";
            ss << "\nCharging: " << (scannedPixel->isCharging() ? "yes" : "no");
            std::cout << ss.str();

            // And connect to first Pixel it finds
            {
                std::lock_guard lock{ mutex };
                if (!pixel)
                {
                    // Create delegate and Pixel
                    delegate = std::shared_ptr<MyDelegate>{ new MyDelegate{} };
                    pixel = Pixel::create(*scannedPixel, delegate);

                    // And connect to it on a new thread so to not block the scanner
                    connectThread = std::thread([pixel]() { connectAndBlink(pixel).get(); });
                }
            }
        }
    };

    // Start scanning
    scanner.start();

    // Wait for user to press a key
    std::system("pause");
    std::cout << "\nBye!";
    if (connectThread.joinable())
    {
        connectThread.join();
    }
}

void printPixelMessage(std::shared_ptr<const PixelMessage> message)
{
    std::stringstream ss{};
    switch (message->type)
    {
    case MessageType::IAmADie: {
        const auto m = static_cast<const IAmADie*>(message.get());
        ss << "IAmADie => pixelId=" << std::hex << m->pixelId << std::dec
            << ", ledCount=" << (int)m->ledCount
            << ", battery=" << (int)m->batteryLevelPercent
            << "%, rollState=" << (int)m->rollState
            << ", face=" << ((int)m->currentFaceIndex + 1);
    } break;
    case MessageType::RollState: {
        const auto m = static_cast<const RollState*>(message.get());
        ss << "RollState => state=" << (int)m->state
            << ", face=" << ((int)m->faceIndex + 1);
    } break;
    case MessageType::BatteryLevel: {
        const auto m = static_cast<const BatteryLevel*>(message.get());
        ss << "BatteryLevel => levelPercent=" << (int)m->levelPercent
            << "%, state=" << (int)m->state;
    } break;
    case MessageType::Rssi: {
        const auto m = static_cast<const Rssi*>(message.get());
        ss << "RSSI => " << (int)m->value << " dBm";
    } break;
    default:
        ss << "message => type=" << getMessageName(message->type);
    }

    const auto s = ss.str();
    if (!s.empty())
    {
        std::cout << "\n>> Received message " + s;
    }
}

// https://stackoverflow.com/a/58523115
std::string serializeTimePoint(const std::chrono::system_clock::time_point& time, const std::string& format)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(time);
    //std::tm tm = *std::gmtime(&tt); //GMT (UTC)
    std::tm tm;
    localtime_s(&tm, &tt); //Locale time-zone, usually UTC by default.
    std::stringstream ss{};
    ss << std::put_time(&tm, format.c_str());
    return ss.str();
}
