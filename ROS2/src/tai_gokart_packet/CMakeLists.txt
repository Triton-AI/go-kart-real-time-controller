cmake_minimum_required(VERSION 3.8)
project(tai_gokart_packet)


if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

set(GKC_PACKET_LIB_SRC
  src/gkc_packet_factory.cpp
  src/gkc_packets.cpp
)

set(GKC_PACKET_LIB_HEADERS
  include/tai_gokart_packet/gkc_packet_factory.hpp
  include/tai_gokart_packet/gkc_packets.hpp
  include/tai_gokart_packet/gkc_packet_subscriber.hpp
)

# generate library
ament_auto_add_library(${PROJECT_NAME} SHARED
  ${GKC_PACKET_LIB_SRC}
  ${GKC_PACKET_LIB_HEADERS}
)


if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # uncomment the line when a copyright and license is not present in all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # uncomment the line when this package is not in a git repo
  #set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
  set(TEST_SOURCES test/test_gkc_packet.cpp)
  set(TEST_POLYGON_REMOVER_EXE test_gkc_packet)
  ament_add_gtest(${TEST_POLYGON_REMOVER_EXE} ${TEST_SOURCES})
  target_link_libraries(${TEST_POLYGON_REMOVER_EXE} ${PROJECT_NAME})

endif()

ament_package()
