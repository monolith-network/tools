#ifndef DEMU_AIR_PRESSURE_HPP
#define DEMU_AIR_PRESSURE_HPP

#include "interfaces/sensor_if.hpp"
#include "environment.hpp"

namespace demu {

class air_pressure : public sensor_if{
public:
   air_pressure(std::string id, environment& env) : sensor_if(id, env, sensor_type::AIR_PRESSURE, "0c23501a-3fb6-42f5-88eb-071e7a74b7d6") {}
   virtual crate::metrics::sensor_reading_v1_c get_value() override final {
      return crate::metrics::sensor_reading_v1_c(
         get_timestamp(), 
         get_node_id(), 
         get_uuid(), 
         _env.poll_air_pressure());
   }
};

} // namespace demu

#endif