#pragma once
#include <cstdint>

#include "fanetExtHeader.h"
#include "fanetGroundTracking.h"
#include "fanetHeader.h"
#include "fanetMessage.h"
#include "fanetTracking.h"

#include <etl/optional.h>
#include <etl/variant.h>

/*
 Name packets are arbitrary length, but have an upper bound of 245
 bytes, assuming a LoRa buffer of 256 bytes and FANET mac header of
 11 bytes.  This is in unicode UTF-8.
*/
#define MAX_NAME_SIZE 245
using Name = etl::array<char, MAX_NAME_SIZE>;

/*
    A Fanet+ Packet, (typically intended to be sent over LoRa)
*/
class Packet {
 public:
  Header header;
  etl::optional<ExtendedHeader> extHeader;

  // Parses a byte stream, will return a packet
  static Packet parse(const char* bytes, size_t length);

  // Encodes the packet to a byte stream, returns the length of bytes
  // encoded packet
  size_t encode(char* to);

  // Note:  Ack types do not carry a payload.
  etl::variant<Tracking,  // Type 1
               Name,      // Type 2
               Message,   // type 3
               // Skip unsupported types
               GroundTracking  // Type 7
               >
      payload;
};