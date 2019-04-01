#include <ice/log/file.hpp>
#include <ice/log.hpp>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ice {
namespace log {

class file::impl {
public:
  impl(const std::filesystem::path& filename, severity severity, bool date, bool milliseconds) :
    severity_(severity), date_(date), milliseconds_(milliseconds) {
    os_.open(filename, std::ios::binary | std::ios::app);
    if (!os_.is_open()) {
      throw std::runtime_error("could not open log file: " + filename.u8string());
    }
  }

  void write(const std::vector<message>& messages) {
    for (const auto& message : messages) {
      if (message.severity > severity_) {
        continue;
      }
      os_ << format(message.time_point, date_, milliseconds_) << " [" << format(message.severity, true) << "] "
          << message.text;
#ifdef _WIN32
      os_ << '\r';
#endif
      os_ << '\n';
    }
    os_ << std::flush;
  }

private:
  std::ofstream os_;
  severity severity_ = severity::debug;
  bool date_ = true;
  bool milliseconds_ = true;
};

file::file(const std::filesystem::path& filename, severity severity, bool date, bool milliseconds) :
  impl_(std::make_unique<impl>(filename, severity, date, milliseconds)) {
}

file::~file() {
}

void file::write(const std::vector<message>& messages) {
  impl_->write(messages);
}

}  // namespace  log
}  // namespace  ice
