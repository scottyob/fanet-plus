#include "fanetExtHeader.h"

#include <etl/bit_stream.h>

using namespace Fanet;


ExtendedHeader ExtendedHeader::parse(const char *from, size_t &size)
{
    auto ret = ExtendedHeader();

    etl::bit_stream_reader reader((void *)from, (void *)(from + kExtendedHeaderMaxSize), etl::endian::big);

    ret.ackType = (ExtendedHeaderAckType) reader.read_unchecked<uint8_t>(2U);
    auto hasDestinationMac = reader.read_unchecked<uint8_t>(1U);
    auto hasSignature = reader.read_unchecked<uint8_t>(1U);
    reader.skip(4U); // Reserved bits

    size = 1;

    if(hasDestinationMac) {
        reader.skip(3); // Skips the destination address for mac address
        ret.destinationMac = Mac::parse(&from[1]);
        size += 3;
    }

    if(hasSignature) {
        // We don't support signatures, skip over these bytes
        size += 4;
    }

    return ExtendedHeader();
}

size_t ExtendedHeader::encode(char *to) const
{
    etl::bit_stream_writer writer((void *)to, (void *)(to + kExtendedHeaderMaxSize), etl::endian::big);

    writer.write_unchecked<uint8_t>((int)ackType, 2U);
    writer.write_unchecked<bool>((bool)destinationMac.has_value(), 1U);
    // Write signature flag.  Library does not support it, so will always be 0
    writer.write<uint8_t>(0, 1U);
    writer.skip(4U); // Reserved bits
    size_t size = 8;

    if(destinationMac.has_value()) {
        writer.skip(8 * 3); // Skip the next 3 bytes mac address
        destinationMac.value().encode(&to[1]);
        size += 3;
    }

    return size;
}
