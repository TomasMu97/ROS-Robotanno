#!/usr/bin/env bash

#=================================================
#	System Required: Ubuntu 16.04
#	Description: Install ROS And PROBOT
#	Version: 2.1.0
#	Author: ps-micro
#	Site: http://www.ps-micro.com/
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

check_system_version()
{
    if [[ "${OSVersion}" == "16.04" ]]; then
        ROS_Ver="kinetic"
    elif [[ "${OSVersion}" == "18.04" ]]; then
        ROS_Ver="melodic"
    else
       echo -e "${Error} PROBOT don't support ${OSDescription} !" && exit 1
    fi
}


check_ros_version()
{
	echo -e "${Tip} The system version: ${OSDescription}" 
	if [ -f "/usr/bin/rosversion" ]; then
		ROSVER=`/usr/bin/rosversion -d`
		if [ $ROSVER ]; then
			echo -e "${Tip} The ROS version: ${ROSVER}"
			return
		fi 
	fi
	echo -e "${Error} ROS has not installed in this computer, please install ROS with 1." 
}


install_probot_dependents()
{
    echo -e "${Green_font_prefix}Install probot dependent packages ...${Font_color_suffix}"

    sudo apt-get -y install ros-${ROS_Ver}-moveit-*
    sudo apt-get -y install ros-${ROS_Ver}-industrial-*
    sudo apt-get -y install ros-${ROS_Ver}-gazebo-ros-control
    sudo apt-get -y install ros-${ROS_Ver}-ros-control ros-${ROS_Ver}-ros-controllers
    sudo apt-get -y install ros-${ROS_Ver}-trac-ik-kinematics-plugin
    sudo apt-get -y install ros-${ROS_Ver}-usb-cam
    sudo apt-get -y install ros-${ROS_Ver}-realsense-camera ros-${ROS_Ver}-realsense2-camera
    sudo apt-get -y install ros-${ROS_Ver}-visp-hand2eye-calibration
    sudo apt-get -y install ros-${ROS_Ver}-robot-localization ros-${ROS_Ver}-interactive-marker-twist-server ros-${ROS_Ver}-twist-mux ros-${ROS_Ver}-hector-gazebo
    sudo apt-get -y install python-sklearn
    sudo apt install ros-${ROS_Ver}-lms1xx
    sudo apt-get -y install python3-pip ros-${ROS_Ver}-mavros
    sudo pip3 install pyyaml numpy rospkg autobahn

    echo -e "${Green_font_prefix}Dependent packages have installed finished.${Font_color_suffix}"
}


main()
{
    echo -e "Robot environment Setup ${Red_font_prefix}[${default_version}]${Font_color_suffix}"

    #Installing ROS packages
    #check_system_version
    #check_ros_version
    #install_probot_dependents

    #Installing catking workspace
    #cd ../../
    mkdir -p catkin_ws/src
    cp  -r MQTT_bridge_src catkin_ws/src/MQTT_bridge_src 
    cp CMakeLists.txt catkin_ws/src/CMakeLists.txt     
    cd catkin_ws/src
    catkin_init_workspace

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

 
}

main $*