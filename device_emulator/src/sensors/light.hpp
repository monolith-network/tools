#ifndef DEMU_LIGHT_HPP
#define DEMU_LIGHT_HPP

#include "interfaces/sensor_if.hpp"
#include "environment.hpp"

namespace demu {
class light : public sensor_if{
public:
   light(std::string id, environment& env) : sensor_if(id, env, sensor_type::LIGHT) {}
   virtual crate::metrics::sensor_reading_v1_c get_value() override final {
      return crate::metrics::sensor_reading_v1_c(
         get_timestamp(), 
         get_node_id(), 
         get_uuid(), 
         _env.poll_light());
   }
};

} // namespace demu

#endif