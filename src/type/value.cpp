//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// value.cpp
//
// Identification: src/type/value.cpp
//
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <string>
#include <utility>
#include <regex>

#include "../include/common/exception.h"
#include "../include/type/value.h"

namespace hmssql {
Value::Value(const Value &other) {
  type_id_ = other.type_id_;
  size_ = other.size_;
  manage_data_ = other.manage_data_;
  value_ = other.value_;
  switch (type_id_) {
    case TypeId::VARCHAR:
      if (size_.len_ == BUSTUB_VALUE_NULL) {
        value_.varlen_ = nullptr;
      } else {
        if (manage_data_) {
          value_.varlen_ = new char[size_.len_];
          memcpy(value_.varlen_, other.value_.varlen_, size_.len_);
        } else {
          value_ = other.value_;
        }
      }
      break;
    default:
      value_ = other.value_;
  }
}

auto Value::ToString() const -> std::string {
  switch (type_id_) {
    case TypeId::BOOLEAN:
      return value_.boolean_ ? "true" : "false";
    case TypeId::TINYINT:
      return std::to_string(value_.tinyint_);
    case TypeId::SMALLINT:
      return std::to_string(value_.smallint_);
    case TypeId::INTEGER:
      return std::to_string(value_.integer_);
    case TypeId::BIGINT:
      return std::to_string(value_.bigint_);
    case TypeId::DECIMAL:
      return std::to_string(value_.decimal_);
    case TypeId::VARCHAR:
      return std::string(value_.varlen_, size_.len_);
    default:
      throw Exception("Unsupported type for ToString");
  }
}

auto Value::operator=(Value other) -> Value & {
  Swap(*this, other);
  return *this;
}

// BOOLEAN and TINYINT
Value::Value(TypeId type, int8_t i) : Value(type) {
  switch (type) {
    case TypeId::BOOLEAN:
      value_.boolean_ = i;
      size_.len_ = (value_.boolean_ == BUSTUB_BOOLEAN_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::TINYINT:
      value_.tinyint_ = i;
      size_.len_ = (value_.tinyint_ == BUSTUB_INT8_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::SMALLINT:
      value_.smallint_ = i;
      size_.len_ = (value_.smallint_ == BUSTUB_INT16_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::INTEGER:
      value_.integer_ = i;
      size_.len_ = (value_.integer_ == BUSTUB_INT32_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::BIGINT:
      value_.bigint_ = i;
      size_.len_ = (value_.bigint_ == BUSTUB_INT64_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    default:
      throw Exception(ExceptionType::INCOMPATIBLE_TYPE, "Invalid Type for one-byte Value constructor");
  }
}

// SMALLINT
Value::Value(TypeId type, int16_t i) : Value(type) {
  switch (type) {
    case TypeId::BOOLEAN:
      value_.boolean_ = i;
      size_.len_ = (value_.boolean_ == BUSTUB_BOOLEAN_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::TINYINT:
      value_.tinyint_ = i;
      size_.len_ = (value_.tinyint_ == BUSTUB_INT8_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::SMALLINT:
      value_.smallint_ = i;
      size_.len_ = (value_.smallint_ == BUSTUB_INT16_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::INTEGER:
      value_.integer_ = i;
      size_.len_ = (value_.integer_ == BUSTUB_INT32_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::BIGINT:
      value_.bigint_ = i;
      size_.len_ = (value_.bigint_ == BUSTUB_INT64_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::TIMESTAMP:
      value_.timestamp_ = i;
      size_.len_ = (value_.timestamp_ == BUSTUB_TIMESTAMP_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    default:
      throw Exception(ExceptionType::INCOMPATIBLE_TYPE, "Invalid Type for two-byte Value constructor");
  }
}

// INTEGER
Value::Value(TypeId type, int32_t i) : Value(type) {
  switch (type) {
    case TypeId::BOOLEAN:
      value_.boolean_ = i;
      size_.len_ = (value_.boolean_ == BUSTUB_BOOLEAN_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::TINYINT:
      value_.tinyint_ = i;
      size_.len_ = (value_.tinyint_ == BUSTUB_INT8_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::SMALLINT:
      value_.smallint_ = i;
      size_.len_ = (value_.smallint_ == BUSTUB_INT16_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::INTEGER:
      value_.integer_ = i;
      size_.len_ = (value_.integer_ == BUSTUB_INT32_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::BIGINT:
      value_.bigint_ = i;
      size_.len_ = (value_.bigint_ == BUSTUB_INT64_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::TIMESTAMP:
      value_.timestamp_ = i;
      size_.len_ = (value_.timestamp_ == BUSTUB_TIMESTAMP_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    default:
      throw Exception(ExceptionType::INCOMPATIBLE_TYPE, "Invalid Type for integer_ Value constructor");
  }
}

// BIGINT and TIMESTAMP
Value::Value(TypeId type, int64_t i) : Value(type) {
  switch (type) {
    case TypeId::BOOLEAN:
      value_.boolean_ = i;
      size_.len_ = (value_.boolean_ == BUSTUB_BOOLEAN_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::TINYINT:
      value_.tinyint_ = i;
      size_.len_ = (value_.tinyint_ == BUSTUB_INT8_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::SMALLINT:
      value_.smallint_ = i;
      size_.len_ = (value_.smallint_ == BUSTUB_INT16_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::INTEGER:
      value_.integer_ = i;
      size_.len_ = (value_.integer_ == BUSTUB_INT32_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::BIGINT:
      value_.bigint_ = i;
      size_.len_ = (value_.bigint_ == BUSTUB_INT64_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::TIMESTAMP:
      value_.timestamp_ = i;
      size_.len_ = (value_.timestamp_ == BUSTUB_TIMESTAMP_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    default:
      throw Exception(ExceptionType::INCOMPATIBLE_TYPE, "Invalid Type for eight-byte Value constructor");
  }
}

// BIGINT and TIMESTAMP
Value::Value(TypeId type, uint64_t i) : Value(type) {
  switch (type) {
    case TypeId::BIGINT:
      value_.bigint_ = i;
      size_.len_ = (value_.bigint_ == BUSTUB_INT64_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    case TypeId::TIMESTAMP:
      value_.timestamp_ = i;
      size_.len_ = (value_.timestamp_ == BUSTUB_TIMESTAMP_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    default:
      throw Exception(ExceptionType::INCOMPATIBLE_TYPE, "Invalid Type for timestamp_ Value constructor");
  }
}

// DECIMAL
Value::Value(TypeId type, double d) : Value(type) {
  switch (type) {
    case TypeId::DECIMAL:
      value_.decimal_ = d;
      size_.len_ = (value_.decimal_ == BUSTUB_DECIMAL_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    default:
      throw Exception(ExceptionType::INCOMPATIBLE_TYPE, "Invalid Type for double Value constructor");
  }
}

// DECIMAL
Value::Value(TypeId type, float f) : Value(type) {
  switch (type) {
    case TypeId::DECIMAL:
      value_.decimal_ = f;
      size_.len_ = (value_.decimal_ == BUSTUB_DECIMAL_NULL ? BUSTUB_VALUE_NULL : 0);
      break;
    default:
      throw Exception(ExceptionType::INCOMPATIBLE_TYPE, "Invalid Type for float value constructor");
  }
}

// VARCHAR
Value::Value(TypeId type, const char *data, uint32_t len, bool manage_data) : Value(type) {
  switch (type) {
    case TypeId::VARCHAR:
      if (data == nullptr) {
        value_.varlen_ = nullptr;
        size_.len_ = BUSTUB_VALUE_NULL;
      } else {
        manage_data_ = manage_data;
        if (manage_data_) {
          assert(len < BUSTUB_VARCHAR_MAX_LEN);
          value_.varlen_ = new char[len];
          assert(value_.varlen_ != nullptr);
          size_.len_ = len;
          memcpy(value_.varlen_, data, len);
        } else {
          // FUCK YOU GCC I do what I want.
          value_.const_varlen_ = data;
          size_.len_ = len;
        }
      }
      break;
    default:
      throw Exception(ExceptionType::INCOMPATIBLE_TYPE, "Invalid Type  for variable-length Value constructor");
  }
}

Value::Value(TypeId type, const std::string &data) : Value(type) {
  switch (type) {
    case TypeId::VARCHAR: {
      manage_data_ = true;
      // TODO(TAs): How to represent a null string here?
      uint32_t len = static_cast<uint32_t>(data.length()) + 1;
      value_.varlen_ = new char[len];
      assert(value_.varlen_ != nullptr);
      size_.len_ = len;
      memcpy(value_.varlen_, data.c_str(), len);
      break;
    }
    default:
      throw Exception(ExceptionType::INCOMPATIBLE_TYPE, "Invalid Type  for variable-length Value constructor");
  }
}

// delete allocated char array space
Value::~Value() {
  switch (type_id_) {
    case TypeId::VARCHAR:
      if (manage_data_) {
        delete[] value_.varlen_;
      }
      break;
    default:
      break;
  }
}

auto Value::CheckComparable(const Value &o) const -> bool {
  switch (GetTypeId()) {
    case TypeId::BOOLEAN:
      return (o.GetTypeId() == TypeId::BOOLEAN || o.GetTypeId() == TypeId::VARCHAR);
    case TypeId::TINYINT:
    case TypeId::SMALLINT:
    case TypeId::INTEGER:
    case TypeId::BIGINT:
    case TypeId::DECIMAL:
      switch (o.GetTypeId()) {
        case TypeId::TINYINT:
        case TypeId::SMALLINT:
        case TypeId::INTEGER:
        case TypeId::BIGINT:
        case TypeId::DECIMAL:
        case TypeId::VARCHAR:
          return true;
        default:
          break;
      }  // SWITCH
      break;
    case TypeId::VARCHAR:
      // Anything can be cast to a string!
      return true;
      break;
    default:
      break;
  }  // END OF SWITCH
  return false;
}

auto Value::CheckInteger() const -> bool {
  switch (GetTypeId()) {
    case TypeId::TINYINT:
    case TypeId::SMALLINT:
    case TypeId::INTEGER:
    case TypeId::BIGINT:
      return true;
    default:
      break;
  }
  return false;
}

auto Value::Like(const Value &other) const -> CmpBool {
  if (IsNull() || other.IsNull()) {
    return CmpBool::CmpNull;
  }
  if (type_id_ != TypeId::VARCHAR || other.type_id_ != TypeId::VARCHAR) {
    throw Exception("LIKE can only be applied to VARCHAR types");
  }
  std::string pattern = other.value_.varlen_;
  pattern = std::regex_replace(pattern, std::regex("%"), ".*");
  pattern = std::regex_replace(pattern, std::regex("_"), ".");
  std::regex regex_pattern(pattern);
  return std::regex_match(value_.varlen_, regex_pattern) ? CmpBool::CmpTrue : CmpBool::CmpFalse;
}
}  // namespace hmssql
