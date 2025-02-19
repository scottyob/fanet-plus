#pragma once
#include <etl/bit_stream.h>

namespace Fanet
{

    /*
                                                                   0
           7       6       5       4       3       2       1       0
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    +                              (lsb)                            +
    |                            Latitude                           |
    +                              (msb)                            +
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    +                                                               +
    |                           Longitude                           |
    +                                                               +
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */
    /// @brief Represents and Encodes a Fanet Location
    class Location
    {
    public:
        // Value is parsed as raw_value / 93206 in Fanet+
        // (to resolve to -90 to +90)
        float latitude;

        // 24 bits on byte 3-5.
        // Value is parsed as raw_value / 46603 in Fanet+
        // (to resolve to -180 to +180)
        float longitude;

        /// @brief Parses location from bit stream, will read 48 bits.
        /// @return Location from parsed bit-stream
        static Location fromBitStream(etl::bit_stream_reader &reader);

        /// @brief Writes 48-bit data structure into Fanet+ bytes
        void toBitStream(etl::bit_stream_writer &writer) const;
    };

}