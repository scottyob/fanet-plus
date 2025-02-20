#include "fanetHeader.h"
#include <iostream>
#include "etl/bit_stream.h"

using namespace Fanet;

size_t Fanet::Header::parse(etl::bit_stream_reader &reader)
{
  hasExtensionHeader = reader.read_unchecked<uint8_t>(1);
  shouldForward = reader.read_unchecked<uint8_t>(1);
  type = (PacketType)reader.read_unchecked<uint8_t>(6);
  srcMac = Mac::parse(reader);
  return 4;
}

size_t Fanet::Header::encode(etl::bit_stream_writer &writer) const
{

  writer.write_unchecked<uint8_t>(hasExtensionHeader, 1U);
  writer.write_unchecked<uint8_t>(shouldForward, 1U);
  writer.write_unchecked<uint8_t>((int)type, 6U);
  srcMac.encode(writer);
  return 4;
}

bool Fanet::Header::operator==(const Header &other) const
{
  return (srcMac == other.srcMac &&
          hasExtensionHeader == other.hasExtensionHeader &&
          shouldForward == other.shouldForward &&
          type == other.type);
}
