#include "fanetPacket.h"
#include <etl/optional.h>
using namespace Fanet;

Packet Packet::parse(const char* bytes, size_t length) {
  Packet packet;

  // Parse the packet header
  packet.header = Header::parse(bytes);

  // Advance the byte packet past the header
  bytes += kHeaderLength;
  length -= kHeaderLength;

  // If there's any extended header attributes, parse them
  if (packet.header.hasExtensionHeader) {
    size_t extHeaderLength = 0;
    packet.extHeader = ExtendedHeader::parse(bytes, extHeaderLength);
    bytes += extHeaderLength;
    length -= extHeaderLength;
  }

  // Parse the payload of the packet
  switch (packet.header.type) {
    case PacketType::Tracking:
      packet.payload = Tracking::parse(bytes, length);
      break;
    case PacketType::Name:
      strncpy((char*)&packet.payload, bytes, length);
      ((char*)&packet.payload)[length] = '\0';  // Ensure null-termination
      break;
    case PacketType::Message: {
      packet.payload = Message::parse(bytes, length);
      break;
    }
    case PacketType::Service:
      break;  // Not Supported
    case PacketType::Landmarks:
      break;  // Not Supported
    case PacketType::RemoteConfig:
      break;  // Not Supported
    case PacketType::GroundTracking:
      packet.payload = GroundTracking::parse(bytes, length);
      break;
  }

  return packet;
}

size_t Packet::encode(char* to) {
  size_t size = kHeaderLength;

  // Encode the packet header
  header.encode(to);
  to += kHeaderLength;

  // Encode the packet payload
  switch (header.type) {
    case PacketType::Tracking:
      size += etl::get<Tracking>(payload).encode(to);
      break;
    case PacketType::Name: {
      auto strName = etl::get<Name>(payload).data();
      auto payloadSize = strlen(strName) + 1;
      strncpy(to, strName, MAX_NAME_SIZE);
      size += payloadSize;
    }
    case PacketType::Message:
      size += etl::get<Message>(payload).encode(to);
      break;
    case PacketType::Service:
      break;  // Not Supported
    case PacketType::Landmarks:
      break;  // Not Supported
    case PacketType::RemoteConfig:
      break;  // Not Supported
    case PacketType::GroundTracking:
      size += etl::get<GroundTracking>(payload).encode(to);
      break;
  }

  return size;
}
