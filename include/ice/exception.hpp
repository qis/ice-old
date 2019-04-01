#pragma once
#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>

namespace ice {

class exception {
public:
  virtual const char* what() const noexcept = 0;
  virtual const char* info() const noexcept = 0;
};

template <typename T>
class exception_stream : public exception, public T {
public:
  using endl = std::ostream& (*)(std::ostream&);

  using T::T;

  exception_stream(exception_stream&& other) = default;
  exception_stream(const exception_stream& other) = default;

  exception_stream& operator=(exception_stream&& other) = default;
  exception_stream& operator=(const exception_stream& other) = default;

  template <typename V>
  exception_stream& operator<<(const V& v) {
    std::ostringstream oss;
    oss.flags(flags_);
    oss.precision(precision_);
    oss.width(width_);
    oss << v;
    width_ = oss.width();
    precision_ = oss.precision();
    flags_ = oss.flags();
    info_.append(oss.str());
    return *this;
  }

  exception_stream& operator<<(const std::error_code& ec) {
    info_.append(std::string(ec.category().name()) + ' ' + std::to_string(ec.value()) + ": " + ec.message());
    return *this;
  }

  exception_stream& operator<<(endl) {
    info_.push_back('\n');
    return *this;
  }

  const char* what() const noexcept override {
    return T::what();
  }

  const char* info() const noexcept override {
    if (info_.empty()) {
      return nullptr;
    }
    return info_.c_str();
  }

private:
  static std::ios_base::fmtflags default_flags() {
    static const std::ios_base::fmtflags flags = std::ostringstream().flags();
    return flags;
  }

  std::string info_;
  std::ios_base::fmtflags flags_ = default_flags();
  std::streamsize precision_ = 0;
  std::streamsize width_ = 0;
};

using runtime_error = exception_stream<std::runtime_error>;
using system_error = exception_stream<std::system_error>;

}  // namespace ice
