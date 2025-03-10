#pragma once

#include "etl/delegate.h"
#include "etl/deque.h"
#include "etl/list.h"
#include "etl/optional.h"
#include "etl/random.h"
#include "etl/unordered_map.h"
#include "fanetMac.h"
#include "fanetNeighbor.h"
#include "fanetPacket.h"

// we keep the neighbors around for 5 minutes before timing them out.
#ifndef FANET_NEIGHBOR_MAX_TIMEOUT
#define FANET_NEIGHBOR_MAX_TIMEOUT 1000 * 60 * 5
#endif

#ifndef FANET_MAX_NEIGHBORS
#define FANET_MAX_NEIGHBORS 120  // How many maximum neighbors we'll keep around
#endif

#ifndef FANET_TX_QUEUE_DEPTH
#define FANET_TX_QUEUE_DEPTH 20  // How many packets can be sitting in the egress queue at one time
#endif

#ifndef FANET_RXMIT_MIN
#define FANET_RXMIT_MIN 10  // Min time (in ms) we should wait before rxmit'ing a packet
#endif

#ifndef FANET_RXMIT_MAX
#define FANET_RXMIT_MAX 500  // Maximum time (in ms) we should wait before rxmit'ing a packet
#endif

#ifndef FANET_FORWARD_MIN_DB_BOOST
#define FANET_FORWARD_MIN_DB_BOOST \
  20.0f  // If a forwarded packet is boosted by this amount, don't rxmit
#endif

#ifndef FANET_FORWARD_MAX_RSSI_DBM
#define FANET_FORWARD_MAX_RSSI_DBM -90.0f  // If a packet is received with this RSSI, don't forward
#endif

#ifndef FANET_CSMA_MIN
#define FANET_CSMA_MIN \
  20  // Wait a min of 20ms before trying to transmit again if the channel is busy
#endif

#ifndef FANET_CSMA_MAX
#define FANET_CSMA_MAX \
  40  // Wait a max of ms before trying to transmit again if the channel is busy
#endif

#ifndef FANET_MAX_SEND_AGE
#define FANET_MAX_SEND_AGE \
  800  // If a packet is older than this many ms, don't try and send it.  Assume too old.
#endif

namespace Fanet {
  /// @brief A packet queued for transmit.
  struct TxPacket {
    unsigned long sendAt;  // Time we wish to send (will time)
    unsigned long rxTime;  // If forwarded, keep track of when this packet was received.
    float rssi;            // If forwarded, keep track of the rx Rssi
    Packet packet;

    bool operator<(const TxPacket& other) const { return sendAt < other.sendAt; }

    TxPacket(unsigned long sendAt, Packet packet, float rssi = 0.0f, unsigned long rxTime = 0)
        : sendAt(sendAt), packet(packet), rssi(rssi) {
      // Time received defaults to time to send if not sent
      this->rxTime = rxTime ? rxTime : sendAt;
    }
  };

  struct Stats {
    uint32_t rx = 0;                 // All packets received
    uint32_t txSuccess = 0;          // All packets transmitted
    uint32_t txFailed = 0;           // An attempted transmission failed
    uint32_t processed = 0;          // Packets passed to the application stack to be processed
    uint32_t forwarded = 0;          // All packets that were forwarded
    uint32_t fwdMinRssiDrp = 0;      // Packets discarded due to Rssi being too good
    uint32_t fwdNeighborDrp = 0;     // Packets discarded due to no neighbor in neighbor table
    uint32_t fwdEnqueuedDrop = 0;    // Packet was already queued
    uint32_t fwdDbBoostDrop = 0;     // Pkts dropped from txQueue with subsequent good rssi
    uint32_t rxFromUsDrp = 0;        // Dropped packets from our own Mac
    uint32_t txAck = 0;              // Number of Acks sent
    uint32_t neighborTableSize = 0;  // Number of neighbors currently in our neighbor table
  };

  /*
  @brief Manages the state and comms of a Fanet Protocol

  Fanet will act in a lot of roles, a receiver of packets, a sender, and also as a relay
  when a packet is requested to be forwarded.  This class manages the state of neighbors
  seen and orchestrates the relaying of Fanet packets when received.
  */
  class FanetManager {
   public:
    /// @brief Creates instance of FanetManager.  Required to Begin before using
    FanetManager() {}

    /// @brief Creates an instance of a FanetManager
    /// @param source address of the device you're initializing.
    /// @param ms Current time (used for seeding random number generator).
    FanetManager(Mac srcAddress, unsigned long ms) { Begin(srcAddress, ms); }

