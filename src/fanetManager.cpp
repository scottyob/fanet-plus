#include "fanetManager.h"
#include "fanetPacket.h"
#include "etl/random.h"

using namespace Fanet;

void Fanet::FanetManager::Begin(Mac srcAddress)
{
    src = srcAddress;
}

etl::optional<Packet> Fanet::FanetManager::handleRx(const etl::array<uint8_t, FANET_MAX_PACKET_SIZE> &bytes,
                                                    const size_t &size,
                                                    unsigned long ms)
{
    // If this packet is useful to the application layer, we'll return it
    etl::optional<Packet> ret;

    // A packet has been received.  First parse it.
    auto packet = Packet::parse(bytes, size);

    // If the packet is from our own mac-address, it's probably a forward and can be dropped
    if (packet.header.srcMac == src)
    {
        return ret;
    }

    // Update our neighbor table based on the source address
    auto it = neighborTable.find(packet.header.srcMac.toInt32());
    auto inTable = (it != neighborTable.end());
    if (inTable)
    {
        it->second = ms;
    }
    else
    {
        if (neighborTable.full())
        {
            flushOldNeighborEntries(ms);
        }
        neighborTable.insert(etl::pair<uint32_t, unsigned long>(packet.header.srcMac, ms));
    }

    // Destination address, if set.
    Mac *dst = NULL;
    bool dstInAddressTable = false;
    if (packet.extHeader.has_value() && packet.extHeader.value().destinationMac.has_value())
    {
        dst = &packet.extHeader.value().destinationMac.value();

        // If the packet is destined for us, do no further processing
        if (*dst == src)
        {
            // If an ack was requested, Let's queue one
            auto ackType = packet.extHeader.value().ackType;
            if (ackType == ExtendedHeaderAckType::Forwarded && !packet.header.shouldForward)
            {
                sendPacket(Ack(), true, packet.header.srcMac);
            }
            else if (ackType == ExtendedHeaderAckType::Forwarded || ackType == ExtendedHeaderAckType::Requested)
            {
                sendPacket(Ack(), false, packet.header.srcMac);
            }

            return packet;
        }

        dstInAddressTable = neighborTable.find(dst->toInt32()) != neighborTable.end();
    }

    // Rules for forwarding:
    // - Forward bit set
    // - If unicast, is not destined for us and in mac table
    bool shouldForward = packet.header.shouldForward && (!dst || dstInAddressTable);
    if (shouldForward)
    {
        Packet txPacket = packet;
        txPacket.header.shouldForward = false;
        // Forwarded packets should always be placed at the back of a queue

        txQueue.push(TxPacket(ms + etl::random_xorshift().range(FANET_RXMIT_MIN, FANET_RXMIT_MAX), txPacket));
    }

    // This packet is not specifically meant for someone else, so, it's probably interesting
    // to the application layer
    return packet;
}

void Fanet::FanetManager::doTx(bool (*f)(const etl::array<uint8_t, FANET_MAX_PACKET_SIZE> *bytes, const size_t &size))
{
    if (txQueue.empty())
        return;

    // Build a tx buffer based on the packet that is due to send
    TxPacket txPacket = txQueue.top();
    etl::array<uint8_t, FANET_MAX_PACKET_SIZE> txBuffer;
    auto size = txPacket.packet.encode(txBuffer);

    // Send the packet on the wire
    auto txSuccess = f(&txBuffer, size);

    if (txSuccess)
    {
        txQueue.pop();
    }
}

etl::optional<unsigned long> Fanet::FanetManager::nextTxTime(const unsigned long ms)
{
    if (txQueue.empty())
        return etl::optional<unsigned long>();

    return 0;
}

bool Fanet::FanetManager::sendPacket(const PacketPayload payload, unsigned long ms, const bool &shouldForward, etl::optional<Mac> destinationMac, const bool requestAck)
{
    if (requestAck && !destinationMac.has_value())
    {
        // Bad request, cannot request an ack for a broadcast
        return false;
    }

    // Not yet initialized
    if (!src.has_value())
    {
        return false;
    }

    // Craft the packet to send
    Packet txPacket;
    txPacket.header.srcMac = src.value();
    txPacket.header.hasExtensionHeader = false;
    txPacket.header.shouldForward = shouldForward;
    txPacket.payload = payload;

    if (destinationMac.has_value())
    {
        txPacket.header.hasExtensionHeader = true;
        ExtendedHeader extHeader;

        // Populate the extension header fields and add it to the packet.
        extHeader.ackType = ExtendedHeaderAckType::None;
        extHeader.includesSignature = false; // Also not supported
        extHeader.destinationMac = destinationMac.value();
        txPacket.extHeader = extHeader;
    }

    txPacket.header.type = ((PacketPayloadBase *)&payload)->getType();

    // Put this onto the front of the send list.  Remove the last if we're full
    txQueue.push(TxPacket(ms, txPacket));

    return true;
}

void Fanet::FanetManager::flushOldNeighborEntries(const unsigned long currentMs)
{
    etl::vector<std::pair<uint32_t, unsigned long>, FANET_MAX_NEIGHBORS> valid_neighbors;

    // Step 1: Iterate and collect valid neighbors
    for (auto it = neighborTable.begin(); it != neighborTable.end();)
    {
        if (currentMs - it->second > FANET_NEIGHBOR_MAX_TIMEOUT)
        {
            it = neighborTable.erase(it); // Remove expired neighbor
        }
        else
        {
            valid_neighbors.push_back(*it);
            ++it;
        }
    }

    // If we're not trying to make a lot of headroom,
    if (!neighborTable.full())
    {
        return;
    }

    // Step 2: Sort valid neighbors by timestamp (oldest first)
    etl::sort(valid_neighbors.begin(), valid_neighbors.end(),
              [](const auto &a, const auto &b)
              { return a.second < b.second; });

    // Step 3: Remove 25% of the oldest entries
    size_t cutoff = valid_neighbors.size() * 0.25;
    valid_neighbors.erase(valid_neighbors.begin(), valid_neighbors.begin() + cutoff);

    // Step 4: Rebuild the unordered_map
    neighborTable.clear();
    for (const auto &entry : valid_neighbors)
    {
        neighborTable.insert(entry);
    }
}
