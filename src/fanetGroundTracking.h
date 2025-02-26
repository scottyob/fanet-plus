#pragma once
#include <stddef.h>
#include <stdint.h>
#include "etl/enum_type.h"
#include "fanetLocation.h"
#include "fanetPayload.h"

namespace Fanet {

  struct GroundTrackingType {
    enum enum_type {
      Other = 0,
      Walking = 1,
      Vehicle = 2,
      Bike = 3,
      Boot = 4,
      NeedARide = 8,
      LandedWell = 9,
      NeedTechnicalSupport = 12,
      NeedMedicalHelp = 13,
      DistressCall = 14,
      DistressCallAutomatically = 15
    };

    ETL_DECLARE_ENUM_TYPE(GroundTrackingType, uint8_t)
    ETL_ENUM_TYPE(Other, "Other")
    ETL_ENUM_TYPE(Walking, "Walking")
    ETL_ENUM_TYPE(Vehicle, "Vehicle")
    ETL_ENUM_TYPE(Bike, "Bike")
    ETL_ENUM_TYPE(Boot, "Boot")
    ETL_ENUM_TYPE(NeedARide, "NeedARide")
    ETL_ENUM_TYPE(LandedWell, "LandedWell")
    ETL_ENUM_TYPE(NeedTechnicalSupport, "NeedTechnicalSupport")
    ETL_ENUM_TYPE(NeedMedicalHelp, "NeedMedicalHelp")
    ETL_ENUM_TYPE(DistressCall, "DistressCall")
    ETL_ENUM_TYPE(DistressCallAutomatically, "DistressCallAutomatically")
    ETL_END_ENUM_TYPE
  };

  /*
                                                                 0
         7       6       5       4       3       2       1       0
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                               |
  +                              (lsb)                            +
  |                            Latitude                           |
  +                              (msb)                            +
  |                                                               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                               |
  +                                                               +
  |                           Longitude                           |
  +                                                               +
  |                                                               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |          Ground Type          |        Reserved       |Tracki.|
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  */
  /// @brief Packet payload for encoding Ground Tracking
  class GroundTracking : PacketPayloadBase {
   public:
    bool shouldTrackOnline = false;
    GroundTrackingType type = GroundTrackingType::Other;
    Location location;

    size_t parse(etl::bit_stream_reader& reader) override;
    size_t encode(etl::bit_stream_writer& writer) const override;
    bool operator==(const PacketPayloadBase&) const override;
    PacketType getType() const override;
  };

}  // namespace Fanet