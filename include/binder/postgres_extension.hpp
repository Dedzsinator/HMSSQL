#pragma once

#include "postgres_parser.hpp"
#include "nodes/nodes.hpp"
#include "nodes/parsenodes.hpp"
#include "../common/util/string_util.h"

namespace hmssql {

class PostgresParserExtension {
public:
    static void ExtendParser();

private:
    static duckdb_libpgquery::PGKeyword* FindKeyword(const char* text, std::vector<duckdb_libpgquery::PGKeyword>& keywords);
};

} // namespace hmssql