#include "fanetMac.h"
#include "etl/optional.h"
#include "etl/priority_queue.h"
#include "etl/unordered_map.h"
#include "fanetPacket.h"

// we keep the neighbors around for 5 minutes before timing them out.
#ifndef FANET_NEIGHBOR_MAX_TIMEOUT
#define FANET_NEIGHBOR_MAX_TIMEOUT 1000 * 60 * 5
#endif

#ifndef FANET_MAX_NEIGHBORS
#define FANET_MAX_NEIGHBORS 120 // How many maximum neighbors we'll keep around
#endif

#ifndef FANET_TX_QUEUE_DEPTH
#define FANET_TX_QUEUE_DEPTH 20 // How many packets can be sitting in the egress queue at one time
#endif

#ifndef FANET_RXMIT_MIN
#define FANET_RXMIT_MIN 300 // Min time (in ms) we should wait before rxmit'ing a packet
#endif

#ifndef FANET_RXMIT_MAX
#define FANET_RXMIT_MAX 600 // Maximum time (in ms) we should wait before rxmit'ing a packet
#endif

namespace Fanet
{
    /// @brief A packet queued for transmit.
    struct TxPacket
    {
        unsigned long sendAt;
        Packet packet;

        bool operator<(const TxPacket &other) const
        {
            return sendAt < other.sendAt;
        }

        TxPacket(unsigned long sendAt, Packet packet) : sendAt(sendAt), packet(packet) {}
    };

    /*
    @brief Manages the state and comms of a Fanet Protocol

    Fanet will act in a lot of roles, a receiver of packets, a sender, and also as a relay
    when a packet is requested to be forwarded.  This class manages the state of neighbors
    seen and orchestrates the relaying of Fanet packets when received.
    */
    class FanetManager
    {
    public:
        /// @brief Creates instance of FanetManager.  Required to Begin before using
        FanetManager() {}

        /// @brief Creates an instance of a FanetManager
        /// @param source address of the device you're initializing.
        FanetManager(Mac srcAddress) : src(srcAddress) {}

        /// @brief Begins or resets the FanetManager
        /// @param srcAddress source address of the device you're initializing
        void Begin(Mac srcAddress);

        /// @brief Handles receiving a packet
        /// @param bytes Receive buffer to handle
        /// @param size Size of received packet
        /// @param ms  Time at which packet was received (typically millis())
        etl::optional<Packet> handleRx(const etl::array<uint8_t, FANET_MAX_PACKET_SIZE> &bytes,
                                       const size_t &size,
                                       unsigned long ms);

        /// @brief Handles transmitting a packet from our tx queue
        /// @param f function pointer to perform the transmit, should return True if sent successfully
        void doTx(bool (*f)(const etl::array<uint8_t, FANET_MAX_PACKET_SIZE> *bytes, const size_t &size));

        /// @brief Time in ms we next wish to perform a tx
        /// @param ms current time
        /// @return the offset of when we next wish to perform a transmit, if set
        etl::optional<unsigned long> nextTxTime(const unsigned long ms);

        /// @brief Requests a packet be sent
        /// @param pkt packet to send
        /// @param pkt time to send (current time)
        /// @param shouldForward if the packet should be forwarded
        /// @param destinationMac
        bool sendPacket(const PacketPayload payload, unsigned long ms, const bool &shouldForward = true, etl::optional<Mac> destinationMac = etl::optional<Mac>(), const bool requestAck = true);

    private:
        etl::optional<Mac> src; // Src address, (ours)

        // Neighbor table with key being mac address, value being when we last saw them
        etl::unordered_map<uint32_t, unsigned long, FANET_MAX_NEIGHBORS> neighborTable;

        // etl:: <Packet, FANET_TX_QUEUE_DEPTH> txQueue;
        etl::priority_queue<TxPacket, FANET_TX_QUEUE_DEPTH> txQueue;

        /// @brief Flushes old neighbors from the state table
        /// @param makeHeadroom Ensures there is
        void flushOldNeighborEntries(const unsigned long currentMs);
    };
}