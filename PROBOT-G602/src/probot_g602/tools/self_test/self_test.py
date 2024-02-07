#!/usr/bin/env python

import rospy, sys
import moveit_commander
import time

from moveit_msgs.msg import RobotTrajectory
from trajectory_msgs.msg import JointTrajectoryPoint

from geometry_msgs.msg import PoseStamped, Pose
from tf.transformations import euler_from_quaternion, quaternion_from_euler

from probot_msgs.msg import SetOutputIO

def moveSingleJoint(index, value):
        group_variable_values = arm.get_current_joint_values()
        group_variable_values[index] = value
        arm.set_joint_value_target(group_variable_values)
        traj = arm.plan()
        arm.execute(traj)

if __name__ == "__main__":

    moveit_commander.roscpp_initialize(sys.argv)
                
    arm = moveit_commander.MoveGroupCommander('manipulator')
                
    end_effector_link = arm.get_end_effector_link()
                       
    reference_frame = 'base_link'
    arm.set_pose_reference_frame(reference_frame)

    arm.allow_replanning(True)
    arm.set_goal_position_tolerance(0.001)
    arm.set_goal_orientation_tolerance(0.001)

    arm.set_max_acceleration_scaling_factor(0.3)
    arm.set_max_velocity_scaling_factor(0.3)

    arm.set_named_target('home')
    arm.go()
    rospy.sleep(1)

    rospy.init_node('probot_self_test')

    ioPub = rospy.Publisher('probot_set_output_io', SetOutputIO, queue_size=1)

    rate = rospy.Rate(1)



    #Io test#
    count = -1

    print ("IO Relay Test.")

    while not rospy.is_shutdown():   
      ioOutput = SetOutputIO()
      ioOutput.type = SetOutputIO.TYPE_RELAY
      ioOutput.mask = 255
      ioOutput.status = 2**(count + 1) - 1
      ioPub.publish(ioOutput)
      count = count +1
      if count == 8:
          break
      print ('IO Relay',count+1)
      rate.sleep()

    count = -1

    print ("IO Digital Test.")

    while not rospy.is_shutdown():   
      ioOutput = SetOutputIO()
      ioOutput.type = SetOutputIO.TYPE_DIGITAL
      ioOutput.mask = 65535
      ioOutput.status = 2**(count + 1) - 1
      ioPub.publish(ioOutput)
      count = count +1
      if count == 16:
          break
      print ('IO Digital',count+1)
      rate.sleep()

    ioOutput = SetOutputIO()
    ioOutput.type = SetOutputIO.TYPE_RELAY
    ioOutput.mask = 255
    ioOutput.status = 0
    ioPub.publish(ioOutput)

    ioOutput = SetOutputIO()
    ioOutput.type = SetOutputIO.TYPE_DIGITAL
    ioOutput.mask = 65535
    ioOutput.status = 0
    ioPub.publish(ioOutput)
    print ("IO Test finished!!!")

    #joint test#
    print ("Joint Test finished!!!")

    print ("Joint1+")
    moveSingleJoint(0, 0.2)
    rospy.sleep(2)
    print ("Joint1-")
    moveSingleJoint(0, 0)
    rospy.sleep(2)

    print ("Joint2+")
    moveSingleJoint(1, 0.2)
    rospy.sleep(2)
    print ("Joint2-")
    moveSingleJoint(1, 0)
    rospy.sleep(2)

    print ("Joint3+")
    moveSingleJoint(2, 0.2)
    rospy.sleep(2)
    print ("Joint3-")
    moveSingleJoint(2, 0)
    rospy.sleep(2)

    print ("Joint4+")
    moveSingleJoint(3, 0.2)
    rospy.sleep(2)
    print ("Joint4-")
    moveSingleJoint(3, 0)
    rospy.sleep(2)

    print ("Joint5+")
    moveSingleJoint(4, 0.2)
    rospy.sleep(2)
    print ("Joint5-")
    moveSingleJoint(4, 0)
    rospy.sleep(2)

    print ("Joint6+")
    moveSingleJoint(5, 0.2)
    rospy.sleep(2)
    print ("Joint6-")
    moveSingleJoint(5, 0)
    rospy.sleep(2)



    rospy.signal_shutdown("Joint Test Finished.")
