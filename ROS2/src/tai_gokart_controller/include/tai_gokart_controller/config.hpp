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

  explicit Configurable(const std::string & string_type)
  {
    if (string_type.size() + 1 > 32) {
      throw std::runtime_error(
              "A string casted to Configurable cannot exceed 32 chars, including null terminator.");
    }
    std::snprintf(&string[0], sizeof(string), "%s", string_type.c_str());
  }

  operator std::string() const
  {
    return std::string(&string[0]);
  }
};

typedef std::pair<std::string, Configurable> Config;
typedef std::map<std::string, Configurable> ConfigList;
}  // namespace gkc
}  // namespace tritonai
#endif  // TAI_GOKART_CONTROLLER__CONFIG_HPP_
