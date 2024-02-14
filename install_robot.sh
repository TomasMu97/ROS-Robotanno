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

    echo -e "${Green_font_prefix}Install probot packages ...${Font_color_suffix}"

    if [ ! -d ~/probot_${robot_name}_ws ]; then
        mkdir ~/probot_${robot_name}_ws
        mkdir ~/probot_${robot_name}_ws/src
        #mkdir ~/probot_${robot_name}_ws/src/probot_${robot_name}
        cp -R PROBOT-G602/src ~/probot_${robot_name}_ws
        
        chmod +x ~/probot_${robot_name}_ws/src/probot_${robot_name}/probot_${robot_name}_demo/scripts/*
        chmod +x ~/probot_${robot_name}_ws/src/probot_${robot_name}/probot_driver/bin/*
        chmod +x ~/probot_${robot_name}_ws/src/probot_${robot_name}/probot_driver/scripts/*

        cd ~/probot_${robot_name}_ws
        catkin_make

        sed -e '/probot/d' ~/.bashrc > ~/.bashrc.tmp
        mv -f ~/.bashrc.tmp ~/.bashrc
	    echo "source ~/probot_${robot_name}_ws/devel/setup.bash --extend" >> ~/.bashrc
	    echo "export LD_LIBRARY_PATH=~/probot_${robot_name}_ws/src/probot_${robot_name}/probot_rviz_plugin/lib/moveIt:${LD_LIBRARY_PATH}" >> ~/.bashrc

	    source ~/.bashrc

        echo -e "${Green_font_prefix}PROBOT has installed to $(pwd) Please have a happy journey!${Font_color_suffix}"
    echo -e "${Green_font_prefix}Probot was installed."
    else
        echo -e "${Red_font_prefix}The probot_${robot_name}_ws folder has existed, please delete and reinstall!${Font_color_suffix}"
    fi