#include "fanetMessage.h"
#include <cstring>
using namespace Fanet;

size_t Message::encode(char *to) const {
    // Copy the subheader to the buffer
    to[0] = subheader;

    // Copy the message to the buffer
    std::strncpy(to + 1, message, sizeof(message));

    // Return the total size of the encoded packet
    return 1 + std::strlen(message);
}

Message Message::parse(const char *buffer, const size_t size) {
    Message msg;

    // Extract the subheader from the buffer
    msg.subheader = buffer[0];

    // Extract the message from the buffer
    std::strncpy(msg.message, buffer + 1, size - 1);
    msg.message[size - 1] = '\0'; // Ensure null-termination

    return msg;
}
