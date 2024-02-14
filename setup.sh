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

#Checks system version and sets compatible ROS version
check_system_version(){
    if [[ "${OSVersion}" == "16.04" ]]; then
        ROS_Ver="kinetic"
    elif [[ "${OSVersion}" == "18.04" ]]; then
        ROS_Ver="melodic"
    elif [[ "${OSVersion}" == "20.04" ]]; then
        ROS_Ver="noetic"
    else
       echo -e "${Error} PROBOT don't support ${OSDescription} !" && exit 1
    fi
}

#Checks if there is already any version of ROS installed
check_ros_version(){
	echo -e "${Tip} The system version: ${OSDescription}" 
	if [ -f "/usr/bin/rosversion" ]; then
		ROSVER=`/usr/bin/rosversion -d`
		if [ $ROSVER ]; then
			echo -e "${Tip} The ROS version: ${ROSVER}"
			return
		fi 
	fi
	echo -e "${Error} ROS has not installed in this computer, please install ROS." 
}

#Install ROS
install_ros(){
    #Check if it is already installed        
	if [ -f "/usr/bin/rosversion" ]; then
		ROSVER=`/usr/bin/rosversion -d`
		if [ $ROSVER ]; then
			echo -e "${Tip} The ROS ${ROSVER} has been installed!" 
			echo && stty erase ^? && read -p "Do you want to continue？ y/n：" choose
			if [[ "${choose}" == "y" ]]; then
				echo -e "${Info}Starting ROS install！" 
			else
				exit
			fi
		fi
	fi

    echo -e "${Green_font_prefix}Install ROS ...${Font_color_suffix}"

    #Install ROS core packages
    sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
    curl -s https://raw.githubusercontent.com/ros/rosdistro/master/ros.asc | sudo apt-key add -
    sudo apt update
	sudo apt install ros-${ROS_Ver}-desktop-full

    #Source environment
    echo "source /opt/ros/${ROS_Ver}/setup.bash" >> ~/.bashrc
	source /opt/ros/${ROS_Ver}/setup.bash
    source ~/.bashrc

    #Dependencies
    sudo apt install python-rosdep python-rosinstall python-rosinstall-generator python-wstool build-essential python-catkin-tools

    #ROSDep
	sudo rosdep init
	rosdep update

    echo -e "${Green_font_prefix}ROS has installed finished.${Font_color_suffix}"
}

#Install Moveit, industrial, gazebo and other packages
install_probot_dependents(){
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

install_probot(){
    echo -e "${Green_font_prefix}Install probot packages ...${Font_color_suffix}"

    if [ ! -d catkin_ws/src/probot_${robot_name} ]; then
    
        cp -R PROBOT-G602/src catkin_ws/
        
        chmod +x catkin_ws/src/probot_${robot_name}/probot_${robot_name}_demo/scripts/*
        chmod +x catkin_ws/src/probot_${robot_name}/probot_driver/bin/*
        chmod +x catkin_ws/src/probot_${robot_name}/probot_driver/scripts/*
    
        cd catkin_ws
        catkin_make

        sed -e '/probot/d' ~/.bashrc > ~/.bashrc.tmp
        mv -f ~/.bashrc.tmp ~/.bashrc
	    echo "source catkin_ws/devel/setup.bash --extend" >> ~/.bashrc
	    echo "export LD_LIBRARY_PATH=catkin_ws/src/probot_${robot_name}/probot_rviz_plugin/lib/moveIt:${LD_LIBRARY_PATH}" >> ~/.bashrc
    
	    source ~/.bashrc
    

    echo -e "Probot packages have been installed."
    fi
    cd ..
}

#Sets catkin workspace and copies files into it. After this, all is set.
set_up_workspace(){

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
}

delete_all(){
    sudo apt-get remove ros-*
    sudo apt-get purge '^ros-*'
    sudo apt remove python-rosdep python-rosinstall python-rosinstall-generator python-wstool build-essential python-catkin-tools
    sudo apt-get autoremove
    sudo rm -rf catkin_ws

}

main()
{
    echo -e "Robot environment Setup ${Red_font_prefix}[${default_version}]${Font_color_suffix}"

    #Installing ROS packages
    check_system_version
    check_ros_version

    #Install ROS
    echo && stty erase ^? && read -p "Install ROS? [y/n]：" choose
    if [[ "${choose}" == "y" ]]; then
        echo -e "${Info}Installing ROS！" 
        install_ros
        install_probot_dependents
    fi

    #Set up catkin workspace
    echo && stty erase ^? && read -p "set up workspace? [y/n]：" choose
    if [[ "${choose}" == "y" ]]; then
        echo -e "${Info}Setting up workspace！" 
        set_up_workspace
    fi

    echo "Probot packages were installed in ${ROS_PACKAGE_PATH}"

    echo && stty erase ^? && read -p "Delete all ROS (DEBUG ONLY) ? [y/n]：" choose
    if [[ "${choose}" == "y" ]]; then
        echo -e "${Info}Hope you know what you're doing, pal！" 
        delete_all
    else
        exit
    fi
 
}

main $*