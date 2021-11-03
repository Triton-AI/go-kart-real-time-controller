/**
 * @file debug.hpp
 * @author Haoru Xue (hxue@ucsd.edu)
 * @brief Debugs
 * @version 0.1
 * @date 2021-10-30
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef GKC_DEBUG_HPP
#define GKC_DEBUG_HPP
#include <iostream>
#include <string>

namespace tritonai {
namespace gkc {

void debug_cout(std::string str){
    std::cout << str << std::endl;
}

}  // namespace gkc
}  // namespace tritonai
#endif  // GKC_DEBUG_HPP