#include "fanetMessage.h"
#include <cstring>
using namespace Fanet;

bool Fanet::Message::operator==(const PacketPayloadBase& other) const {
  if (getType() != other.getType()) {
    return false;
  }
  Message* otherMessage = static_cast<Message*>((PacketPayloadBase*)&other);
  // Check that the message s-string is equal
  if (!strcmp(message, otherMessage->message)) {
    return true;
  }
  return false;
}

size_t Fanet::Message::parse(etl::bit_stream_reader& reader) {
  auto byte = reader.read<uint8_t>(8U);
  int i = 0;
  while (byte.has_value() && byte.value() != '\0') {
    message[i++] = byte.value();
    byte = reader.read<uint8_t>(8U);
  }
  return i;
}

size_t Fanet::Message::encode(etl::bit_stream_writer& writer) const {
  int i = 0;
  while (message[i] != '\0' && i < sizeof(message)) {
    writer.write_unchecked(message[i++], 8U);
  }
  writer.write_unchecked('\0', 8U);
  return i;
}

PacketType Fanet::Message::getType() const {
  return PacketType::Message;
}
