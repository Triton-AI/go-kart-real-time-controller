cmake_minimum_required(VERSION 3.8)
project(tai_gokart_controller)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

set(GKC_INTERFACE_LIB_SRC
  src/comm.cpp
  src/tai_gokart_interface.cpp
  src/tai_gokart_controller_node.cpp
)

set(GKC_INTERFACE_LIB_HEADERS
  include/tai_gokart_controller/comm.hpp
  include/tai_gokart_controller/tai_gokart_interface.hpp
  include/tai_gokart_controller/tai_gokart_controller_node.hpp
  include/tai_gokart_controller/config.hpp
)

ament_auto_add_library(${PROJECT_NAME} SHARED
  ${GKC_INTERFACE_LIB_SRC}
  ${GKC_INTERFACE_LIB_HEADERS}
)

rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN tritonai::gkc::GkcNode
  EXECUTABLE tai_gokart_controller_node
)

if(BUILD_TESTING)
  set(ament_cmake_copyright_FOUND TRUE)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
  set(TEST_SOURCES test/test_gkc_interface.cpp)
  set(TEST_GKC_INTERFACE_EXE test_gkc_interface)
  ament_add_gtest(${TEST_GKC_INTERFACE_EXE} ${TEST_SOURCES})
  target_link_libraries(${TEST_GKC_INTERFACE_EXE} ${PROJECT_NAME})
endif()

ament_auto_package(
  INSTALL_TO_SHARE
    launch
    param
)
