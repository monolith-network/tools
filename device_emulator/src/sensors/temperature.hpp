#ifndef DEMU_TEMPERATURE_HPP
#define DEMU_TEMPERATURE_HPP

#include "interfaces/sensor_if.hpp"
#include "environment.hpp"

namespace demu {
class temperature : public sensor_if{
public:
   temperature(std::string id, environment& env) : sensor_if(id, env, sensor_type::TEMPERATURE, "58eb7f04-30f3-4a0c-94b7-c6f140870d1d") {}
   virtual crate::metrics::sensor_reading_v1_c get_value() override final {
      return crate::metrics::sensor_reading_v1_c(
         get_timestamp(), 
         get_node_id(), 
         get_uuid(), 
         _env.poll_temperature());
   }
};

} // namespace demu

#endif