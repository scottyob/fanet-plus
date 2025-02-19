#pragma once

#include <cstdint>
#include <cstddef>

namespace Fanet
{

    class Mac
    {
    public:
        /// @brief Manufacturer ID
        uint8_t manufacturer;

        /// @brief Device ID
        uint16_t device;

        size_t encode(char *to) const
        {
            to[0] = manufacturer;
            to[1] = (uint8_t)device;
            to[2] = device >> 8;
            return 3;
        }

        static Mac parse(const char *from)
        {
            Mac ret;
            ret.manufacturer = from[0];
            ret.device = from[1];
            ret.device |= from[2] << 8;
            return ret;
        }
    };

}