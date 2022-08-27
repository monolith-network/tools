cmake_minimum_required(VERSION 3.8)

project(stream_receiver)

set(CMAKE_CXX_STANDARD 20)

find_package (Threads REQUIRED)
find_package (Nettle REQUIRED)
find_package (crate REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(stream_receiver
   main.cpp)

target_link_libraries(stream_receiver
        PRIVATE
        Threads::Threads
        ${CRATE_LIBRARIES}
        ${NETTLE_LIBRARIES})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/configs 
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR})