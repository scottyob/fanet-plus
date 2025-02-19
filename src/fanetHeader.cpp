#include "fanetHeader.h"
#include <iostream>
#include "etl/bit_stream.h"

using namespace Fanet;


Header Header::parse(const char* bytes) {
  auto header = Header();

  // Header is 4 bytes in length
  etl::bit_stream_reader reader((void*)bytes, (void*)(bytes + 3), etl::endian::big);

  header.hasExtensionHeader = reader.read_unchecked<uint8_t>(1);
  header.shouldForward = reader.read_unchecked<uint8_t>(1);
  header.type = (PacketType)reader.read_unchecked<uint8_t>(6);
  header.srcMac = Mac::parse(&bytes[1]);

  return header;
}

void Header::encode(char* to) {
  // Header is 4 bytes in length
  etl::bit_stream_writer writer((void*)to, (void*)(to + 3), etl::endian::big);

  writer.write_unchecked<uint8_t>(hasExtensionHeader, 1U);
  writer.write_unchecked<uint8_t>(shouldForward, 1U);
  writer.write_unchecked<uint8_t>((int)type, 6U);
  srcMac.encode(&to[1]);
}
