#ifndef DEMU_SENSOR_IF_HPP
#define DEMU_SENSOR_IF_HPP

#include <chrono>
#include <string>
#include "types.hpp"
#include "environment.hpp"
#include <crate/metrics/reading_v1.hpp>
#include <crate/externals/uuid4/uuid4.h>

namespace demu {
   enum class sensor_type {
      TEMPERATURE,
      MOTION,
      HUMIDITY,
      FLAME,
      LIGHT,
      AIR_PRESSURE
   };

   static std::string sensor_type_to_string(const sensor_type type) {
      switch (type) {
         case sensor_type::TEMPERATURE:  return "TEMPERATURE";
         case sensor_type::MOTION:       return "MOTION";
         case sensor_type::HUMIDITY:     return "HUMIDITY";
         case sensor_type::FLAME:        return "FLAME";
         case sensor_type::LIGHT:        return "LIGHT";
         case sensor_type::AIR_PRESSURE: return "AIR_PRESSURE";
      }
      return "UNKNOWN";
   }

   class sensor_if {
   public:
      sensor_if() = delete;
      sensor_if(std::string node_id, environment &env, const sensor_type type) : 
         _node_id(node_id), _env(env), _type(type)  {
            char u[UUID4_LEN];
            uuid4_init();
            uuid4_generate(u);
            _uuid = std::string(u);
         }
      virtual ~sensor_if(){}
      virtual crate::metrics::sensor_reading_v1_c get_value() = 0;
      sensor_type get_type() const { return _type; }
      std::string get_node_id() const { return _node_id; }
      std::string get_uuid() const { return _uuid; }
      int64_t get_timestamp() {
         return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
         ).count();
      }
   protected:
      environment &_env;
   private:
      sensor_type _type;
      std::string _node_id;
      std::string _uuid;
   };
}

#endif