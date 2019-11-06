#pragma once
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace ice {

using namespace ::nlohmann;

namespace config {

inline json parse(std::istream& is) {
  std::stringstream ss;
  for (std::string line; std::getline(is, line);) {
    enum class state { none, escape, escape_u, escape_0, escape_1, escape_2, string, slash, end };
    auto s = state::none;
    std::size_t size = 0;
    for (auto it = line.begin(), end = line.end(); s != state::end && it != end; ++it) {
      switch (s) {
      case state::none:
        switch (*it) {
        case '"':
          s = state::string;
          break;
        case '/':
          s = state::slash;
          break;
        }
        break;
      case state::string:
        switch (*it) {
        case '"':
          s = state::none;
          break;
        case '\\':
          s = state::escape;
          break;
        }
        break;
      case state::escape:
        switch (*it) {
        case 'u':
          s = state::escape_u;
          break;
        default:
          s = state::string;
          break;
        }
        break;
      case state::escape_u:
      case state::escape_0:
      case state::escape_1:
      case state::escape_2:
        if (*it < '0' || *it > '9') {
          throw std::domain_error("invalid unicode escape sequence");
        }
        s = static_cast<state>(static_cast<int>(s) + 1);
        break;
      case state::slash:
        if (*it != '/') {
          throw std::domain_error("invalid comment syntax");
        }
        size--;
        s = state::end;
        continue;
      case state::end:
        break;
      }
      size++;
    }
    ss.write(line.data(), size);
  }
  return json::parse(std::move(ss));
}

inline json parse(const std::filesystem::path& path) {
  std::ifstream is(path, std::ios::binary);
  if (!is) {
    throw std::domain_error("could not open file: " + path.u8string());
  }
  return parse(static_cast<std::istream&>(is));
}

inline json parse(const std::string& s) {
  std::istringstream is(s);
  return parse(static_cast<std::istream&>(is));
}

}  // namespace config
}  // namespace ice
