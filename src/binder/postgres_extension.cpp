#include "../../include/binder/postgres_extension.hpp"
#include "postgres_parser.hpp"
#include <algorithm>

namespace hmssql {

void PostgresParserExtension::ExtendParser() {
    // Get the keyword list once and store it
    auto keywords = duckdb::PostgresParser::KeywordList();

    // Add keywords with proper enums
    for (auto& kw : keywords) {
        if (StringUtil::Equals(kw.text, "CREATE")) {
            kw.category = duckdb_libpgquery::PGKeywordCategory::PG_KEYWORD_RESERVED;
        } else if (StringUtil::Equals(kw.text, "DATABASE")) {
            kw.category = duckdb_libpgquery::PGKeywordCategory::PG_KEYWORD_RESERVED;
        } else if (StringUtil::Equals(kw.text, "USE")) {
            kw.category = duckdb_libpgquery::PGKeywordCategory::PG_KEYWORD_RESERVED;
        }
    }
}

duckdb_libpgquery::PGKeyword* PostgresParserExtension::FindKeyword(const char* text, std::vector<duckdb_libpgquery::PGKeyword>& keywords) {
    // Convert to lowercase for case-insensitive comparison
    std::string lower_text = text;
    std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
    
    // Search through existing keywords
    for (auto& kw : keywords) {
        if (StringUtil::Equals(kw.text, lower_text)) {
            return &kw;
        }
    }
    return nullptr;
}

} // namespace hmssql