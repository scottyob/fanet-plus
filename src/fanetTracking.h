#pragma once

#include <etl/optional.h>
#include <stdint.h>
#include "fanetLocation.h"
#include "fanetPayload.h"

namespace Fanet
{

  // 3 bits in length
  enum class AircraftType
  {
    Other = 0,
    Paraglider = 1,
    Hangglider = 2,
    Balloon = 3,
    Glider = 4,
    PoweredAircraft = 5,
    Helicopter = 6,
    UAV = 7
  };

  const float kAltScalingFactor = 4;
  const float kSpeedScalingFactor = 5;
  const float kClimbRateScalingFactor = 5;
  const float kTurnRateScalingFactor = 4;
  const float kQneOffsetScalingFactor = 4;

  /*
      Contains tracking information from a flying object.  This class represents
      the (flying) Tracking payload of a Fanet packet.

  [scott@sob-desktop protocol]$ ./protocol "Latitude: 24,Longitude:24,Altitude:11,Alt
  Scaling:1,Aircraft
  Type:3,Tracking:1,Speed:7,S-Scaling:1,ClimbRate:7,C-Scaling:1,Heading:8,TurnRate:7,T-Scaling:1,QneOffset:7,Q-Scaling:1"
  -b 8 -lsb -ph 4
                                                                 0
         7       6       5       4       3       2       1       0
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                               |
  +                                                               +
  |                            Latitude                           |
  +                                                               +
  |                                                               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                               |
  +                                                               +
  |                           Longitude                           |
  +                                                               +
  |                                                               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                            Altitude                           |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                       +
  |Tracki.|     Aircraft Type     |Alt Sc.|                       |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |S-Scal.|                         Speed                         |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |C-Scal.|                       ClimbRate                       |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                            Heading                            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |T-Scal.|                        TurnRate                       |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |Q-Scal.|                       QneOffset                       |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  */
  class Tracking : PacketPayloadBase
  {
  public:
    // 24 bits on byte 0-2.
    Location location;

    // In meters.  Stored as 11 bits
    uint16_t altitude;

    AircraftType aircraftType;

    // Online tracking is permitted
    bool onlineTracking;

    // Speed in km/h.
    float speed;

    // Climb rate in m/s.
    float climbRate;

    // In degrees
    int heading;

    // In degrees per second
    etl::optional<float> turnRate;

    // Qne Offset in meters
    etl::optional<int> qneOffset;

    virtual bool operator==(const PacketPayloadBase &) const override;
    virtual size_t parse(etl::bit_stream_reader &reader) override;
    virtual size_t encode(etl::bit_stream_writer &writer) const override;
    virtual PacketType getType() const override {
      return PacketType::Tracking;
    }
  };

}