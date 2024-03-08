cmake_minimum_required(VERSION 3.0)

project(gpio_monitor)

find_package(gpiod REQUIRED PATHS /usr/local/libgpiod/cmake)
find_package(MariaDB REQUIRED)

include_directories(${gpiod_INCLUDE_DIRS})
include_directories(${MARIADB_INCLUDE_DIRS})

add_executable(gpio_monitor gpio_monitor.c)

target_link_libraries(gpio_monitor ${gpiod_LIBRARIES} ${MARIADB_LIBRARIES})
