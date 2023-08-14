#include "pch.h"
#include "Systemic/Pixels/PixelInfo.h"

#include "Systemic/Pixels/Helpers.h"

namespace Systemic::Pixels
{
    DieType PixelInfo::dieType()
    {
        return Helpers::getDieType(ledCount());
    }

    int PixelInfo::dieFaceCount()
    {
        return Helpers::getFaceCount(dieType());
    }
}