    /// @brief Begins or resets the FanetManager
    /// @param srcAddress source address of the device you're initializing
    void Begin(Mac srcAddress, unsigned long ms);

    /// @brief Handles receiving a packet
    /// @param bytes Receive buffer to handle
    /// @param size Size of received packet
    /// @param ms  Time at which packet was received (typically millis())
    etl::optional<Packet> handleRx(const etl::array<uint8_t, FANET_MAX_PACKET_SIZE>& bytes,
                                   const size_t& size,
                                   unsigned long ms,
                                   float rssi,
                                   float snr);

    /// @brief Handles transmitting a packet from our tx queue
    /// @param ms current time
    /// @param f function pointer to perform the transmit, should return True if sent successfully
    void doTx(unsigned long ms,
              etl::delegate<bool(const etl::array<uint8_t, FANET_MAX_PACKET_SIZE>* bytes,
                                 const size_t& size)> f);

    /// @brief Time in ms we next wish to perform a tx
    /// @param ms current time
    /// @return the offset of when we next wish to perform a transmit, if set
    etl::optional<unsigned long> nextTxTime(const unsigned long& ms);

    /// @brief Requests a packet be sent
    /// @param pkt packet to send
    /// @param pkt time to send (current time)
    /// @param shouldForward if the packet should be forwarded
    /// @param destinationMac
    bool sendPacket(const PacketPayload payload,
                    unsigned long ms,
                    const bool& shouldForward = true,
                    etl::optional<Mac> destinationMac = etl::optional<Mac>(),
                    const ExtendedHeaderAckType requestAck = ExtendedHeaderAckType::None);

    /// @brief If set, we'll transmit our position as ground positions
    /// @param type
    void setGroundType(etl::optional<GroundTrackingType::enum_type> type) { groundType = type; }

    /// @brief Returns the ground tracking type
    /// @return current ground tracking type
    etl::optional<GroundTrackingType::enum_type> getGroundType() const { return groundType; }

    /// @brief Sets the position to transmit based on location update rules (how much traffic we
    /// see)
    /// @param lat latitude
    /// @param lng longitude
    /// @param alt altitude
    void setPos(const float& lat,
                const float& lng,
                const uint32_t& alt,
                const unsigned long& ms,
                int heading = 0,  // in degrees
                float climbRate = 0.0f,
                float speedKmh = 0.0f) {
      this->lat = lat;
      this->lng = lng;
      this->alt = alt;
      this->climbRate = climbRate;
      this->heading = heading;
      this->speed = speedKmh;
      queueTrackingUpdate(ms);
    }

    // Public attributes that can be sent for tracking updates
    AircraftType aircraftType;

    /// @brief Gets runtime statistics
    /// @return Stats object
    Stats getStats() {
      auto ret = stats;
      ret.neighborTableSize = neighborTable.size();
      return ret;
    }

    /// @brief Gets a copy of the neighbor table
    etl::unordered_map<uint32_t, Neighbor, FANET_MAX_NEIGHBORS> getNeighborTable() {
      return neighborTable;
    }

    /// @brief Flushes old neighbors from the state table
    void flushOldNeighborEntries(const unsigned long& currentMs);

   protected:
    etl::optional<Mac> src;  // Src address, (ours)

    // Neighbor table with key being mac address, value being when we last saw them
    etl::unordered_map<uint32_t, Neighbor, FANET_MAX_NEIGHBORS> neighborTable;

    // etl:: <Packet, FANET_TX_QUEUE_DEPTH> txQueue;
    etl::list<TxPacket, FANET_TX_QUEUE_DEPTH> txQueue;

    /// @brief Queues a packet for transmission
    /// @param txPacket packet to queue
    /// @param ms current ms
    void queueForwardFrame(TxPacket txPacket, const unsigned long& ms);

    /// @brief Random number generator
    etl::random_xorshift random;

    // When a transmit has failed, we'll wait a random amount of time before trying
    // any transmissions again.  This is the time we'll wait for a new tx.
    unsigned long csmaNextTx = 0;

    // Our last known location we want to transmit
    float lat;
    float lng;
    uint32_t alt;
    float climbRate;
    int heading;
    float speed;
    etl::optional<GroundTrackingType::enum_type> groundType;

    /// @brief Queues a tracking update packet if the internal has been long enough since our last
    /// update
    /// @param ms Current ms
    void queueTrackingUpdate(const unsigned long& ms);

    // Time which we're allowed to enqueue a tracking packet
    unsigned long nextAllowedTrackingTime = 0;

    unsigned long lastLocationSentMs = 0;

    // Keep track of statistics
    Stats stats;
  };
}  // namespace Fanet
