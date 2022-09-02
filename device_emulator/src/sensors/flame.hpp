#ifndef DEMU_FLAME_HPP
#define DEMU_FLAME_HPP

#include "interfaces/sensor_if.hpp"
#include "environment.hpp"

namespace demu {
class flame : public sensor_if{
public:
   flame(std::string id, environment& env) : sensor_if(id, env, sensor_type::FLAME, "f360c409-2c4d-420f-bf2d-56c9056d6583") {}
   virtual crate::metrics::sensor_reading_v1_c get_value() override final {
      return crate::metrics::sensor_reading_v1_c(
         get_timestamp(), 
         get_node_id(), 
         get_uuid(), 
         _env.poll_flames());
   }
};

} // namespace demu

#endif