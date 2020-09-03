#include <ice/color.hpp>
#include <ice/log.hpp>
#include <ice/log/console.hpp>
#include <iostream>

namespace ice {
namespace log {

class console::impl
{
public:
  impl(severity severity, bool date, bool milliseconds)
    : severity_(severity), date_(date), milliseconds_(milliseconds)
  {}

  void write(const std::vector<message>& messages)
  {
    bool cout = false;
    bool cerr = false;
    for (const auto& message : messages) {
      if (message.severity > severity_) {
        continue;
      }
      std::ostream& os = message.severity < severity::warning ? std::cerr : std::cout;
      if (&os == &std::cout) {
        cout = true;
      } else {
        cerr = true;
      }
      //color(os);
      os << format(message.time_point, date_, milliseconds_) << " [";
      color(os, message.severity);
      os << format(message.severity, true);
      color(os);
      os << "] ";
      if (message.severity > severity::info) {
        color(os, severity::debug);
      }
      os << message.text;
      color(os);
#ifdef _WIN32
      os << '\r';
#endif
      os << '\n';
    }
    if (cout) {
      std::cout << std::flush;
    }
    if (cerr) {
      std::cerr << std::flush;
    }
  }

private:
  std::ostream& color(std::ostream& os)
  {
    return os << ice::color::reset;
  }

  std::ostream& color(std::ostream& os, severity severity)
  {
    switch (severity) {
    case severity::emergency:
      return os << ice::color::cyan;
    case severity::alert:
      return os << ice::color::blue;
    case severity::critical:
      return os << ice::color::magenta;
    case severity::error:
      return os << ice::color::red;
    case severity::warning:
      return os << ice::color::yellow;
    case severity::notice:
      return os << ice::color::green;
    case severity::info:
      return os << ice::color::reset;
    case severity::debug:
      return os << ice::color::grey;
    }
    return os;
  }

  severity severity_ = severity::debug;
  bool date_ = true;
  bool milliseconds_ = true;
};

console::console(severity severity, bool date, bool milliseconds)
  : impl_(std::make_unique<impl>(severity, date, milliseconds))
{}

console::~console() {}

void console::write(const std::vector<message>& messages)
{
  impl_->write(messages);
}

}  // namespace  log
}  // namespace  ice
