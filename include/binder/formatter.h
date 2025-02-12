#pragma once

#include <sstream>
#include <string>

namespace bustub {

class SimpleFormatter {
 public:
  template <typename... Args>
  static std::string Format(const std::string &format, Args... args) {
    std::ostringstream oss;
    FormatImpl(oss, format, args...);
    return oss.str();
  }

 private:
  template <typename T, typename... Args>
  static void FormatImpl(std::ostringstream &oss, const std::string &format, T value, Args... args) {
    size_t pos = format.find("{}");
    if (pos != std::string::npos) {
      oss << format.substr(0, pos) << value;
      FormatImpl(oss, format.substr(pos + 2), args...);
    } else {
      oss << format;
    }
  }

  static void FormatImpl(std::ostringstream &oss, const std::string &format) {
    oss << format;
  }
};

}  // namespace bustub