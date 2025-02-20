#pragma once

#include "fanetPayload.h"

namespace Fanet
{

    class Ack : PacketPayloadBase
    {
        bool operator==(const PacketPayloadBase &other) const override
        {
            // If the payload of the other packet is not an Ack, it can't be equal
            if (!dynamic_cast<const Ack *>(&other))
            {
                return false;
            }

            // Otherwise, yeah, we're equal
            return true;
        }
        size_t parse(etl::bit_stream_reader &reader) override {
            return 0;
        }
        size_t encode(etl::bit_stream_writer &writer) const override {
            return 0;
        }
        PacketType getType() const { return PacketType::Ack; }
    }; // Empty payload

}