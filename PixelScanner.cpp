#include "pch.h"
#include "Systemic/Pixels/PixelScanner.h"

#include <cassert>
#include "Systemic/BluetoothLE/Scanner.h"
#include "Systemic/BluetoothLE/ScannedPeripheral.h"
#include "Systemic/Pixels/ScannedPixel.h"
#include "Systemic/Pixels/PixelBleUuids.h"
#include "Systemic/Pixels/Helpers.h"

namespace
{
    using namespace Systemic::Pixels;

    ScannedPixelData readScannedPixelData(std::shared_ptr<const Systemic::BluetoothLE::ScannedPeripheral> p)
    {
        ScannedPixelData data{};
        if (p)
        {
            auto& manufacturersData = p->manufacturersData();
            auto& servicesData = p->servicesData();
            if (!manufacturersData.empty() &&
                manufacturersData[0].data().size() >= 5 &&
                !servicesData.empty() &&
                servicesData[0].data().size() >= 8)
            {
                auto& manufData = manufacturersData[0].data();
                auto& servData = servicesData[0].data();

                struct
                {
                    uint32_t pixelId{};
                    uint32_t buildTimestamp{};
                } info1;
                memcpy(&info1, servData.data(), servData.size());

                struct
                {
                    uint8_t ledCount{};
                    PixelDesignAndColor designAndColor{};
                    PixelRollState rollState{};
                    uint8_t currentFaceIndex{};
                    uint8_t battery{};
                } info2;
                memcpy(&info2, manufData.data(), manufData.size());

                data.name = p->name();
                data.address = p->address();
                data.rssi = p->rssi();

                data.pixelId = info1.pixelId;
                data.firmwareDate = Helpers::getFirmwareDate(info1.buildTimestamp);

                data.ledCount = info2.ledCount;
                data.designAndColor = info2.designAndColor;
                data.rollState = info2.rollState;
                data.currentFace = info2.currentFaceIndex + 1;
                // MSB is battery charging
                data.batteryLevel = info2.battery & 0x7f;
                data.isCharging = (info2.battery & 0x80) > 0;
            }
        }
        return data;
    }
}

namespace Systemic::Pixels
{
    PixelScanner::PixelScanner(const ScannedPixelListener& listener)
        : _listener(listener)
    {
    }

    PixelScanner::~PixelScanner()
    {
        stop();
    }

    void PixelScanner::start()
    {
        std::lock_guard lock{ _mutex };

        _scanner.reset(new Systemic::BluetoothLE::Scanner
            {
                [this](auto p)
                {
                    auto data = readScannedPixelData(p);
                    if (data.pixelId)
                    {
                        const auto max = _scannedPixels.size();
                        size_t i = 0;
                        for (; i < max; ++i)
                        {
                            if (_scannedPixels[i]->pixelId() == data.pixelId)
                            {
                                break;
                            }
                        }

                        const auto pixel = std::shared_ptr<ScannedPixel>(new ScannedPixel{ data });
                        {
                            std::lock_guard lock{ _mutex };
                            if (i < max)
                            {
                                _scannedPixels[i] = pixel;
                            }
                            else
                            {
                                _scannedPixels.push_back(pixel);
                            }
                        }

                        if (_listener)
                        {
                            _listener(pixel);
                        }
                    }
                },
                {
                    PixelBleUuids::service
                }
            });
    }

    void PixelScanner::stop()
    {
        std::lock_guard lock{ _mutex };

        _scanner.reset();
    }

    void PixelScanner::clear()
    {
        std::lock_guard lock{ _mutex };

        _scannedPixels.clear();
    }
}
