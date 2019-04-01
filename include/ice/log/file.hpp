#pragma once
#include <ice/log/sink.hpp>
#include <filesystem>
#include <memory>

namespace ice {
namespace log {

class file : public sink {
public:
  file(const std::filesystem::path& filename,
    severity severity = severity::debug,
    bool date = true,
    bool milliseconds = true);

  virtual ~file();

  void write(const std::vector<message>& messages) override;

private:
  class impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace  log
}  // namespace  ice
