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
## Step 1: setup.sh
Launching this script will ask if you want to install ROS (if you haven't installed it beforehand) and probot_g602 packages. It also has a debug only feature that removes all ROS and probot_g602 packages, as well as catkin workspaces.
## Step 2: executable.sh
This script will make the same workspace that is used in ED-Scorbot and ED-Robotanno (mosquitto and json libraries) and compile them into a MQTT-ROS bridge executable to be run alongside the web app to control a robot.
## Step 3: launching the probot package
By launching command "roslaunch probot_bringup probot_g602_bringup.launch sim:=true" we will be running RVIZ with a simulation of our robot. Then, running the bridge executable will open a ROS node that will communicate with the nodes run by the package probot_bringup. 
## Step 4: Servitization!
We can now run our microservices and control our simulated robot via web app. Microservices order is: 
- java, python
- angular
- MQTT-ROS executable

