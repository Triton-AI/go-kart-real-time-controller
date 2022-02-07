/**
 * @file config.hpp
 * @author Haoru Xue (haoru.xue@autoware.org)
 * @brief
 * @version 0.1
 * @date 2022-02-06
 *
 * @copyright Copyright 2022 Triton AI
 *
 */

#ifndef TAI_GOKART_CONTROLLER__CONFIG_HPP_
#define TAI_GOKART_CONTROLLER__CONFIG_HPP_

#include <map>
#include <cstdint>
#include <string>
#include <utility>

namespace tritonai
{
namespace gkc
{
union Configurable {
  char string[32];  // Must be null-terminated
  int64_t integer;
  double floating;
  bool boolean;
};

typedef std::pair<std::string, Configurable> Config;
typedef std::map<std::string, Configurable> ConfigList;
}  // namespace gkc
}  // namespace tritonai
#endif  // TAI_GOKART_CONTROLLER__CONFIG_HPP_
