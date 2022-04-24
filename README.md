# Go-Kart Real-Time Controller

[![ROS2 CI Workflow](https://github.com/Triton-AI/go-kart-real-time-controller/actions/workflows/ros2-ci.yaml/badge.svg)](https://github.com/Triton-AI/go-kart-real-time-controller/actions/workflows/ros2-ci.yaml)

## Navigation

`src` folder contains the ROS2 driver, ROS2 messages, and a C++ communication library.

Hardware design (PCB) has been moved to a separate [repository](https://github.com/Triton-AI/go-kart-real-time-controller-mbed)).

RTC code is located in its own [repository](https://github.com/Triton-AI/go-kart-real-time-controller-mbed).

## Install

1. Install ROS2 Galactic
2. Clone this repository

## Build

```{bash}
cd go-kart-real-time-controller
source /opt/ros/galactic/setup.bash
colcon build
```

## Documentations

- [Getting started with packet communication library](ROS2/src/tai_gokart_packet/design/README.md)
- [Packet API documentation](ROS2/src/tai_gokart_packet/design/Packet_API.md)
