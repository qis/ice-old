#pragma once
#include <ice/log/sink.hpp>
#include <memory>

namespace ice {
namespace log {

class console : public sink {
public:
  console(severity severity = severity::debug, bool date = true, bool milliseconds = true);
  virtual ~console();

  void write(const std::vector<message>& messages) override;

private:
  class impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace  log
}  // namespace  ice
