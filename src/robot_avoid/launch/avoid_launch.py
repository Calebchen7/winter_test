from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([

        Node(
            package='robot_avoid',
            executable='obstacle_detector',
            name='obstacle_detector_node',
            output='screen'
        ),
       
        Node(
            package='robot_avoid',
            executable='robot_control',
            name='robot_control_node',
            output='screen'
        )
    ])
