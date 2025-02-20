#pragma once
#include <stdint.h>
#include <stddef.h>
#include "fanetLocation.h"
#include "fanetPayload.h"

namespace Fanet
{

    enum class GroundTrackingType : uint8_t
    {
        Other = 0,
        Walking = 1,
        Vehicle = 2,
        Bike = 3,
        Boot = 4,
        NeedARide = 8,
        LandedWell = 9,
        NeedTechnicalSupport = 12,
        NeedMedicalHelp = 13,
        DistressCall = 14,
        DistressCallAutomatically = 15
    };

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
    |          Ground Type          |        Reserved       |Tracki.|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */
    /// @brief Packet payload for encoding Ground Tracking
    class GroundTracking : PacketPayloadBase
    {
    public:
        bool shouldTrackOnline = false;
        GroundTrackingType type = GroundTrackingType::Other;
        Location location;

        size_t parse(etl::bit_stream_reader &reader) override;
        size_t encode(etl::bit_stream_writer &writer) const override;
        bool operator==(const PacketPayloadBase &) const override;
        PacketType getType() const override;
    };

}