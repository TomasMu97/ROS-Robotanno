search_mode=OPTIMIZE_MAX_JOINT
srdf_filename=probot_g602.srdf
robot_name_in_srdf=probot_g602
moveit_config_pkg=probot_g602_moveit_config
robot_name=probot_g602
planning_group_name=manipulator
ikfast_plugin_pkg=probot_g602_ikfast_manipulator_plugin
base_link_name=base_link
eef_link_name=tool0
ikfast_output_path=/home/hcx/probot_ws/src/probot_6axis_ros/packages/ros_driver/probot_g602_ikfast_manipulator_plugin/src/probot_g602_manipulator_ikfast_solver.cpp

rosrun moveit_kinematics create_ikfast_moveit_plugin.py\
  --search_mode=$search_mode\
  --srdf_filename=$srdf_filename\
  --robot_name_in_srdf=$robot_name_in_srdf\
  --moveit_config_pkg=$moveit_config_pkg\
  $robot_name\
  $planning_group_name\
  $ikfast_plugin_pkg\
  $base_link_name\
  $eef_link_name\
  $ikfast_output_path
