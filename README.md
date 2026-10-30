# PX4 v1.16 最小 Offboard 测试

该节点以 20 Hz 发布位置控制消息：

- 约 2 秒后请求 PX4 进入 Offboard
- 约 5 秒后发送解锁命令
- 目标位置为 `N=0, E=0, D=-height_m`

默认 `height_m=1.0`，即目标高度比本地原点高 1 米。

## 警告

该代码会自动请求 Offboard 并解锁。首次测试必须拆除螺旋桨，并使用 RC 或 QGroundControl 随时接管。不要在人员附近运行。

## 编译

```bash
mkdir -p ~/px4_ws/src
cd ~/px4_ws/src
git clone -b release/1.16 https://github.com/PX4/px4_msgs.git
```

把当前 `px4_offboard_test` 目录复制到 `~/px4_ws/src`，然后执行：

```bash
cd ~/px4_ws
source /opt/ros/humble/setup.bash
colcon build --symlink-install --packages-up-to px4_offboard_test
source install/setup.bash
```

## 启动 Agent

串口示例：

```bash
ros2 run micro_ros_agent micro_ros_agent \
  serial --dev /dev/ttyUSB0 -b 921600 -v6
```

UDP 示例：

```bash
ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888 -v6
```

## 运行

```bash
source /opt/ros/humble/setup.bash
source ~/px4_ws/install/setup.bash
ros2 run px4_offboard_test offboard_test
```

修改目标高度：

```bash
ros2 run px4_offboard_test offboard_test \
  --ros-args -p height_m:=0.5
```

按 `Ctrl+C` 停止节点。停止后飞控会触发 Offboard 链路丢失保护。
