#!/usr/bin/env bash

#=================================================
#	System Required: Ubuntu 18.04
#	Description: Install ROS, Catkin And PROBOT
#	Version: 1.0
#	Author: tmunoz1
#=================================================

robot_name="g602"
default_version="v2.1.0"

Green_font_prefix="\033[32m"
Red_font_prefix="\033[31m"
Green_background_prefix="\033[42;37m"
Red_background_prefix="\033[41;37m"
Font_color_suffix="\033[0m"

Info="${Green_font_prefix}[Info]${Font_color_suffix}"
Error="${Red_font_prefix}[Error]${Font_color_suffix}"
Tip="${Green_font_prefix}[Warn]${Font_color_suffix}"

OSVersion=$(lsb_release -r --short)
OSDescription=$(lsb_release -d --short)

    #Installing catking workspace
    #cd ../../
    mkdir -p catkin_ws/src
    catkin_init_workspace
    #install_probot
    sudo cp  -r MQTT_bridge_src catkin_ws/src/MQTT_bridge_src 
    sudo cp CMakeLists.txt catkin_ws/src/CMakeLists.txt     
    cd catkin_ws/src


    # Installing MQTT/JSON
    #cd src
    git clone https://github.com/eclipse/mosquitto
    git clone https://github.com/nlohmann/json/
    cd mosquitto
    rm -rf build
    mkdir build
    cd build
    cmake .. -DWITH_STATIC_LIBRARIES=ON -DWITH_PIC=ON -DWITH_TLS=OFF -DDOCUMENTATION=OFF
    make -j4

    ls -l lib
    mkdir ../../lib
    cp lib/libmosquitto_static.a ../../lib/libmosquitto_static.a

    #lets make our executable
    cd ../../..
    catkin_make