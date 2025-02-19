#pragma once
#include "fanetMac.h"

#include <cstdint>
#include <cstddef>
#include <etl/optional.h>

// Extension header can be a max of 8 bytes
#define kExtendedHeaderMaxSize 8

namespace Fanet
{

    enum class ExtendedHeaderAckType : uint8_t
    {
        None = 0,      // none (default)
        Requested = 1, // requested
        Forwarded = 2, // requested (via forward, if received via forward (received forward bit = 0). must be used if forward is set)
        Reserved = 3   // reserved
    };

    enum class ExtendedHeaderCastType : bool
    {
        Broadcast = 0,
        Unicast = 1,
    };

    /// @brief Fanet+ Extension Header
    class ExtendedHeader
    {
    public:
        ExtendedHeaderAckType ackType;
        bool includesSignature;

        // Will set the Cast bit if set
        etl::optional<Mac> destinationMac;

        // NOTE:  Signature is NOT supported

        /// @brief Parses extension header
        /// @param from Byte stream to parse from
        /// @param size Size of header (bytes read)
        /// @return
        static ExtendedHeader parse(const char *from, size_t &size);
        size_t encode(char *to) const;
    };

}