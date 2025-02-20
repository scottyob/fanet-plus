#pragma once

#include <cstdint>
#include <cstddef>
#include "etl/bit_stream.h"

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

        size_t encode(etl::bit_stream_writer &writer) const {
            writer.write_unchecked<uint8_t>(manufacturer);
            writer.write_unchecked<uint16_t>(etl::reverse_bytes<uint16_t>(device));
            return 3;
        }

        uint32_t toInt32() const {
            uint32_t ret = device;
            ret |= manufacturer << 16;
            return ret;
        }

        static Mac parse(const char *from)
        {
            Mac ret;
            ret.manufacturer = from[0];
            ret.device = from[1];
            ret.device |= from[2] << 8;
            return ret;
        }

        static Mac parse(etl::bit_stream_reader &reader) {
            Mac ret;
            ret.manufacturer = reader.read_unchecked<uint8_t>(8U);
            ret.device = reader.read_unchecked<uint8_t>(8U);
            ret.device |= reader.read_unchecked<uint8_t>(8U) << 8;
            return ret;
        }

        bool operator==(const Mac &other) const {
            return this->toInt32() == other.toInt32();
        }

        bool operator<(const Mac &other) const {
            return this->toInt32() < other.toInt32();
        }

        bool operator>(const Mac &other) const {
            return this->toInt32() > other.toInt32();
        }

        // Allow casting to uint32_t
        operator uint32_t() const {
            return this->toInt32();
        }
    };

}