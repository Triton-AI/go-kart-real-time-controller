# Go Kart Controller - ROS2 Driver

## Purpose of Package

This packet is a ROS2 driver for Triton AI's go-kart real-time controller. The current development work targets ROS2 Galactic.

The packet is capable of sending control messages and receive sensor and state report from the controller.

## API

## Inner Working

The packet assumes the following state machine exists on the MCU. See a detailed explanation in the [MCU code repository](https://github.com/Triton-AI/go-kart-real-time-controller-mbed).

![GKC State Machine](gkc_state_machine.png)

The state machine proposal is purposefully made close to [ROS2 Lifecycle](https://design.ros2.org/articles/node_lifecycle.html) to enable easier management, with the major difference being an additional "emergency stop" state. Since ROS2 Lifecycle system does not support custom state, it is implemented as part of the uninitialized state in ROS2 Lifecycle, while the error state is used to direct all emergency stop requests to the uninitialized state.

Another difference is in the shutdown procedure where shutdown transition will always eventually end in emergency stop state (by signaling transition error and be directed to uninitialized state). The rationale is that the software should always have control or communication with the vehicle as much as possible, instead of destructing such interface. Only fatal error or user interrupt can destruct the node.

## Known Issues
