#include "fanetGroundTracking.h"

size_t GroundTracking::encode(char *to) const
{
    etl::bit_stream_writer bit_stream((void*) to, (void*)(to + 256), etl::endian::big);
    // Encode the location
    location.toBitStream(bit_stream);

    // Write the ground type
    bit_stream.write_unchecked<uint8_t>((int)type, 4U);
    bit_stream.skip(3U); // Unused
    bit_stream.write_unchecked<uint8_t>(shouldTrackOnline, 1U);
    return 49;
}

GroundTracking GroundTracking::parse(const char *buffer, const size_t size)
{
    etl::bit_stream_reader bit_stream((void *)buffer, size, etl::endian::big);

    auto ret = GroundTracking();
    ret.location = Location::fromBitStream(bit_stream);
    ret.type = (GroundTrackingType)bit_stream.read_unchecked<uint8_t>(4U);
    bit_stream.skip(3U); // Unused
    ret.shouldTrackOnline = bit_stream.read_unchecked<uint8_t>(1U);

    return ret;
}
