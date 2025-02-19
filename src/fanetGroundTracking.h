#pragma once
#include <stdint.h>
#include <stddef.h>
#include "fanetLocation.h"

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
class GroundTracking
{
public:
    bool shouldTrackOnline = false;
    GroundTrackingType type = GroundTrackingType::Other;
    Location location;

    /// @brief Encodes the packet into the buffer
    /// @param to Buffer to encode payload into
    /// @return Size of the encoded packet.
    size_t encode(char *to) const;

    /// @brief Parses character array into a Message type
    /// @param  buffer buffer where message resides.
    /// @param size size of buffer
    /// @return Returns a `Message` object parsed from the buffer.
    static GroundTracking parse(const char *buffer, const size_t size);
};