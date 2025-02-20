#pragma once
#include <stdint.h>
#include <stddef.h>
#include "fanetPayload.h"

namespace Fanet
{

    class Message : PacketPayloadBase
    {
    public:
        // Spec deems this subheader is TBD for future use.  0 is for "normal use"
        char subheader = 0;

        /// @brief Unicode message up to 244 bytes (assuming Fanet mac header of 11, + 256 bytes for max LoRa buffer)
        char message[244];

        bool operator==(const PacketPayloadBase &) const override;
        size_t parse(etl::bit_stream_reader &reader) override;
        size_t encode(etl::bit_stream_writer &writer) const override;
        PacketType getType() const override;
    };

}