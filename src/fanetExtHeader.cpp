#include "fanetExtHeader.h"

#include <etl/bit_stream.h>

using namespace Fanet;

size_t ExtendedHeader::parse(etl::bit_stream_reader &reader)
{
    ackType = (ExtendedHeaderAckType)reader.read_unchecked<uint8_t>(2U);
    auto hasDestinationMac = reader.read_unchecked<uint8_t>(1U);
    auto hasSignature = reader.read_unchecked<uint8_t>(1U);
    reader.skip(4U); // Reserved bits

    size_t size = 1;

    if (hasDestinationMac)
    {
        destinationMac = Mac::parse(reader);
        size += 3;
    }

    if (hasSignature)
    {
        // We don't support signatures, skip over these bytes
        size += 4;
        reader.skip(4);
    }

    return size;
}

size_t ExtendedHeader::encode(etl::bit_stream_writer &writer) const
{
    writer.write_unchecked<uint8_t>((int)ackType, 2U);
    writer.write_unchecked<bool>((bool)destinationMac.has_value(), 1U);
    // Write signature flag.  Library does not support it, so will always be 0
    writer.write<uint8_t>(0, 1U);
    writer.skip(4U); // Reserved bits
    size_t size = 8;

    if (destinationMac.has_value())
    {
        size += destinationMac.value().encode(writer);
    }

    return size;
}

bool Fanet::ExtendedHeader::operator==(const ExtendedHeader &other) const
{
    return ackType == other.ackType &&
           includesSignature == other.includesSignature &&
           destinationMac == other.destinationMac;
}
