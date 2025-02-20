#pragma once

#include "fanetPayload.h"
#define MAX_NAME_SIZE 245
#include "etl/string.h"

namespace Fanet
{

    class Name : public PacketPayloadBase
    {
    public:
        etl::string<MAX_NAME_SIZE> name = {0};

        virtual bool operator==(const PacketPayloadBase &other) const override;
        virtual size_t parse(etl::bit_stream_reader &reader) override;
        virtual size_t encode(etl::bit_stream_writer &writer) const override;
        virtual PacketType getType() const override {
            return PacketType::Name;
        }
    };

}