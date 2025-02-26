#include "fanetGroundTracking.h"

using namespace Fanet;

size_t Fanet::GroundTracking::parse(etl::bit_stream_reader& reader) {
  location = Location::fromBitStream(reader);
  type = (GroundTrackingType)reader.read_unchecked<uint8_t>(4U);
  reader.skip(3U);  // Unused
  shouldTrackOnline = reader.read_unchecked<uint8_t>(1U);

  return 56;
}

size_t Fanet::GroundTracking::encode(etl::bit_stream_writer& writer) const {
  // Encode the location
  location.toBitStream(writer);

  // Write the ground type
  writer.write_unchecked<uint8_t>((int)type, 4U);
  writer.skip(3U);  // Unused
  writer.write_unchecked<uint8_t>(shouldTrackOnline, 1U);
  return 56;
}

bool Fanet::GroundTracking::operator==(const PacketPayloadBase& other) const {
  if (getType() != other.getType()) {
    return false;
  }
  auto otherGround = static_cast<const GroundTracking*>(&other);
  return (location == otherGround->location && type == otherGround->type &&
          shouldTrackOnline == otherGround->shouldTrackOnline);
}

PacketType Fanet::GroundTracking::getType() const {
  return PacketType::GroundTracking;
}
