#pragma once
#include <ice/exception.hpp>
#include <ice/log/sink.hpp>
#include <memory>
#include <sstream>

namespace ice {
namespace log {

void add(std::shared_ptr<ice::log::sink> sink);
void remove(std::shared_ptr<ice::log::sink> sink);

std::string format(time_point tp, bool date = true, bool milliseconds = true);
std::string format(severity s, bool padding = true);

class stream : public std::stringbuf, public std::ostream {
public:
  explicit stream(severity severity);

  stream(stream&& other);
  stream& operator=(stream&& other);

  virtual ~stream();

  template <typename T>
  stream& operator<<(const T& v) {
    static_cast<std::ostream&>(*this) << v;
    return *this;
  }

  stream& operator<<(const std::error_code& ec);
  stream& operator<<(const std::exception_ptr& e);

private:
  severity severity_ = severity::info;
  time_point time_point_ = clock::now();
};

class emergency : public stream {
public:
  emergency() : stream(severity::emergency) {}

  template <typename T>
  emergency& operator<<(const T& v) {
    static_cast<stream&>(*this) << v;
    return *this;
  }
};

class critical : public stream {
public:
  critical() : stream(severity::critical) {}

  template <typename T>
  critical& operator<<(const T& v) {
    static_cast<stream&>(*this) << v;
    return *this;
  }
};

class error : public stream {
public:
  error() : stream(severity::error) {}

  template <typename T>
  error& operator<<(const T& v) {
    static_cast<stream&>(*this) << v;
    return *this;
  }
};

class warning : public stream {
public:
  warning() : stream(severity::warning) {}

  template <typename T>
  warning& operator<<(const T& v) {
    static_cast<stream&>(*this) << v;
    return *this;
  }
};

class notice : public stream {
public:
  notice() : stream(severity::notice) {}

  template <typename T>
  notice& operator<<(const T& v) {
    static_cast<stream&>(*this) << v;
    return *this;
  }
};

class info : public stream {
public:
  info() : stream(severity::info) {}

  template <typename T>
  info& operator<<(const T& v) {
    static_cast<stream&>(*this) << v;
    return *this;
  }
};

class debug : public stream {
public:
  debug() : stream(severity::debug) {}

  template <typename T>
  debug& operator<<(const T& v) {
    static_cast<stream&>(*this) << v;
    return *this;
  }
};

}  // namespace log
}  // namespace ice
