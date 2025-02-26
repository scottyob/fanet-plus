#include <math.h>
#include <iostream>

#include <etl/bit_stream.h>
#include "fanetTracking.h"

using namespace Fanet;

size_t Fanet::Tracking::parse(etl::bit_stream_reader& reader) {
  // Get the location
  location = Location::fromBitStream(reader);

  // LSB of location
  altitude = etl::read_unchecked<uint16_t>(reader, 8U);

  onlineTracking = etl::read_unchecked<char>(reader, 1U);
  auto aircraftTypeInt = etl::read_unchecked<uint8_t>(reader, 3U);
  aircraftType = (AircraftType)aircraftTypeInt;
  bool scaling = etl::read_unchecked<char>(reader, 1U);
  auto altitudeMsb = etl::read_unchecked<uint8_t>(reader, 3U);

  // Tracking is 11 bits, with the most significant bits disjoint
  altitude |= (altitudeMsb << 8);

  if (scaling) {
    altitude *= kAltScalingFactor;
  }

  // Speed is in counts of 0.5km/h
  scaling = etl::read_unchecked<char>(reader, 1U);
  uint8_t speed = etl::read_unchecked<uint8_t>(reader, 7U);
  speed = speed * 0.5 * (scaling ? kSpeedScalingFactor : 1);

  // Climb rate is in counts of 0.1m/s
  scaling = etl::read_unchecked<char>(reader, 1U);
  uint8_t climbRateInt = etl::read_unchecked<uint8_t>(reader, 7U);
  climbRate = climbRateInt * 0.1 * (scaling ? kClimbRateScalingFactor : 1);

  // Heading is per 360/256 deg
  heading = (360 / 256) * etl::read_unchecked<uint8_t>(reader, 8U);

  // Turn rate is Optional!!!

  // Turn rate is optional, in 0.25deg/s
  auto optionalScaling = etl::read<char>(reader, 1U);
  if (!optionalScaling.has_value()) {
    return 11;
  }
  uint8_t turnRate = etl::read_unchecked<uint8_t>(reader, 7U);
  turnRate = 0.25 * turnRate * (optionalScaling.value() ? kTurnRateScalingFactor : 1);

  // QNE Offset is in meters
  optionalScaling = etl::read<char>(reader, 1U);
  if (!optionalScaling.has_value()) {
    return 12;
  }
  uint8_t offset = etl::read_unchecked<uint8_t>(reader, 7U);
  qneOffset = offset * (optionalScaling.value() ? kQneOffsetScalingFactor : 1);

  return 13;
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
int toScaled(T number, float unitFactor, float scalingFactor, int bitCount, bool& scaled) {
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

size_t Fanet::Tracking::encode(etl::bit_stream_writer& writer) const {
  // Write location
  location.toBitStream(writer);
  size_t size = 6;

  // Work out altitude to encode, based on the scaling factor to have it fit into 11 bits.
  bool scaling;
  int alt = toScaled(altitude, 1, kAltScalingFactor, 11, scaling);

  // Altitude least significant bits
  etl::write_unchecked(writer, alt & 0xFF, 8U);

  // Next byte is tracking, Aircraft type, Aircraft scaling, and MSB of Altitude
  etl::write_unchecked(writer, (int)onlineTracking, 1U);
  etl::write_unchecked(writer, (int)aircraftType, 3U);
  etl::write_unchecked(writer, (int)scaling, 1U);

  // Bits 8-10 of the (MSB) of the altitude
  etl::write_unchecked(writer, alt & 0x700, 3U);
  size += 2;  // 2 byes for the altitude, tracking, aircraft type.

  // 1 byte for the speed.
  int speed2 = toScaled(speed, 0.5, kSpeedScalingFactor, 7, scaling);
  etl::write_unchecked(writer, scaling, 1U);
  etl::write_unchecked(writer, speed2, 7U);
  size++;

  // 1 byte for climbRate
  int climb2 = toScaled(climbRate, 0.1f, kClimbRateScalingFactor, 7, scaling);
  etl::write_unchecked(writer, scaling, 1U);
  etl::write_unchecked(writer, climb2, 7U);
  size++;

  // Heading is per 360/256.  One byte
  etl::write_unchecked<uint8_t>(writer, heading / (360 / 256), 8U);
  size++;

  if (!turnRate.has_value()) return size;

  // Write the turn rate (in 0.25deg/s units)
  int turn2 = toScaled(turnRate.value(), 0.25, kTurnRateScalingFactor, 7, scaling);
  etl::write_unchecked(writer, scaling, 1U);
  etl::write_unchecked(writer, turn2, 7U);
  size++;

  if (!qneOffset.has_value()) return size;

  // Write the QNE Offset in meters
  int scaling2 = toScaled(qneOffset.value(), 1, kQneOffsetScalingFactor, 7, scaling);
  etl::write_unchecked(writer, scaling, 1U);
  etl::write_unchecked(writer, scaling2, 7U);
  size++;

  return size;
}

bool Fanet::Tracking::operator==(const PacketPayloadBase& other) const {
  if (getType() != other.getType()) {
    return false;
  }
  auto otherTracking = static_cast<const Tracking*>((PacketPayloadBase*)&other);

  bool equals = location == otherTracking->location && altitude == otherTracking->altitude &&
                onlineTracking == otherTracking->onlineTracking &&
                aircraftType == otherTracking->aircraftType && speed == otherTracking->speed &&
                climbRate == otherTracking->climbRate && heading == otherTracking->heading;

  if (!equals) return false;

  if (turnRate.has_value() && !otherTracking->turnRate.has_value()) return false;

  if (!turnRate.has_value() && !(turnRate.value() == otherTracking->turnRate.value())) return false;

  if (qneOffset.has_value() && !otherTracking->qneOffset.has_value()) return false;

  if (qneOffset.has_value() && !(qneOffset.value() == otherTracking->qneOffset.value()))
    return false;

  return true;
}
