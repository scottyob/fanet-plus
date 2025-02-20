#pragma once
#include <cstdint>

#include "fanetExtHeader.h"
#include "fanetGroundTracking.h"
#include "fanetHeader.h"
#include "fanetMessage.h"
#include "fanetTracking.h"
#include "fanetName.h"
#include "fanetPayload.h"
#include "fanetAck.h"

#include <etl/optional.h>
#include <etl/variant.h>
#include <etl/checksum.h>
#include <etl/bit_stream.h>

/*
 Name packets are arbitrary length, but have an upper bound of 245
 bytes, assuming a LoRa buffer of 256 bytes and FANET mac header of
 11 bytes.  This is in unicode UTF-8.
*/
#define FANET_MAX_PACKET_SIZE 256

namespace Fanet
{
    using PacketPayload = etl::variant<
        Ack,      // Type 0
        Tracking, // Type 1
        Name,     // Type 2
        Message,  // type 3
        // Skip unsupported types
        GroundTracking // Type 7
        >;

    /*
        A Fanet+ Packet, (typically intended to be sent over LoRa)
    */
    class Packet
    {
    public:
        Header header;
        etl::optional<ExtendedHeader> extHeader;

        // All packet payloads can be treated as a PacketPayloadBase
        // To get a packet of type Tracking for instance, you'd use
        // auto trackingPayload = etl::get<Tracking>(packet.payload)
        PacketPayload payload;

        // Parses a byte stream, will return a packet
        static Packet parse(const etl::array<uint8_t, FANET_MAX_PACKET_SIZE> &bytes, size_t length);

        // Encodes the packet to a byte stream, returns the length of bytes
        // encoded packet
        size_t encode(etl::array<uint8_t, FANET_MAX_PACKET_SIZE> &bytes) const;

        bool operator==(const Packet &other) const;
    };

}