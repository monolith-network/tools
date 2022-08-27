#ifndef DEMU_CONFIG
#define DEMU_CONFIG

#include <string>
#include <optional>
#include <vector>

#include "interfaces/sensor_if.hpp"
#include "types.hpp"
#include "environment.hpp"

namespace demu {

   struct configuration {
      std::string device_name;
      std::string location_name;
      std::vector<sensor_if*> sensors;
      std::vector<environment_details> env_details;
      std::string submission_server_address;
      short submission_server_port;
      std::string registrar_address;
      short registrar_port;
   };

   extern std::optional<configuration> load_config(const std::string& file, environment &env);

} // namespace demu

#endif