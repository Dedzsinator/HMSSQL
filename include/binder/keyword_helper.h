//===----------------------------------------------------------------------===//
//                         HMSSQL
//
// binder/keyword_helper.h
//
//
//===----------------------------------------------------------------------===//



#pragma once

#include <string>

#include "../include/common/config.h"

namespace hmssql {

class KeywordHelper {
 public:
  /** Return true if the given text matches a keyword of the parser. */
  static auto IsKeyword(const std::string &text) -> bool;

  /** Return true if the given std::string needs to be quoted when written as an identifier. */
  static auto RequiresQuotes(const std::string &text) -> bool;

  // Writes a std::string that is optionally quoted + escaped so it can be used as an identifier
  static auto WriteOptionallyQuoted(const std::string &text, char quote = '"') -> std::string;
};

}  // namespace hmssql
