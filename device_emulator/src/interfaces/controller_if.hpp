#ifndef DEMU_CONTROLLER_IF_HPP
#define DEMU_CONTROLLER_IF_HPP

#include <string>
#include "environment.hpp"
#include <crate/control/action_v1.hpp>

namespace demu {
   enum class controller_type_e {
      FIRE_EXTINGUISHER
   };

   static std::string sensor_type_to_string(const controller_type_e type) {
      switch (type) {
         case controller_type_e::FIRE_EXTINGUISHER:  return "FIRE_EXTINGUISHER";
      }
      return "UNKNOWN";
   }

   class controller_if {
   public:
      controller_if() = delete;
      controller_if(std::string controller_id, environment &env, const controller_type_e type, std::string uuid) : 
         _controller_id(controller_id), _env(env), _type(type), _uuid(uuid)  {}
      virtual ~controller_if(){}
      virtual bool execute_action(crate::control::action_v1_c action) = 0;
      controller_type_e get_type() const { return _type; }
      std::string get_controller_id() const { return _controller_id; }
      std::string get_uuid() const { return _uuid; }
      int64_t get_timestamp() {
         return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
         ).count();
      }
   protected:
      environment &_env;
   private:
      controller_type_e _type;
      std::string _controller_id;
      std::string _uuid;
   };

} // namespace demu

#endif