/**
 * @file debug.hpp
 * @author Haoru Xue (hxue@ucsd.edu)
 * @brief Debugs
 * @version 0.1
 * @date 2021-10-30
 *
 * @copyright Copyright (c) 2021 [Triton AI]
 *
 */

#ifndef TAI_GOKART_PACKET__GKC_DEBUG_HPP_
#define TAI_GOKART_PACKET__GKC_DEBUG_HPP_
#include <iostream>
#include <string>

namespace tritonai
{
namespace gkc
{

void debug_cout(std::string str)
{
  std::cout << str << std::endl;
}

}  // namespace gkc
}  // namespace tritonai
#endif  // TAI_GOKART_PACKET__GKC_DEBUG_HPP_
