//#include "/home/tomas/probot_g602_ws/src/probot_g602/probot_g602_demo/src/moveit_fk_demo.cpp"
#include "moveit_fk_demo.cpp"
#include <pthread.h>

int axi_address;
std::string config_file;
std::vector<std::string> joint_list;
MetaInfoObject mi;
pthread_t search_home_thread;
pthread_t move_to_point_thread;
pthread_t apply_trajectory_thread;

//const char* command_home = "./home";
//const char* command_node = "./node";

/**
 * A global variable maintaining the current point to move the arm
 * This point is initially empty and its value is updated when the user
 * requests via ARM_MOVE_TO_POINT signal. After the robotc arm is moved
 * its value is reset to an empty point again.
**/
Point current_point;

/**
 * A global variable maintaining the current trajectory to be executed
 * This trajectory is initially empty and its value is updated when the user
 * requests via ARM_APPLY_TRAJECTORY signal. After the trajectory is applied 
 * its value is reset to and empty trajectory again.
 **/
Trajectory current_trajectory;

/**
 * Global flag informing that a trajectory is being executed or not 
**/
bool executing_trajectory = false;
bool cancel_trajectory = false;

/*
void parse_arguments(int argc, char *argv[]){
  // Argument parsing
  argparse::ArgumentParser parser("mqtt_rbtanno");
  //parser.add_argument("-m", "--mqtt_id").help("MQTT client identifier.").required().scan<'i', int>();
  parser.add_argument("-x", "--axi_addr").help("AXI memory address where the RobotAnno hardware is mapped.").required().default_value(0x40000000).scan<'i', int>();
  parser.add_argument("-c", "--config_file").help("Optional. Configuration and trajectory file in JSON format. This file can be used to configure each joint's controller parameters. Default is 'initial_config.json'").default_value(std::string("initial_config.json"));
  parser.add_argument("-f", "--trajectory_file").help("File which contains the trajectory in JSON format").default_value(std::string("trajectories.json"));
  // parser.add_argument("-n", "--n_traj").help("Optional. Index for a file with multiple trajectories. Check the format at URL.").default_value(int(0)).scan<'i', int>();
  parser.add_argument("-i", "--ip").help("Optional. IP of the MQTT broker to connect to. Default is ...").default_value(std::string("192.168.0.100"));
  parser.add_argument("-V", "--verbose").help("Optional. Increase verbosity of output.").default_value(false).implicit_value(true);
  //parser.add_argument("-j", "--joints").help("Optional. List of joints to be used in the robot. Example: J1,J2,J3").default_value(std::string("J1,J2,J3,J4,J5,J6"));

  //std::cout << "Parsers added" << std::endl;

  try
  {
  parser.parse_args(argc, argv);
  }
  catch (const std::runtime_error &err)
  {
  std::cout << "RUNTIME ERROR!!!!" << std::endl;
  std::exit(-1);
  }

  std::cout << "Parsing done" << std::endl;

  // Get all argument values
  axi_address = parser.get<int>("--axi_addr");
  //int mqtt_id = parser.get<int>("--mqtt_id");
  config_file = parser.get<std::string>("--config_file").c_str();
  // int trajectory_index = parser.get<int>("--n_traj");
  const char *ip = parser.get<std::string>("--ip").c_str();
  bool verbose = parser.get<bool>("--verbose");
  
  const char *trajectory_file = parser.get<std::string>("--trajectory_file").c_str();

  std::cout << "End parsing" << std::endl;
}
*/

void generate_metainfo(void)
{
  mi = initial_metainfo();
  mi.signal = ARM_METAINFO;
  mi.name = "RbtAnno";

  joint_list.resize(mi.mi_joints.size());

	for (int Jidx = 0; Jidx < mi.mi_joints.size(); Jidx++){
		joint_list[Jidx]=mi.mi_joints[Jidx].name;
	} 

}

int extract_signal(std::string message)
{
    int result = 0;
    json json_obj = json::parse(message);

    result = json_obj["signal"];

    return result;
}





void* search_home_threaded_function(void* arg){
	//the answer
	CommandObject output = CommandObject();
  output.signal = ARM_HOME_SEARCHED;
	output.client = owner;
	output.error = error_state;

  std::vector<double> real_point = Home();
  output.point.coordinates = real_point;


	//publish message notifying that home has been reached
	publish_message(ROBOT_NAME_COMMANDS_TOPIC,output.to_json().dump().c_str());
	std::cout << "Home position reached" << std::endl;
	return NULL;
}

