/***********************************************************************
Copyright 2019 Wuhan PS-Micro Technology Co., Itd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
***********************************************************************/

#include <ros/ros.h>
#include <moveit/move_group_interface/move_group_interface.h>
//#include "/home/tomas/probot_g602_ws/src/probot_g602/probot_g602_demo/src_test/communication-layer.cpp"
#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include "communication-layer.cpp"
//#include "argparse.hpp"
//#include "nholmann/json.hpp"
#include <unistd.h>
#include <signal.h>


std::vector<double> Point_FK;
std::string reference_frame = "base_link";


/*

void parse_arguments(int argc, char *argv[]){

  // Declare parser
  argparse::ArgumentParser parser("mqtt_rbtanno");

  //Add required arguments
    parser.add_argument("-j", "--joints").nargs(7).help("Define values of joints to move.").required();
        

  try {
    parser.parse_args(argc, argv);
  }
  catch (const std::runtime_error &err) {
    std::cout << "RUNTIME ERROR!!!!" << std::endl;
    std::exit(-1);
  }

    // Access the parsed values
    std::vector<std::string> paramStrings = parser.get<std::vector<std::string>>("--joints");

    for (const auto& str : paramStrings) {
        Point_FK.push_back(std::stod(str));
    }

}
*/


//TODO: Iniciar toda la comunicacion con el robot desde aquí. No depender de RViz.
std::vector<double> MoveToPoint (std::vector<double> Point_target){
  
  //Init ros node 
  ros::VP_string remappings;
  remappings.push_back(std::make_pair("foo","bar"));
  ros::init(remappings, "moveit_fk");
  ros::AsyncSpinner spinner(1);
  spinner.start();
  moveit::planning_interface::MoveGroupInterface arm("manipulator");

  //Set movement parameters - For now, joint speed is fixed to 10% of max
  arm.setPoseReferenceFrame(reference_frame);
  arm.allowReplanning(true);
  arm.setGoalJointTolerance(0.001);
  arm.setGoalJointTolerance(0.001);
  arm.setMaxAccelerationScalingFactor(0.1);
  arm.setMaxVelocityScalingFactor(0.1);

  //Set desired position
  std::vector<double> joint_group_positions(6);
  for (int i = 0; i < 6; i++){
	joint_group_positions[i]=Point_target[i]*3.14159265/180.0;
	}

  //Move target
  arm.setJointValueTarget(joint_group_positions);
  arm.move();
  sleep(Point_target[6]/1000);

  //Get current position
  Point_FK = arm.getCurrentJointValues();
  for (int i = 0; i < 6; i++){
	Point_FK[i]=Point_FK[i]*180.0/3.14159265;
	}

  /* Print current position (debug feature, to be erased later)
  std::cout << "Current Values: ";
  for (const auto& value : Point_FK) {
  std::cout << value << " ";
  }
  */

  ros::shutdown();

  return Point_FK;
}

std::vector<double> Home(void)
{
  ros::VP_string remappings;
  remappings.push_back(std::make_pair("foo","bar"));

  ros::init(remappings, "moveit_home");
  ros::AsyncSpinner spinner(1);
  spinner.start();

  moveit::planning_interface::MoveGroupInterface arm("manipulator");

  //Set speed and acceleration parameters
  arm.setPoseReferenceFrame(reference_frame);
  arm.allowReplanning(true);
  arm.setGoalJointTolerance(0.001);
  arm.setMaxAccelerationScalingFactor(0.1);
  arm.setMaxVelocityScalingFactor(0.1);

  //Set goal: home
  arm.setNamedTarget("home");
  arm.move();
  sleep(1);

  //Get current position
  Point_FK = arm.getCurrentJointValues();

  //Print current position (debug feature, to be erased later)
  std::cout << "Current Values: ";
  for (const auto& value : Point_FK) {
  std::cout << value << " ";
  }

  ros::shutdown();

  return Point_FK;

}

//std::vector<std::vector<double>>
void                                executeTrajectory(const std::vector<std::vector<double>>& target_trajectory) {

  // Initialize ROS
  ros::VP_string remappings;
  remappings.push_back(std::make_pair("foo","bar"));
  ros::init(remappings, "moveit_trajectory");
  ros::AsyncSpinner spinner(1);
  spinner.start();
  moveit::planning_interface::MoveGroupInterface arm("manipulator");
  std::string end_effector_link = arm.getEndEffectorLink();
  
  //Set Robot parameters
  std::string reference_frame = "base_link";
  arm.setPoseReferenceFrame(reference_frame);
  arm.allowReplanning(true);
  arm.setGoalJointTolerance(0.001);
  arm.setMaxAccelerationScalingFactor(0.1);
  arm.setMaxVelocityScalingFactor(0.1);

  // Set up trajectory execution parameters
  arm.setPlanningTime(5.0);  // Set your planning time

  // Define the vector to store reached points
  std::vector<std::vector<double>> reached_points;

  // Iterate through each target point in the trajectory
  for (const auto& target_point : target_trajectory) {
    // Set the joint values for the target point
    arm.setJointValueTarget(target_point);

    // Plan the trajectory
    moveit::planning_interface::MoveGroupInterface::Plan plan;
    bool success = (arm.plan(plan) == moveit::planning_interface::MoveItErrorCode::SUCCESS);

    if (success) {
      // Execute the trajectory
      arm.execute(plan);

      // Wait for the specified delay
      int delay_ms = static_cast<int>(target_point.back());
      ros::Duration(delay_ms / 1000.0).sleep();

      // Get and store the real joint positions
      std::vector<double> current_joint_positions = arm.getCurrentJointValues();
      reached_points.push_back(current_joint_positions);
    } else {
      ROS_WARN("Failed to plan for the target point.");
    }
  }

  // Shut down the ROS node
  ros::shutdown();

  // Return the reached points
  //return reached_points;
}




