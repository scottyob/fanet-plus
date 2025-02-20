#include "fanetPacket.h"
#include <etl/optional.h>
#include <etl/bit_stream.h>

using namespace Fanet;

Packet Packet::parse(const etl::array<uint8_t, FANET_MAX_PACKET_SIZE> &bytes, size_t length)
{
  Packet packet;
  etl::bit_stream_reader reader((void*)(&bytes.data()[0]), length, etl::endian::big);

  // Parse the packet header
  size_t bytesParsed = packet.header.parse(reader);

  // If there's any extended header attributes, parse them
  if (packet.header.hasExtensionHeader)
  {
    ExtendedHeader extHeader;
    bytesParsed += extHeader.parse(reader);
  }

  // Parse the payload of the packet
  switch (packet.header.type)
  {
  case PacketType::Ack:
    packet.payload = Ack();
    break;
  case PacketType::Tracking:
    packet.payload = Tracking();
    break;
  case PacketType::Name:
    packet.payload = Name();
    break;
  case PacketType::Message:
    packet.payload = Message();
    break;
  case PacketType::Service:
    break; // Not Supported
  case PacketType::Landmarks:
    break; // Not Supported
  case PacketType::RemoteConfig:
    break; // Not Supported
  case PacketType::GroundTracking:
    packet.payload = GroundTracking();
    break;
  }
  // Parse the payload based on the subtype
  PacketPayloadBase *payload = (PacketPayloadBase*)&packet.payload;
  bytesParsed += payload->parse(reader);

  return packet;
}

size_t Packet::encode(etl::array<uint8_t, FANET_MAX_PACKET_SIZE> &bytes) const
{
  etl::bit_stream_writer writer((void*)(&bytes.data()[0]), FANET_MAX_PACKET_SIZE, etl::endian::big);
  size_t size = 0;

  // Encode the packet header
  size += header.encode(writer);

  // If there's an extension header, encode this
  if (extHeader.has_value())
  {
    size += extHeader.value().encode(writer);
  }

  // Encode the packet payload
  const PacketPayloadBase *payload = (PacketPayloadBase *)&this->payload;
  size += payload->encode(writer);

  return size;

}

bool Fanet::Packet::operator==(const Packet &other) const
{
  if (!(header == other.header))
    return false;

  if (extHeader.has_value() != other.extHeader.has_value())
    return false;
  
  if (extHeader.has_value() && !(extHeader.value() == other.extHeader.value())) {
    return false;
  }

  const PacketPayloadBase* payloadBase = (PacketPayloadBase*)&payload;
  const PacketPayloadBase* otherPayloadBase = (PacketPayloadBase*)&other.payload;
  if(!payloadBase->operator==(*otherPayloadBase)) {
    return false;
  }

  return true;
}
