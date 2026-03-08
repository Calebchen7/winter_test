#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "std_msgs/msg/bool.hpp"
#include <termios.h>
#include <unistd.h>
#include <iostream>

using namespace std::chrono_literals;

class RobotControl : public rclcpp::Node
{
public:
    RobotControl() : Node("robot_control"), obstacle_detected_(false), stopped_(false)
    {
        // 发布速度指令
        cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
        
        // 订阅障碍信号
        obstacle_sub_ = this->create_subscription<std_msgs::msg::Bool>(
            "/obstacle_detected", 10, std::bind(&RobotControl::obstacle_callback, this, std::placeholders::_1));
        
        //控制循环
        timer_ = this->create_wall_timer(100ms, std::bind(&RobotControl::control_loop, this));
        
        // 设置终端
        tcgetattr(STDIN_FILENO, &old_tio_);
        termios new_tio = old_tio_;
        new_tio.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
        
        RCLCPP_INFO(this->get_logger(), "控制节点启动，按A左转，D右转，Ctrl+C退出");
    }
    
    ~RobotControl()
    {
        // 恢复终端模式
        tcsetattr(STDIN_FILENO, TCSANOW, &old_tio_);
    }

private:
    bool obstacle_detected_;
    bool stopped_;
    geometry_msgs::msg::Twist last_cmd_;
    termios old_tio_;

    void obstacle_callback(const std_msgs::msg::Bool::SharedPtr msg)
    {
        if (msg->data && !stopped_)
        {
            stopped_ = true;
            RCLCPP_WARN(this->get_logger(), "检测到障碍物，已停止");
        }
        else if (!msg->data)
        {
            stopped_ = false;
        }
        obstacle_detected_ = msg->data;
    }

    char get_key()
    {
        char c = 0;
        fd_set set;
        timeval timeout = {0, 0};
        
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        
        if (select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout) > 0)
        {
            read(STDIN_FILENO, &c, 1);
        }
        return c;
    }

    void control_loop()
    {
        geometry_msgs::msg::Twist cmd;
        char key = get_key();
        
        if (stopped_)
        {
            if (key == 'a' || key == 'A')
            {
                cmd.linear.x = 0.1;
                cmd.angular.z = 0.5;
                RCLCPP_INFO(this->get_logger(), "执行左转");
            }
            else if (key == 'd' || key == 'D')
            {
                cmd.linear.x = 0.1;
                cmd.angular.z = -0.5;
                RCLCPP_INFO(this->get_logger(), "执行右转");
            }
            else
            {
                cmd.linear.x = 0.0;
                cmd.angular.z = 0.0;
            }
        }
        else
        {
            if (!obstacle_detected_)
            {
                cmd.linear.x = 0.2;
                cmd.angular.z = 0.0;
            }
        }
        
        cmd_vel_pub_->publish(cmd);
        last_cmd_ = cmd;
    }

    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr obstacle_sub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RobotControl>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
