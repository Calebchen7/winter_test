#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "std_msgs/msg/bool.hpp"

using std::placeholders::_1;

class ObstacleDetector : public rclcpp::Node
{
public:
    ObstacleDetector() : Node("obstacle_detector")
    {
        // 订阅激光雷达数据
        scan_sub_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
            "/scan", 10, std::bind(&ObstacleDetector::scan_callback, this, _1));
        
        // 发布障碍信号
        obstacle_pub_ = this->create_publisher<std_msgs::msg::Bool>("/obstacle_detected", 10);
        
        RCLCPP_INFO(this->get_logger(), "障碍检测节点启动，监听/scan话题");
    }

private:
    void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
        std_msgs::msg::Bool obstacle_msg;
        bool has_obstacle = false;
        // 安全距离0.5米
        float safe_distance = 0.5f;
        
        
        // 检测前方30度范围内的障碍
        int center_idx = msg->ranges.size() / 2;
        // 左右各15个角度
        int range = 15; 
        
        for (int i = center_idx - range; i <= center_idx + range; ++i)
        {
            if (msg->ranges[i] < safe_distance && msg->ranges[i] > 0.0f)
            {
                has_obstacle = true;
                break;
            }
        }
        
        obstacle_msg.data = has_obstacle;
        obstacle_pub_->publish(obstacle_msg);
    }

    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr obstacle_pub_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ObstacleDetector>());
    rclcpp::shutdown();
    return 0;
}
