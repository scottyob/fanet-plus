#include "fanetManager.h"
#include "etl/delegate.h"
#include "fanetPacket.h"

using namespace Fanet;

void Fanet::FanetManager::Begin(Mac srcAddress, unsigned long ms) {
  src = srcAddress;
  random.initialise(ms);
}

etl::optional<Packet> Fanet::FanetManager::handleRx(
    const etl::array<uint8_t, FANET_MAX_PACKET_SIZE>& bytes,
    const size_t& size,
    unsigned long ms,
    float rssi) {
  // If this packet is useful to the application layer, we'll return it
  etl::optional<Packet> ret;

  // A packet has been received.  First parse it.
  auto packet = Packet::parse(bytes, size);

  // If the packet is from our own mac-address, it's probably a forward and can be dropped
  if (packet.header.srcMac == src) {
    return ret;
  }

  // Update our neighbor table based on the source address
  auto it = neighborTable.find(packet.header.srcMac.toInt32());
  auto inTable = (it != neighborTable.end());
  if (inTable) {
    it->second = ms;
  } else {
    if (neighborTable.full()) {
      flushOldNeighborEntries(ms);
    }
    neighborTable.insert(etl::pair<uint32_t, unsigned long>(packet.header.srcMac, ms));
  }

  // Destination address, if set.
  Mac* dst = NULL;
  bool dstInAddressTable = false;
  if (packet.extHeader.has_value() && packet.extHeader.value().destinationMac.has_value()) {
    dst = &packet.extHeader.value().destinationMac.value();

    // If the packet is destined for us, do no further processing
    if (*dst == src) {
      // If an ack was requested, Let's queue one
      auto ackType = packet.extHeader.value().ackType;
      if (ackType == ExtendedHeaderAckType::Forwarded && !packet.header.shouldForward) {
        // The sender requested a 2-hop Ack on an already forwarded packet.  We'll send the
        // ack back to the original sender with forward / possibly two hops away
        sendPacket(Ack(), ms, true, packet.header.srcMac);
      } else if (ackType == ExtendedHeaderAckType::Forwarded ||
                 ackType == ExtendedHeaderAckType::Requested) {
        // The sender requested an ack, but either did not request it be forwarded, or did request
        // the ack be forwarded but we got it directly.  Here we assume that we'll have
        // bi-directional communication and we'll send the ack directly back to the sender.
        sendPacket(Ack(), ms, false, packet.header.srcMac);
      }

      return packet;
    }

    dstInAddressTable = neighborTable.find(dst->toInt32()) != neighborTable.end();
  }

  // Rules for forwarding:
  // - Forward bit set
  // - If unicast, is not destined for us and in mac table
  bool shouldForward = packet.header.shouldForward && (!dst || dstInAddressTable);
  if (shouldForward) {
    auto txPacket = TxPacket(ms + random.range(FANET_RXMIT_MIN, FANET_RXMIT_MAX), packet, rssi, ms);
    queueForwardFrame(txPacket, ms);
  }

  // This packet is not specifically meant for someone else, so, it's probably interesting
  // to the application layer
  return packet;
}

void Fanet::FanetManager::doTx(
    unsigned long ms,
    etl::delegate<bool(const etl::array<uint8_t, FANET_MAX_PACKET_SIZE>* bytes, const size_t& size)>
        f) {
  if (txQueue.empty()) return;

  // Build a tx buffer based on the packet that is due to send
  TxPacket txPacket = txQueue.front();
  etl::array<uint8_t, FANET_MAX_PACKET_SIZE> txBuffer;
  auto size = txPacket.packet.encode(txBuffer);

  // Send the packet on the wire
  auto txSuccess = f(&txBuffer, size);
  if (txSuccess) {
    txQueue.pop_front();
    // 15ms + 2ms per byte before we're allowed to send again.
    // No idea why these values, they came from the stm32 Fanet implementation.
    csmaNextTx = ms + 15 + (size * 2);
  } else {
    // If the transmit failed, we'll wait a random amount of time before trying again
    csmaNextTx = ms + random.range(FANET_CSMA_MIN, FANET_CSMA_MAX);
  }
}

etl::optional<unsigned long> Fanet::FanetManager::nextTxTime(const unsigned long ms) {
  // Find the first eligible packet to send, removing those that are now too old from
  // the send queue
  for (auto it = txQueue.begin(); it != txQueue.end();) {
    if (it->rxTime > ms + FANET_MAX_SEND_AGE) {
      // If this packet has been in the queue for too long, drop it
      it = txQueue.erase(it);
      continue;
    }
    return etl::max(txQueue.front().sendAt, csmaNextTx);
  }

  // If we're here, there's nothing to send!  Our send queue is empty!
  return etl::optional<unsigned long>();
}

