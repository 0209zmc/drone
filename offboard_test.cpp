#include <chrono>
#include <cstdint>
#include <limits>

#include "rclcpp/rclcpp.hpp"

#include "px4_msgs/msg/offboard_control_mode.hpp"
#include "px4_msgs/msg/trajectory_setpoint.hpp"
#include "px4_msgs/msg/vehicle_command.hpp"

using namespace std::chrono_literals;
using OffboardControlMode = px4_msgs::msg::OffboardControlMode;
using TrajectorySetpoint = px4_msgs::msg::TrajectorySetpoint;
using VehicleCommand = px4_msgs::msg::VehicleCommand;

class OffboardTest : public rclcpp::Node
{
public:
  OffboardTest()
  : Node("px4_offboard_test")
  {
    height_m_ = declare_parameter<double>("height_m", 1.0);

    offboard_pub_ = create_publisher<OffboardControlMode>(
      "/fmu/in/offboard_control_mode", rclcpp::SensorDataQoS());
    setpoint_pub_ = create_publisher<TrajectorySetpoint>(
      "/fmu/in/trajectory_setpoint", rclcpp::SensorDataQoS());
    command_pub_ = create_publisher<VehicleCommand>(
      "/fmu/in/vehicle_command", rclcpp::SensorDataQoS());

    timer_ = create_wall_timer(50ms, [this]() { timer_callback(); });

    RCLCPP_WARN(
      get_logger(),
      "Offboard test started: target height %.2f m. Remove propellers!", height_m_);
  }

private:
  void timer_callback()
  {
    const uint64_t timestamp =
      static_cast<uint64_t>(get_clock()->now().nanoseconds() / 1000);

    OffboardControlMode mode{};
    mode.timestamp = timestamp;
    mode.position = true;
    offboard_pub_->publish(mode);

    TrajectorySetpoint setpoint{};
    setpoint.timestamp = timestamp;
    setpoint.position[0] = 0.0F;
    setpoint.position[1] = 0.0F;
    setpoint.position[2] = -static_cast<float>(height_m_);  // NED: up is negative Z
    setpoint.velocity.fill(std::numeric_limits<float>::quiet_NaN());
    setpoint.acceleration.fill(std::numeric_limits<float>::quiet_NaN());
    setpoint.jerk.fill(std::numeric_limits<float>::quiet_NaN());
    setpoint.yaw = 0.0F;
    setpoint.yawspeed = std::numeric_limits<float>::quiet_NaN();
    setpoint_pub_->publish(setpoint);

    // 40 x 50 ms = 2 s. Stream setpoints before requesting Offboard.
    if (counter_ >= 40 && counter_ < 45) {
      send_command(VehicleCommand::VEHICLE_CMD_DO_SET_MODE, 1.0F, 6.0F);
    }

    // 100 x 50 ms = 5 s. Request arming after Offboard has had time to start.
    if (counter_ >= 100 && counter_ < 105) {
      send_command(
        VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM,
        static_cast<float>(VehicleCommand::ARMING_ACTION_ARM), 0.0F);
      RCLCPP_WARN(get_logger(), "Arm command sent");
    }

    if (counter_ < 106) {
      ++counter_;
    }
  }

  void send_command(uint32_t command, float param1, float param2)
  {
    VehicleCommand msg{};
    msg.timestamp = static_cast<uint64_t>(get_clock()->now().nanoseconds() / 1000);
    msg.command = command;
    msg.param1 = param1;
    msg.param2 = param2;
    msg.target_system = 1;
    msg.target_component = 1;
    msg.source_system = 1;
    msg.source_component = 1;
    msg.from_external = true;
    command_pub_->publish(msg);
  }

  rclcpp::Publisher<OffboardControlMode>::SharedPtr offboard_pub_;
  rclcpp::Publisher<TrajectorySetpoint>::SharedPtr setpoint_pub_;
  rclcpp::Publisher<VehicleCommand>::SharedPtr command_pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  double height_m_{1.0};
  int counter_{0};
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<OffboardTest>());
  rclcpp::shutdown();
  return 0;
}
