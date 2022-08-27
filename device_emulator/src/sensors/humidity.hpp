#ifndef DEMU_HUMIDITY_HPP
#define DEMU_HUMIDITY_HPP

#include "interfaces/sensor_if.hpp"
#include "environment.hpp"

namespace demu {
class humidity : public sensor_if{
public:
   humidity(std::string id, environment& env) : sensor_if(id, env, sensor_type::HUMIDITY) {}
   virtual crate::metrics::sensor_reading_v1 get_value() override final {
      return crate::metrics::sensor_reading_v1(
         get_timestamp(), 
         get_node_id(), 
         get_uuid(), 
         _env.poll_humidity());
   }
};

} // namespace demu

#endif