bool Fanet::FanetManager::sendPacket(const PacketPayload payload,
                                     unsigned long ms,
                                     const bool& shouldForward,
                                     etl::optional<Mac> destinationMac,
                                     const ExtendedHeaderAckType requestAck) {
  if (requestAck != ExtendedHeaderAckType::None && !destinationMac.has_value()) {
    // Bad request, cannot request an ack for a broadcast
    return false;
  }

  // Not yet initialized
  if (!src.has_value()) {
    return false;
  }

  // Craft the packet to send
  Packet txPacket;
  txPacket.header.srcMac = src.value();
  txPacket.header.hasExtensionHeader = false;
  txPacket.header.shouldForward = shouldForward;
  txPacket.payload = payload;

  if (destinationMac.has_value() || requestAck != ExtendedHeaderAckType::None) {
    txPacket.header.hasExtensionHeader = true;
    ExtendedHeader extHeader;

    // Populate the extension header fields and add it to the packet.
    //
    extHeader.ackType = requestAck;
    extHeader.includesSignature = false;  // Also not supported
    if (destinationMac.has_value()) extHeader.destinationMac = destinationMac.value();
    txPacket.extHeader = extHeader;
  }

  txPacket.header.type = ((PacketPayloadBase*)&payload)->getType();

  // Put this onto the front of the send list.  Only forwarded packets have a delay, so, assume
  // no sorting needed
  if (txQueue.full()) {
    // If we're full, remove the latest packet to send
    txQueue.pop_back();
  }
  txQueue.push_front(TxPacket(ms, txPacket, 0.0f, ms));
  return true;
}

void Fanet::FanetManager::flushOldNeighborEntries(const unsigned long currentMs) {
  etl::vector<std::pair<uint32_t, unsigned long>, FANET_MAX_NEIGHBORS> valid_neighbors;

  // Step 1: Iterate and collect valid neighbors
  for (auto it = neighborTable.begin(); it != neighborTable.end();) {
    if (currentMs - it->second > FANET_NEIGHBOR_MAX_TIMEOUT) {
      it = neighborTable.erase(it);  // Remove expired neighbor
    } else {
      valid_neighbors.push_back(*it);
      ++it;
    }
  }

  // If we're not full, we're done
  if (!neighborTable.full()) {
    return;
  }

  // Step 2: Sort valid neighbors by timestamp (oldest first)
  etl::sort(valid_neighbors.begin(), valid_neighbors.end(), [](const auto& a, const auto& b) {
    return a.second < b.second;
  });

  // Step 3: Remove 25% of the oldest entries
  size_t cutoff = valid_neighbors.size() * 0.25;
  valid_neighbors.erase(valid_neighbors.begin(), valid_neighbors.begin() + cutoff);

  // Step 4: Rebuild the unordered_map
  neighborTable.clear();
  for (const auto& entry : valid_neighbors) {
    neighborTable.insert(entry);
  }
}

void Fanet::FanetManager::queueForwardFrame(TxPacket txPacket, unsigned long ms) {
  if (txPacket.rssi > FANET_FORWARD_MAX_RSSI_DBM) {
    // If this frame is significantly strong, assume little good we will be done
    // forwarding it and drop it here.

    // TODO:
    // There could be a case where we have a lot more altitude than the sender, so,
    // this is something to consider at a later time.  Perhaps if a message or ground
    // based tracking message is received, we should still forward it anyway?
    return;
  }

  // If the packet is destined for a neighbor that's not in our neighbor table.
  // assume we can't deliver it there and drop the packet.
  if (txPacket.packet.extHeader.has_value() &&
      txPacket.packet.extHeader.value().destinationMac.has_value()) {
    auto it =
        neighborTable.find(txPacket.packet.extHeader.value().destinationMac.value().toInt32());
    if (it == neighborTable.end()) {
      return;
    }
  }

  // Ensure the forwarded frame does not have the forward flag set
  txPacket.packet.header.shouldForward = false;

  // Check this packet already in our tx Queue?
  for (auto it = txQueue.begin(); it != txQueue.end(); it++) {
    if (it->packet == txPacket.packet) {
      // If this frame is 20dB stronger, assume it has been re-broadcast
      // to our general direction and can be removed from the tx queue
      if (txPacket.rssi > it->rssi + FANET_FORWARD_MIN_DB_BOOST) {
        // Remove the packet from the queue
        txQueue.erase(it);
        return;
      }
      // Adjust the new tx time in the hope that we'll still get a new one
      // come in even stronger
      it->sendAt = ms + random.range(FANET_RXMIT_MIN, FANET_RXMIT_MAX);
      etl::insertion_sort(txQueue.begin(), txQueue.end(), [](const TxPacket& a, const TxPacket& b) {
        return a.sendAt < b.sendAt;
      });
      return;
    }
  }

  // put the packet on the back of the tx queue
  txQueue.push_back(txPacket);

  // ensure the packet is sorted
  etl::insertion_sort(txQueue.begin(), txQueue.end(), [](const TxPacket& a, const TxPacket& b) {
    return a.sendAt < b.sendAt;
  });
}
