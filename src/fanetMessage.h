#pragma once
#include <stdint.h>
#include <stddef.h>

class Message {
public:
    // Spec deems this subheader is TBD for future use.  0 is for "normal use"
    char subheader = 0;

    /// @brief Unicode message up to 244 bytes (assuming Fanet mac header of 11, + 256 bytes for max LoRa buffer)
    char message[244];

    /// @brief Encodes the packet into the buffer
    /// @param to Buffer to encode payload into
    /// @return Size of the encoded packet.
    size_t encode(char* to) const;

    /// @brief Parses character array into a Message type
    /// @param  buffer buffer where message resides.
    /// @param size size of buffer
    /// @return Returns a `Message` object parsed from the buffer.
    static Message parse(const char* buffer, const size_t size);
};