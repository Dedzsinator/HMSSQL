

#include "../include/binder/keyword_helper.h"
#include "../include/binder/binder.h"
#include "../include/common/util/string_util.h"

namespace hmssql {

auto KeywordHelper::IsKeyword(const std::string &text) -> bool { return Binder::IsKeyword(text); }

auto KeywordHelper::RequiresQuotes(const std::string &text) -> bool {
  for (size_t i = 0; i < text.size(); i++) {
    if (i > 0 && (text[i] >= '0' && text[i] <= '9')) {
      continue;
    }
    if (text[i] >= 'a' && text[i] <= 'z') {
      continue;
    }
    if (text[i] == '_') {
      continue;
    }
    return true;
  }
  return IsKeyword(text);
}

auto KeywordHelper::WriteOptionallyQuoted(const std::string &text, char quote) -> std::string {
  if (!RequiresQuotes(text)) {
    return text;
  }
  return std::string(1, quote) + StringUtil::Replace(text, std::string(1, quote), std::string(2, quote)) +
         std::string(1, quote);
}

}  // namespace hmssql
