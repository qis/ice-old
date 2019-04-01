#pragma once
#include <chrono>
#include <string>
#include <vector>

namespace ice {
namespace log {

using clock = std::chrono::system_clock;
using time_point = clock::time_point;

enum class severity {
  emergency = 0,
  alert = 1,
  critical = 2,
  error = 3,
  warning = 4,
  notice = 5,
  info = 6,
  debug = 7,
};

struct message {
  log::time_point time_point;
  log::severity severity;
  std::string text;
};

class sink {
public:
  virtual ~sink() = default;
  virtual void write(const std::vector<ice::log::message>& messages) = 0;
};

class null : public sink {
public:
  void write(const std::vector<ice::log::message>& messages) final {
  }
};

}  // namespace log
}  // namespace ice
