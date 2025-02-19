#include "fanetLocation.h"

const float kLatitudeScaling = 93206;
const float kLongitudeScaling = 46603;

Location Location::fromBitStream(etl::bit_stream_reader &reader)
{
    auto ret = Location();
    // We we're storing the 24 bits in an int32_t, we'll need to shift them across the remaining 8 bits.
    ret.latitude = (etl::reverse_bytes(etl::read_unchecked<int32_t>(reader, 24U)) >> 8) / kLatitudeScaling;
    ret.longitude = (etl::reverse_bytes(etl::read_unchecked<int32_t>(reader, 24U)) >> 8) / kLongitudeScaling;
    return ret;

}

void Location::toBitStream(etl::bit_stream_writer &writer) const
{
    // Encode latitude and longitude
    int32_t lat_i = etl::reverse_bytes((int32_t)roundf(latitude * kLatitudeScaling)) >> 8;
    int32_t lon_i = etl::reverse_bytes((int32_t)roundf(longitude * kLongitudeScaling)) >> 8;
    etl::write_unchecked(writer, lat_i, 24U);
    etl::write_unchecked(writer, lon_i, 24U);
}
