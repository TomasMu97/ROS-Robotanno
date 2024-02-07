# ROS-Robotanno
This is the first attempt to run a ROS-driven Robotanno g602 using @adalbertocajueiro MQTT adaptation

# Dependences and system requirements
This code has been developed in Ubuntu 18.04.6 LTS. 
    ## It is required to have installed:
    - Curl
    ## It is optional to have installed:
    - ROS melodic
    - Catkin
    - Rviz, gazebo, Moveit

# Setting up the workspace
After cloning this repo, setup.sh can be run to install ROS (if it wasn't installed) and set up the workspace. After that, command catkin_make can be run in catkin_ws to compile the program. Please check user permissions (I had to run "sudo chown $USER: -R catkin_ws/") before compiling.
