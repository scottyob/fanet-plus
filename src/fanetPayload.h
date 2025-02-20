#pragma once

#include "etl/bit_stream.h"
#include "fanetHeader.h"

namespace Fanet
{

    /// @brief Base class all Payload types must support
    class PacketPayloadBase
    {
    public:
        virtual bool operator==(const PacketPayloadBase &) const = 0;
        virtual size_t parse(etl::bit_stream_reader &reader) = 0;
        virtual size_t encode(etl::bit_stream_writer &writer) const = 0;
        virtual PacketType getType() const = 0;
    };
}