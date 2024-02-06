cmake_minimum_required(VERSION 3.10)
project(probot_g602_demo)

# Find ROS package and libraries
find_package(catkin REQUIRED COMPONENTS
moveit_core
moveit_ros_perception
moveit_ros_planning_interface
#probot_msgs
#trajectory_msgs
roscpp
rospy
)

# Specify the include directories
include_directories(
  ${catkin_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/json/include/"
  # Add other include directories as needed
)

# Add executable target
add_executable(MQTT-ROS_Bridge {CMAKE_CURRENT_SOURCE_DIR}/MQTT_bridge_src/simulated_server.cpp)
target_include_directories(MQTT-ROS_Bridge PUBLIC 
"${CMAKE_CURRENT_SOURCE_DIR}/json/single_include/"
"${CMAKE_CURRENT_SOURCE_DIR}/mosquitto/include/"
"${CMAKE_CURRENT_SOURCE_DIR}/MQTT_bridge_src"
)

# Link ROS libraries
target_link_libraries(MQTT-ROS_Bridge ${catkin_LIBRARIES})

# Link other libraries (e.g., Mosquitto, JSON)
target_link_libraries(MQTT-ROS_Bridge 
"{CMAKE_CURRENT_SOURCE_DIR}/lib/libmosquitto_static.a"
"{CMAKE_CURRENT_SOURCE_DIR}/json/single_include/nlohmann/json.hpp"
"{CMAKE_CURRENT_SOURCE_DIR}/mosquitto/include/mosquitto.h"
"{CMAKE_CURRENT_SOURCE_DIR}/MQTT_bridge_src"

)

# Install rules
install(TARGETS MQTT-ROS_Bridge
  RUNTIME DESTINATION {CMAKE_CURRENT_SOURCE_DIR}
)
