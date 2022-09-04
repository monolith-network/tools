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
      std::string address;
      uint32_t http_port;
      uint32_t metric_submission_port;
      uint32_t controller_port;
   };

   extern std::optional<configuration> load_config(const std::string& file, environment &env);

} // namespace demu

#endif