#pragma once

#include "etl/optional.h"
#include "fanetGroundTracking.h"
#include "fanetLocation.h"
#include "fanetMac.h"

namespace Fanet {

  /// @brief Neighbor entry
  struct Neighbor {
    etl::optional<Location> location = etl::nullopt;
    etl::optional<uint32_t> altitude = etl::nullopt;
    etl::optional<GroundTrackingType> groundTrackingType = etl::nullopt;
    Mac address;
    float rssi = 0.0f;
    float snr = 0.0f;
    unsigned long lastSeen = 0;
  };

}  // namespace Fanet