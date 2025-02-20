#include "fanetName.h"

bool Fanet::Name::operator==(const PacketPayloadBase &other) const
{
    auto otherName = dynamic_cast<const Name*>(&other);
    if (otherName == nullptr) {
        return false;
    }
    return name == otherName->name;
}

size_t Fanet::Name::parse(etl::bit_stream_reader &reader)
{
    auto byte = reader.read<uint8_t>(8U);
    int i = 0;
    while(byte.has_value() && byte.value() != '\0') {
        name[i++] = byte.value();
        byte = reader.read<uint8_t>(8U);
    }
    return i;
}

size_t Fanet::Name::encode(etl::bit_stream_writer &writer) const
{
    int i = 0;
    while(name[i] != '\0' && i < sizeof(name)) {
        writer.write_unchecked(name[i++], 8U);
    }
    writer.write_unchecked('\0', 8U);
    return i;
}
