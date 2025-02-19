#pragma once

#include <stdint.h>
#include "fanetMac.h"

namespace Fanet
{

  // Header is 4 bytes (3 for src address + 1 for additional fields)
  const auto kHeaderLength = 4;

  // Packet payload types
  enum class PacketType : uint8_t
  {
    Ack = 0,           // Acknowledge
    Tracking = 1,      // Tracking
    Name = 2,          // Name
    Message = 3,       // Message
    Service = 4,       // Service
    Landmarks = 5,     // Landmarks
    RemoteConfig = 6,  // Remote Configuration
    GroundTracking = 7 // Ground Tracking
  };

  /*
  This is the top level header.  This class doesn't contain the
  optional extended header.  This header is 4 bytes in length.

  This header is intended to be used in a Packet along with the
  packet payloads and any additional extended header information.

  /*
  Header:
  [scott@sob-desktop protocol]$ ./protocol "Type: 6,Forward:1,Ext:1,SrcVendor:8,SrcAddress:16" -b 8
  -lsb -ph 4
                                                                 0
         7       6       5       4       3       2       1       0
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |  Ext  |Forward|                      Type                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                           SrcVendor                           |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                               |
  +                           SrcAddress                          +
  |                                                               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  */
  class Header
  {
  public:
    PacketType type;
    bool shouldForward;
    bool hasExtensionHeader;
    Mac srcMac;

    // Parses a packet header from a 4 byte array
    static Header parse(const char *bytes);

    // Encodes the packet into a 4 byte array.
    void encode(char *to);
  };

}