builtin_interfaces/Time stamp

float32 wheel_speed_fl  # wheel speeds in rpm
float32 wheel_speed_fr
float32 wheel_speed_rl
float32 wheel_speed_rr

float32 voltage   # battery voltage in volt
float32 amperage  # battery current draw in amp

float32 brake_pressure  # brake pressure in psi
float32 throttle_pos  # throttle paddle position out of 1.0
float32 steering_angle_rad  # (left +, right -) average wheel angle of the front wheels in rad
float32 servo_angle_rad  # (left +, right -) servo offset from center in rad

bool fault_brake  # fault flag in actuation subsystem
bool fault_throttle
bool fault_steering

bool fault_critical  # fault flag with severity level
bool fault_error
bool fault_warning
bool fault_info

uint8 state
