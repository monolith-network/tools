#ifndef DEMU_ENVIRONMENT
#define DEMU_ENVIRONMENT

#include "types.hpp"

#include <cstdint>
#include <vector>
#include <crate/networking/message_server.hpp>
#include <chrono>

namespace demu {

class environment {
public:
   void configure(std::vector<environment_details> details);
   double poll_temperature();
   double poll_humidity();
   uint32_t poll_air_pressure();
   uint8_t poll_light();
   uint8_t poll_flames();
   bool is_active();

   void extinguish_flames();
private:
   static constexpr double FIRE_START_COOLDOWN_SEC = 15.0;

   std::vector<environment_details> _details;
   struct levels {
      double temperature {70.0};
      double humidity {10.0};
      double pressure {101.3};
      uint8_t light {0};
      uint8_t flames {0};
      bool activity {false};
   };
   struct ranges {
      double temp_low{50.0};
      double temp_high{120.0};
      double humidity_low{0.00};
      double humidity_high{100.0};
      double pressure_low {80.5};
      double pressure_high {115.3};
      uint32_t light_low {0};
      uint32_t light_high {10};
      uint8_t flames_low {0};
      uint8_t flames_high {10};
      uint8_t activity_prob {5};
   };
   levels level;
   ranges range;
   bool has_fire {false};
   bool is_hot{false};
   bool is_dry{false};
   void update();

    std::chrono::time_point<std::chrono::steady_clock> _last_fire_extinguish;
};

} // namespace demu

#endif