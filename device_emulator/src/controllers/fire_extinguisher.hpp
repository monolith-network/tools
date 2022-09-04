#ifndef DEMU_FIRE_EXTINGUISHER_HPP
#define DEMU_FIRE_EXTINGUISHER_HPP

#include "interfaces/controller_if.hpp"
#include "environment.hpp"

#include <iostream>

namespace demu {

class fire_extinguisher : public controller_if{
public:
   fire_extinguisher(std::string id, environment& env) : controller_if(id, env, controller_type_e::FIRE_EXTINGUISHER, "fire_extinguisher_controller") {}
   virtual bool execute_action(crate::control::action_v1_c action) override final {
      
      auto [ts, cid, aid, value] = action.get_data();

      if (aid == "toggle_extinguisher") {
         if (value > 0.0) {
            _env.extinguish_flames();
         }
         return true;
      }

      std::cout << get_uuid() << "> Unknown action id: " << aid << std::endl;
      return false;
   }
};

} // namespace demu

#endif