#include <math.h>
#include <iostream>

#include "fanetTracking.h"
#include <etl/bit_stream.h>

using namespace Fanet;


Tracking Tracking::parse(const char *buffer, const size_t size)
{
    auto tracking = Tracking();

    etl::bit_stream_reader bit_stream((void *)buffer, size, etl::endian::big);
    
    // Get the location
    tracking.location = Location::fromBitStream(bit_stream);
    
    // LSB of location
    tracking.altitude = etl::read_unchecked<uint16_t>(bit_stream, 8U);

    tracking.onlineTracking = etl::read_unchecked<char>(bit_stream, 1U);
    auto aircraftType = etl::read_unchecked<uint8_t>(bit_stream, 3U);
    tracking.aircraftType = (AircraftType)aircraftType;
    bool scaling = etl::read_unchecked<char>(bit_stream, 1U);
    auto altitudeMsb = etl::read_unchecked<uint8_t>(bit_stream, 3U);

    // Tracking is 11 bits, with the most significant bits disjoint
    tracking.altitude |= (altitudeMsb << 8);

    if (scaling)
    {
        tracking.altitude *= kAltScalingFactor;
    }

    // Speed is in counts of 0.5km/h
    scaling = etl::read_unchecked<char>(bit_stream, 1U);
    uint8_t speed = etl::read_unchecked<uint8_t>(bit_stream, 7U);
    tracking.speed = speed * 0.5 * (scaling ? kSpeedScalingFactor : 1);

    // Climb rate is in counts of 0.1m/s
    scaling = etl::read_unchecked<char>(bit_stream, 1U);
    uint8_t climbRate = etl::read_unchecked<uint8_t>(bit_stream, 7U);
    tracking.climbRate = climbRate * 0.1 * (scaling ? kClimbRateScalingFactor : 1);

    // Heading is per 360/256 deg
    tracking.heading = (360/256) * etl::read_unchecked<uint8_t>(bit_stream, 8U);

    if(!bit_stream.size_bits() < 96) {
        return tracking;
    }

    // Turn rate is optional, in 0.25deg/s
    scaling = etl::read_unchecked<char>(bit_stream, 1U);
    uint8_t turnRate = etl::read_unchecked<uint8_t>(bit_stream, 7U);
    tracking.turnRate = 0.25 * turnRate * (scaling ? kTurnRateScalingFactor : 1);

    if(!bit_stream.size_bits() < 104) {
        return tracking;
    }

    // QNE Offset is in meters
    scaling = etl::read_unchecked<char>(bit_stream, 1U);
    uint8_t offset = etl::read_unchecked<uint8_t>(bit_stream, 7U);
    tracking.qneOffset = offset * (scaling ? kQneOffsetScalingFactor : 1);

    return tracking;
}

/// @brief Scales a number by a unit factor, and determines if a scaling factor needs to be applied
/// @tparam T The type of the number to scale
/// @param number The number to scale the base unit by
/// @param unitFactor The factor to scale the number by
/// @param scalingFactor The factor this unit is scaled by
/// @param bitCount The number of bits available
/// @param scaled Reference to store if the number was scaled
/// @return The scaled number
template <typename T>
int toScaled(T number, float unitFactor, float scalingFactor, int bitCount, bool &scaled) {
    // Get the number to be represented in the packet.
    T ret = number / unitFactor;

    // Works out the largest number we can represent given the number of bits
    int constrainedMax = pow(2, bitCount) - 1;

    // If the return value can fit unscaled, return it
    if ((int)ret <= constrainedMax) {
        scaled = false;
        return ret;
    }

    // return the scaled value.
    scaled = true;
    return etl::clamp(int(ret / scalingFactor), 0, constrainedMax);
}


size_t Tracking::encode(char *to) const
{
    etl::bit_stream_writer bit_stream((void*) to, (void*)(to + 256), etl::endian::big);
    
    // Write location
    location.toBitStream(bit_stream);
    size_t size = 48;

    // Work out altitude to encode, based on the scaling factor to have it fit into 11 bits.
    bool scaling;
    int alt = toScaled(altitude, 1, kAltScalingFactor, 11, scaling);

    // Altitude least significant bits
    etl::write_unchecked(bit_stream, alt & 0xFF, 8U);

    // Next byte is tracking, Aircraft type, Aircraft scaling, and MSB of Altitude
    etl::write_unchecked(bit_stream, (int)onlineTracking, 1U);
    etl::write_unchecked(bit_stream, (int)aircraftType, 3U);
    etl::write_unchecked(bit_stream, (int)scaling, 1U);

    // Bits 8-10 of the (MSB) of the altitude
    etl::write_unchecked(bit_stream, alt & 0x700, 3U);
    size += 2; // 2 byes for the altitude, tracking, aircraft type.

    // 1 byte for the speed.
    int speed2 = toScaled(speed, 0.5, kSpeedScalingFactor, 7, scaling);
    etl::write_unchecked(bit_stream, scaling, 1U);
    etl::write_unchecked(bit_stream, speed2, 7U);
    size++;

    // 1 byte for climbRate
    int climb2 = toScaled(climbRate, 0.1f, kClimbRateScalingFactor, 7, scaling);
    etl::write_unchecked(bit_stream, scaling, 1U);
    etl::write_unchecked(bit_stream, climb2, 7U);
    size++;

    // Heading is per 360/256.  One byte
    etl::write_unchecked<uint8_t>(bit_stream, heading / (360/256), 8U);
    size++;

    if(!turnRate.has_value())
        return size;
    
    // Write the turn rate (in 0.25deg/s units)
    int turn2 = toScaled(turnRate.value(), 0.25, kTurnRateScalingFactor, 7, scaling);
    etl::write_unchecked(bit_stream, scaling, 1U);
    etl::write_unchecked(bit_stream, turn2, 7U);
    size++;

    if(!qneOffset.has_value())
        return size;
    
    // Write the QNE Offset in meters
    int scaling2 = toScaled(qneOffset.value(), 1, kQneOffsetScalingFactor, 7, scaling);
    etl::write_unchecked(bit_stream, scaling, 1U);
    etl::write_unchecked(bit_stream, scaling2, 7U);
    size++;

    return size;
}
