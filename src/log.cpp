#include <ice/exception.hpp>
#include <ice/log.hpp>
#include <ice/log/console.hpp>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <set>
#include <thread>
#include <tuple>
#include <cstdio>

#ifdef _WIN32
#  include <windows.h>
#  include <iostream>
#endif

namespace ice {
namespace log {
namespace {

class logger
{
  logger() = default;

public:
  logger(logger&& other) = delete;
  logger& operator=(logger&& other) = delete;

  ~logger()
  {
    try {
      {
        std::lock_guard<std::mutex> lock(messages_mutex_);
        stop_ = true;
      }
      cv_.notify_one();
      if (thread_.joinable()) {
        thread_.join();
      }
    }
    catch (...) {
    }
  }

  void add(std::shared_ptr<ice::log::sink> sink)
  {
    std::lock_guard<std::mutex> lock(sinks_mutex_);
    sinks_.insert(std::move(sink));
  }

  void remove(std::shared_ptr<ice::log::sink> sink)
  {
    std::lock_guard<std::mutex> lock(sinks_mutex_);
    sinks_.erase(sink);
  }

  void queue(ice::log::time_point time_point, ice::log::severity severity, std::string message)
  {
    std::lock_guard<std::mutex> lock(messages_mutex_);
    messages_.push_back({ time_point, severity, std::move(message) });
    if (!stop_ && !thread_.joinable()) {
      std::lock_guard<std::mutex> lock(sinks_mutex_);
      if (sinks_.empty()) {
        sinks_.emplace(std::make_shared<console>());
      }
      thread_ = std::thread([this]() {
        run();
      });
    }
    cv_.notify_one();
  }

  static logger& get()
  {
    static logger logger;
    return logger;
  }

private:
  void write(const std::vector<message>& messages)
  {
    if (messages.empty()) {
      return;
    }
    std::vector<std::weak_ptr<sink>> sinks;
    {
      std::lock_guard<std::mutex> lock(sinks_mutex_);
      for (auto& sink : sinks_) {
        sinks.emplace_back(sink);
      }
    }
    for (auto& wp : sinks) {
      if (auto sink = wp.lock()) {
        sink->write(messages);
      }
    }
  }

  void run()
  {
    while (!stop_) {
      std::vector<message> messages;
      {
        std::unique_lock<std::mutex> lock(messages_mutex_);
        while (!stop_ && messages_.empty()) {
          cv_.wait(lock);
        }
        messages = std::move(messages_);
        messages_.clear();
      }
      write(messages);
    }
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    std::lock_guard<std::mutex> lock(messages_mutex_);
    write(messages_);
    messages_.clear();
  }

  std::vector<message> messages_;
  std::mutex messages_mutex_;

  std::set<std::shared_ptr<sink>> sinks_;
  std::mutex sinks_mutex_;

  std::condition_variable cv_;
  std::thread thread_;

  std::atomic<bool> stop_ = { false };
};

}  // namespace

void add(std::shared_ptr<ice::log::sink> sink)
{
  logger::get().add(std::move(sink));
}

void remove(std::shared_ptr<ice::log::sink> sink)
{
  logger::get().remove(std::move(sink));
}

std::string format(time_point tp, bool date, bool milliseconds)
{
  auto time = clock::to_time_t(tp);
  tm tm = {};
#ifndef _WIN32
  localtime_r(&time, &tm);
#else
  localtime_s(&tm, &time);
#endif

  auto Y = tm.tm_year + 1900;
  auto M = tm.tm_mon + 1;
  auto D = tm.tm_mday;
  auto h = tm.tm_hour;
  auto m = tm.tm_min;
  auto s = tm.tm_sec;
  auto ms = 0;

  if (milliseconds) {
    const auto t = tp.time_since_epoch();
    const auto s = std::chrono::duration_cast<std::chrono::seconds>(t);
    const auto m = std::chrono::duration_cast<std::chrono::milliseconds>(t) - s;
    ms = static_cast<int>(m.count());
  }

  int size = 0;
  std::string str;
  if (date && milliseconds) {
    str.resize(23 + 1);
    size = std::snprintf(
      &str[0], str.size(), "%04d-%02d-%02d %02d:%02d:%02d.%03d", Y, M, D, h, m, s, ms);
  } else if (date && !milliseconds) {
    str.resize(19 + 1);
    size = std::snprintf(&str[0], str.size(), "%04d-%02d-%02d %02d:%02d:%02d", Y, M, D, h, m, s);
  } else if (!date && milliseconds) {
    str.resize(12 + 1);
    size = std::snprintf(&str[0], str.size(), "%02d:%02d:%02d.%03d", h, m, s, ms);
  } else {
    str.resize(8 + 1);
    size = std::snprintf(&str[0], str.size(), "%02d:%02d:%02d", h, m, s);
  }
  str.resize(size > 0 ? size : 0);
  return str;
}

std::string format(severity s, bool padding)
{
  switch (s) {
  case severity::emergency:
    return padding ? "emergency" : "emergency";
  case severity::alert:
    return padding ? "alert    " : "alert";
  case severity::critical:
    return padding ? "critical " : "critical";
  case severity::error:
    return padding ? "error    " : "error";
  case severity::warning:
    return padding ? "warning  " : "warning";
  case severity::notice:
    return padding ? "notice   " : "notice";
  case severity::info:
    return padding ? "info     " : "info";
  case severity::debug:
    return padding ? "debug    " : "debug";
  }
  return padding ? "unknown  " : "unknown";
}

stream::stream(severity severity) : std::stringbuf(), std::ostream(this), severity_(severity) {}

stream::stream(stream&& other)
  : std::stringbuf(std::move(other)), std::ostream(this), severity_(other.severity_),
    time_point_(other.time_point_)
{}

stream& stream::operator=(stream&& other)
{
  static_cast<std::stringbuf&>(*this) = std::move(other);
  severity_ = other.severity_;
  time_point_ = other.time_point_;
  return *this;
}

stream::~stream()
{
  try {
    auto s = str();
    auto pos = s.find_last_not_of(" \t\n\v\f\r");
    if (pos != std::string::npos) {
      s.erase(pos + 1);
      s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
      logger::get().queue(time_point_, severity_, std::move(s));
    }
  }
  catch (...) {
  }
}

stream& stream::operator<<(const std::error_code& ec)
{
  auto& os = *this;
  os << ec.category().name() << ' ' << ec.value() << ": " << ec.message();
  return os;
}

stream& stream::operator<<(const std::exception_ptr& e)
{
  auto& os = *this;
  try {
    if (e) {
      std::rethrow_exception(e);
    }
  }
  catch (const ice::exception& e) {
    if (auto se = dynamic_cast<const std::system_error*>(&e)) {
      os << se->code().category().name() << ' ' << se->code().value() << ": ";
    }
    os << e.what();
    if (auto info = e.info()) {
      os << ": " << info;
    }
  }
  catch (const std::exception& e) {
    if (auto se = dynamic_cast<const std::system_error*>(&e)) {
      os << se->code().category().name() << ' ' << se->code().value() << ": ";
    }
    os << e.what();
  }
  catch (...) {
    os << "unhandled exception";
  }
  return os;
}

}  // namespace log
}  // namespace ice
