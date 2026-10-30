cmake_minimum_required(VERSION 3.8)
project(px4_offboard_test)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(px4_msgs REQUIRED)

add_executable(offboard_test src/offboard_test.cpp)
ament_target_dependencies(offboard_test rclcpp px4_msgs)

install(
  TARGETS offboard_test
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
