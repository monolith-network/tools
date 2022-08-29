#ifndef DEMU_MOTION_HPP
#define DEMU_MOTION_HPP

#include "interfaces/sensor_if.hpp"
#include "environment.hpp"

namespace demu {
class motion : public sensor_if{
public:
   motion(std::string id, environment& env) : sensor_if(id, env, sensor_type::MOTION) {}
   virtual crate::metrics::sensor_reading_v1_c get_value() override final {
      return crate::metrics::sensor_reading_v1_c(
         get_timestamp(), 
         get_node_id(), 
         get_uuid(), 
         _env.is_active());
   }
};

} // namespace demu

#endif