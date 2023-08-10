
#include <gazebo/physics/Model.hh>
#include <gazebo/physics/Joint.hh>
#include <conveyor_belt/conveyor_belt_plugin.hpp>
#include <gazebo_ros/node.hpp>
#include <rclcpp/rclcpp.hpp>


#include <memory>

namespace gazebo{


/// Class to hold private data members (PIMPL pattern)
class ConveyorBeltPluginPrivate
{
public:
  /// Connection to world update event. Callback is called while this is alive.
  gazebo::event::ConnectionPtr update_connection_;

  /// Node for ROS communication.
  gazebo_ros::Node::SharedPtr ros_node_;

  /// The joint that controls the movement of the belt.
  gazebo::physics::JointPtr belt_joint_;

  double belt_velocity_;
  double max_velocity_;
  double power_;


  /// Position limit of belt joint to reset 
  double limit_;

  /// Service for enabling the vacuum gripper
  // rclcpp::Service<ariac_msgs::srv::ConveyorBeltControl>::SharedPtr enable_service_;

  // // Subscriber to competition state
  // rclcpp::Subscription<ariac_msgs::msg::CompetitionState>::SharedPtr competition_state_sub_;

  // /// Status Publisher
  // rclcpp::Publisher<ariac_msgs::msg::ConveyorBeltState>::SharedPtr status_pub_;
  // ariac_msgs::msg::ConveyorBeltState status_msg_;

  rclcpp::Time last_publish_time_;
  int update_ns_;

  void OnUpdate();

  /// Callback for enable service
  // void SetConveyorPower(
  //   ariac_msgs::srv::ConveyorBeltControl::Request::SharedPtr,
  //   ariac_msgs::srv::ConveyorBeltControl::Response::SharedPtr);

  /// Method to publish status
  void PublishStatus();

  // void CompetitionStateCallback(const ariac_msgs::msg::CompetitionState::SharedPtr msg_);
};

ConveyorBeltPlugin::ConveyorBeltPlugin()
: impl_(std::make_unique<ConveyorBeltPluginPrivate>())
{
  printf("Hello Conveyor Belt!\n");
}

ConveyorBeltPlugin::~ConveyorBeltPlugin()
{
}

void ConveyorBeltPlugin::Load(gazebo::physics::ModelPtr model, sdf::ElementPtr sdf)
{
  // Create ROS node
  impl_->ros_node_ = gazebo_ros::Node::Get(sdf);

  // // Create status publisher
  // impl_->status_pub_ = impl_->ros_node_->create_publisher<ariac_msgs::msg::ConveyorBeltState>("/ariac/conveyor_state", 10);
  // impl_->status_msg_.enabled = false;
  // impl_->status_msg_.power = 0;

  // // Create subscriber
  // impl_->competition_state_sub_ = impl_->ros_node_->create_subscription<ariac_msgs::msg::CompetitionState>(
  //   "/ariac/competition_state", 10, 
  //   std::bind(&ConveyorBeltPluginPrivate::CompetitionStateCallback, impl_.get(), std::placeholders::_1));

  // Create belt joint
  impl_->belt_joint_ = model->GetJoint("belt_moving_joint");

  if (!impl_->belt_joint_) {
    RCLCPP_ERROR(impl_->ros_node_->get_logger(), "Belt joint not found, unable to start conveyor plugin");
    return;
  }

  // Set velocity (m/s)
  impl_->max_velocity_ = sdf->GetElement("max_velocity")->Get<double>();

  // Set limit (m)
  impl_->limit_ = impl_->belt_joint_->UpperLimit();

  // Register enable service
  // impl_->enable_service_ = impl_->ros_node_->create_service<ariac_msgs::srv::ConveyorBeltControl>(
  //     "/ariac/set_conveyor_power", 
  //     std::bind(
  //       &ConveyorBeltPluginPrivate::SetConveyorPower, impl_.get(),
  //       std::placeholders::_1, std::placeholders::_2));

  double publish_rate = sdf->GetElement("publish_rate")->Get<double>();
  impl_->update_ns_ = int((1/publish_rate) * 1e9);

  impl_->last_publish_time_ = impl_->ros_node_->get_clock()->now();

  // Create a connection so the OnUpdate function is called at every simulation iteration. 
  impl_->update_connection_ = gazebo::event::Events::ConnectWorldUpdateBegin(
    std::bind(&ConveyorBeltPluginPrivate::OnUpdate, impl_.get()));
}

void ConveyorBeltPluginPrivate::OnUpdate()
{

  power_ = 100;
  belt_velocity_ = max_velocity_;

  belt_joint_->SetVelocity(0, belt_velocity_);

  double belt_position = belt_joint_->Position(0);

  if (belt_position >= limit_){
    belt_joint_->SetPosition(0, 0);
  }

  // Publish status at rate
  // rclcpp::Time now = ros_node_->get_clock()->now();
  // if (now - last_publish_time_ >= rclcpp::Duration(0, update_ns_)) {
  //   PublishStatus();
  //   last_publish_time_ = now;
  // }
    
}

// Register this plugin with the simulator
GZ_REGISTER_MODEL_PLUGIN(ConveyorBeltPlugin)

}