/**
 * Function to move the arm to a single point. It must be executed into a
 * thread to avoid blocking the main process and disconnect from the broker.
 * Before calling this function all pre-conditions have already been validated
 */
void* move_to_point_threaded_function(void* arg){

  //Lock-variables ON
  executing_trajectory = true;
	
  //the answer
	MovedObject output = MovedObject();
	output.client = owner;
	output.error = error_state;

	Point realPoint = current_point;
  //realPoint.coordinates[4]=-1*realPoint.coordinates[4];

  std::vector<double> doubleVector = realPoint.coordinates;
  std::vector<double> real_point = MoveToPoint(doubleVector);
  output.content.coordinates = real_point;


	//publish message notifying that the point has been published
	publish_message(ROBOT_NAME_MOVED_TOPIC,output.to_json().dump().c_str());
	std::cout << "Arm moved to point " << output.content.to_json().dump().c_str() << std::endl;
  
  //Lock-variables OFF
  executing_trajectory = false;
  cancel_trajectory = false;

	//after publishing the message, the current_point must be re-instantiated with empty point
	current_point = Point();

	return NULL;
}

/**
 * Function to aply a trajectory. It must be executed into a
 * thread to avoid blocking the main process and disconnect from the broker.
 * Before calling this function all pre-conditions have already been validated
 */
void* apply_trajectory_threaded_function(void* arg){
	//points to be considered come from the global variable "current_trajectory"
	std::vector<Point> points = std::vector<Point>(current_trajectory.points);

  MovedObject output = MovedObject();
	output.client = owner;
	output.error = error_state;

	//trajectory execution has been started
	executing_trajectory = true;


  while (!points.empty() && !cancel_trajectory){
    Point p = points.front();
    std::vector<double> doubleVector = p.coordinates;
    std::vector<double> real_point = MoveToPoint(doubleVector);
    output.content.coordinates = real_point;

    //usleep(2000);

    int err = publish_message(ROBOT_NAME_MOVED_TOPIC,output.to_json().dump().c_str());
    std::cout << "Arm moved to point " << output.content.to_json().dump().c_str() << err << std::endl;

		points.erase(points.begin());
	}

//TODO: Utilizar funcion ExecuteTrajectory y el nodo ROS de trayectorias

  //executeTrajectory(points);
  //trajectory execution has finished
	executing_trajectory = false;
  cancel_trajectory = false;

	//clean the current trajectory variable
	current_trajectory = Trajectory();

	return NULL;
}

class CommunicationLayerImpl: public CommunicationLayer
{
public:
  void handle_metainfo_topic(const struct mosquitto_message *message)
  {
    MetaInfoObject::from_json_string((char *)message->payload);
    //I dunno if this is necessary 
    int sig = extract_signal((char *)message->payload);
    MetaInfoObject output = MetaInfoObject();
    MetaInfoObject receivedMetainfo = MetaInfoObject::from_json_string((char *)message->payload);
    switch (sig)
    {
      case ARM_GET_METAINFO: //user requested arm status
        output=mi;
		    std::cout << "Request metainfo received. "
				  << " Sending metainfo: " 
				  << output.to_json().dump().c_str() << std::endl;
		    publish_message(METAINFO_TOPIC, output.to_json().dump().c_str());
		  break;
    }
  }
  void handle_robot_name_commands_topic(const struct mosquitto_message *message)
  {
    std::cout << "Command received." << std::endl;

    CommandObject::from_json_string((char *)message->payload);
    
    //obtains the signal/code
    int sig = extract_signal((char *)message->payload);

    //the answer
    CommandObject output = CommandObject();

    //parses the received command from the payload
    CommandObject receivedCommand = CommandObject::from_json_string((char *)message->payload);

    //Rbtanno robot = Rbtanno(axi_address, config_file);
    //robot.config_joints(joint_list);

    switch (sig)
    {
      case ARM_CHECK_STATUS: //user requested arm status
        output.signal=ARM_STATUS;
		    std::cout << "Request status received. "
				  << " Sending payload " 
				  << output.to_json().dump().c_str() << std::endl;
		    publish_message(ROBOT_NAME_COMMANDS_TOPIC, output.to_json().dump().c_str());
		  break;

      case ARM_CONNECT: //user wants to connect to the arm to become the owner
        std::cout << "Request to connect received: "
        << receivedCommand.to_json().dump().c_str()
        << std::endl;

        if (!owner.is_valid())
        {
          owner = receivedCommand.client;
          output.signal = ARM_CONNECTED;
          output.client = owner;

          std::cout 	<< "Arm's owner is " << owner.id << std::endl << "client is " << receivedCommand.client.id << std::endl;

          publish_message(ROBOT_NAME_COMMANDS_TOPIC, output.to_json().dump().c_str());

          int err = pthread_create(&search_home_thread, NULL, &search_home_threaded_function, NULL);
          pthread_detach(search_home_thread);

          // handle err?
        }
        else
        {
          std::cout << "Connection refused. Arm is busy." << std::endl;
        }

      break;

      case ARM_MOVE_TO_POINT: //user requested to move the arm to a single point
        std::cout << "Move to point request received. " << std::endl;

        if (owner.is_valid())
        {
          std::cout << "Owner is valid" << std::endl;

          Client client = receivedCommand.client;
          if (owner.id == client.id) // only owner can do that
          {
            Point Point_target = receivedCommand.point;
            if (!receivedCommand.point.is_empty() && !executing_trajectory && !cancel_trajectory){
              current_point = receivedCommand.point;
              int err = pthread_create(&move_to_point_thread, NULL, &move_to_point_threaded_function, NULL);
              pthread_detach(move_to_point_thread);

              // handle err?
            }

          }
          else
          {
            std::cout << "Owner is not client" << std::endl;
            // other client is trying to move the arm ==> ignore
          }
        }
        else
        {
          std::cout << "Owner is not valid" << std::endl;
          // arm has no owner ==> ignore
        }
        
      break;

      case ARM_APPLY_TRAJECTORY:  //user requested to apply a trajectory
        std::cout << "Apply trajectory received. " << std::endl;

        
        if (owner.is_valid())
        {
          Client client = receivedCommand.client;
          if (owner.id == client.id) //only owner can do that
          {
            if (!executing_trajectory){
              current_trajectory = receivedCommand.trajectory;
				      int err = pthread_create(&apply_trajectory_thread, NULL, &apply_trajectory_threaded_function, NULL);
				      pthread_detach(apply_trajectory_thread); 
            }

          }
          else
          {
            std::cout << "Owner is not client" << std::endl;
            // other client is trying to move the arm ==> ignore
          }
        }
        else
        {
          std::cout << "Owner is not valid" << std::endl;
          // arm has no owner ==> ignore
        }
        
      break;

      //TODO
      case ARM_CANCEL_TRAJECTORY: //user requested to cancel trajectory execution
        std::cout << "Cancel trajectory received. " << std::endl;
        if (owner.is_valid())
        {
          Client client = receivedCommand.client;
          if (owner.id == client.id) // only owner can do that
          {
            cancel_trajectory = true;
            output.signal = ARM_CANCELED_TRAJECTORY;
            output.client = owner;
            output.error = error_state;

            publish_message(ROBOT_NAME_COMMANDS_TOPIC, output.to_json().dump().c_str());
            std::cout << "Trajectory cancelled. " << std::endl;
          }
        }
        else
        {
          // arm has no owner ==> ignore
        }
      break;

      case ARM_DISCONNECT: //user requested to disconnect from the arm
        Client client = receivedCommand.client;
        std::cout << "Request disconnect received: " << receivedCommand.to_json().dump().c_str() << std::endl;
        if (owner.id == client.id) // only owner can do that
        {
          owner = Client();
          output.signal = ARM_DISCONNECTED;
          output.client = client;

          publish_message(ROBOT_NAME_COMMANDS_TOPIC, output.to_json().dump().c_str());
          std::cout 	<< "Client disconnected " << output.to_json().dump().c_str() << std::endl;
        }

        int err = pthread_create(&search_home_thread, NULL, &search_home_threaded_function, NULL);
        pthread_detach(search_home_thread);

      break;
    }
    
    //std::cout << "handle_robot_name_commands_topic" << std::endl;
  }
  
  void handle_robot_name_moved_topic(const struct mosquitto_message *message)
  {
    MovedObject::from_json_string((char *)message->payload);
    //TODO implement your business code
    std::cout << "handle RbtAnno/moved topic" << std::endl;
  }
  
};

CommunicationLayerImpl impl;

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
  if (std::strcmp(message->topic,METAINFO_TOPIC.c_str()) == 0)
  {
    impl.handle_metainfo_topic(message);
  }
  else if (std::strcmp(message->topic,ROBOT_NAME_COMMANDS_TOPIC.c_str()) == 0)
  {
    impl.handle_robot_name_commands_topic(message);
  }
  else if (std::strcmp(message->topic,ROBOT_NAME_MOVED_TOPIC.c_str()) == 0)
  {
    impl.handle_robot_name_moved_topic(message);
  }
  
}

  //moverse a un punto:
/*
{
	"signal":7,
	"client":{"id":"root"},
	"point":{"coordinates":[150,0,0,0,0,0,500]}
}
*/

//ejecutar trayectoria:
/*
{
	"signal":8,
	"client":{"id":"root"},
	"trajectory":
	{
		"points":
		[
			{"coordinates":[150,0,0,0,0,0,500]},
			{"coordinates":[130,0,0,0,0,0,500]}
		]
	}
}
*/

/*

void robot_test(void){

  mi = initial_metainfo();
  //std::vector<std::string> joint_list;
  joint_list.resize(mi.mi_joints.size());

	for (int Jidx = 0; Jidx < mi.mi_joints.size(); Jidx++){
		joint_list[Jidx]=mi.mi_joints[Jidx].name;
	} 


  std::cout << "Creando instancia robot..." << std::endl;
  Rbtanno robot = Rbtanno(axi_address, config_file);
  std::cout << "Configuring Joints..." << std::endl;
  robot.config_joints(joint_list);
  std::cout << "Joints configured!" << std::endl;
  
  //Primero movemos a un punto
  MovedObject output = MovedObject();
  Point Punto_Objetivo;
  Punto_Objetivo.coordinates = {90,-60,30,-15,-5,0,2000};
  std::cout << "Punto_Objetivo_COMMAND:" << Punto_Objetivo.to_json().dump().c_str() << std::endl;
  output.content = Punto_Objetivo;
  publish_message(ROBOT_NAME_MOVED_TOPIC,output.to_json().dump().c_str());
  std::cout << "Punto_Objetivo_MOVED:" << Punto_Objetivo.to_json().dump().c_str() << std::endl;

  usleep(2000000);

  Punto_Objetivo.coordinates = {45,-45,45,-45,-45,0,2000};
  std::cout << "Punto_Objetivo_COMMAND:" << Punto_Objetivo.to_json().dump().c_str() << std::endl;
  output.content = Punto_Objetivo;
  publish_message(ROBOT_NAME_MOVED_TOPIC,output.to_json().dump().c_str());
  std::cout << "Punto_Objetivo_MOVED:" << Punto_Objetivo.to_json().dump().c_str() << std::endl;

  //Por ahora, el punto que se genera para enviarse por moved es el movimiento relativo en grados efectuado por el robot.

  
  //Luego movemos una trayectoria
  Trajectory Trayectoria_objetivo;
  Point Punto_uno;
  Point Punto_dos;
  Point Punto_tres;
  Punto_uno.coordinates ={-20,10,0,-20,0,0,2000};
  Punto_dos.coordinates ={-20,-40,0,-20,0,0,2000};
  Punto_tres.coordinates ={-20,10,0,-20,0,0,2000};
  Trayectoria_objetivo.points = {Punto_uno,Punto_dos,Punto_tres};

  std::cout << "Trayectoria_objetivo:" << Trayectoria_objetivo.to_json().dump().c_str() << std::endl;
  robot.Trajectory_Class(Trayectoria_objetivo);
  
  //Por ultimo nos vamos a home
  std::cout << "Searching home..." << std::endl;
  //robot.home();
  std::cout << "Home searched" << std::endl;
  
}

*/

/*
int MoveToPoint_ROS(int argc, char **argv)
{
    ros::init(argc, argv, "moveit_fk_demo");
    ros::AsyncSpinner spinner(1);
    spinner.start();

    moveit::planning_interface::MoveGroupInterface arm("manipulator");

    arm.setGoalJointTolerance(0.001);
    arm.setMaxAccelerationScalingFactor(0.1);
    arm.setMaxVelocityScalingFactor(0.1);


    double targetPose[6] = {0.391410, -0.676384, -0.376217, 0.0, 1.052834, 0.454125};
    std::vector<double> joint_group_positions(6);
    joint_group_positions[0] = targetPose[0];
    joint_group_positions[1] = targetPose[1];
    joint_group_positions[2] = targetPose[2];
    joint_group_positions[3] = targetPose[3];
    joint_group_positions[4] = targetPose[4];
    joint_group_positions[5] = targetPose[5];

    arm.setJointValueTarget(joint_group_positions);
    arm.move();
    sleep(1);

    ros::shutdown(); 

    return 0;
}
*/



