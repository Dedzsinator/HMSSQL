//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// statement_type.h
//
// Identification: src/include/enums/statement_type.h
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include "../include/common/config.h"
#include "fmt/format.h"

namespace hmssql {

//===--------------------------------------------------------------------===//
// Statement Types
//===--------------------------------------------------------------------===//
enum class StatementType : uint8_t {
  INVALID_STATEMENT,        // invalid statement type
  SELECT_STATEMENT,         // select statement type
  INSERT_STATEMENT,         // insert statement type
  UPDATE_STATEMENT,         // update statement type
  CREATE_STATEMENT,         // create statement type
  DELETE_STATEMENT,         // delete statement type
  EXPLAIN_STATEMENT,        // explain statement type
  DROP_STATEMENT,           // drop statement type
  INDEX_STATEMENT,          // index statement type
  VARIABLE_SET_STATEMENT,   // set variable statement type
  VARIABLE_SHOW_STATEMENT,  // show variable statement type
  CREATE_VIEW_STATEMENT,
  CREATE_TEMP_TABLE_STATEMENT,
};

}  // namespace hmssql

template <>
struct fmt::formatter<hmssql::StatementType> : formatter<string_view> {
  template <typename FormatContext>
  auto format(hmssql::StatementType c, FormatContext &ctx) const {
    string_view name;
    switch (c) {
      case hmssql::StatementType::INVALID_STATEMENT:
        name = "Invalid";
        break;
      case hmssql::StatementType::SELECT_STATEMENT:
        name = "Select";
        break;
      case hmssql::StatementType::INSERT_STATEMENT:
        name = "Insert";
        break;
      case hmssql::StatementType::UPDATE_STATEMENT:
        name = "Update";
        break;
      case hmssql::StatementType::CREATE_STATEMENT:
        name = "Create";
        break;
      case hmssql::StatementType::DELETE_STATEMENT:
        name = "Delete";
        break;
      case hmssql::StatementType::EXPLAIN_STATEMENT:
        name = "Explain";
        break;
      case hmssql::StatementType::DROP_STATEMENT:
        name = "Drop";
        break;
      case hmssql::StatementType::INDEX_STATEMENT:
        name = "Index";
        break;
      case hmssql::StatementType::VARIABLE_SHOW_STATEMENT:
        name = "VariableShow";
        break;
      case hmssql::StatementType::VARIABLE_SET_STATEMENT:
        name = "VariableSet";
        break;
      case hmssql::StatementType::CREATE_VIEW_STATEMENT:         
        name = "CreateView";                                     
        break;
      case hmssql::StatementType::CREATE_TEMP_TABLE_STATEMENT:
        name = "CreateTempTable";                             
        break;
    }
    return formatter<string_view>::format(name, ctx);
  }
};
