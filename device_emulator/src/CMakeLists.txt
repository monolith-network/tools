cmake_minimum_required(VERSION 3.8)

project(demu)

set(CMAKE_CXX_STANDARD 20)

find_package (Threads REQUIRED)
find_package (libutil REQUIRED)
find_package (Nettle REQUIRED)
find_package (crate REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(demu
   config.cpp
   environment.cpp
   demu.cpp)

target_link_libraries(demu
        PRIVATE
        Threads::Threads
        libutil
        ${CRATE_LIBRARIES}
        ${NETTLE_LIBRARIES})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/configs 
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR})