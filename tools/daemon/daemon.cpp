#include <iostream>
#include <string>
#include "binder/binder.h"
#include "common/hmssql_instance.h"
#include "common/exception.h"
#include "common/util/string_util.h"
#include "libfort/lib/fort.hpp"
#include "utf8proc/utf8proc.h"
#include "../../third_party/httplib/httplib.h"
#include "../../third_party/json/json.hpp"
#include "../../third_party/spdlog/spdlog.h"
#include "../../third_party/spdlog/sinks/basic_file_sink.h"

using json = nlohmann::json;

auto GetWidthOfUtf8(const void *beg, const void *end, size_t *width) -> int {
  size_t computed_width = 0;
  utf8proc_ssize_t n;
  utf8proc_ssize_t size = static_cast<const char *>(end) - static_cast<const char *>(beg);
  auto pstring = static_cast<utf8proc_uint8_t const *>(beg);
  utf8proc_int32_t data;
  while ((n = utf8proc_iterate(pstring, size, &data)) > 0) {
    computed_width += utf8proc_charwidth(data);
    pstring += n;
    size -= n;
  }
  *width = computed_width;
  return 0;
}

// Function to handle SQL queries and return JSON response
auto HandleSqlQuery(hmssql::HMSSQL &hmssql, const std::string &query) -> json {
  json response;
  try {
    auto writer = hmssql::FortTableWriter();
    hmssql.ExecuteSql(query, writer);
    for (const auto &table : writer.tables_) {
      std::stringstream ss;
      ss << table;
      response["result"].push_back(ss.str());
    }
    response["status"] = "success";
    spdlog::info("Query executed successfully: {}", query);
  } catch (hmssql::Exception &ex) {
    response["status"] = "error";
    response["message"] = ex.what();
    spdlog::error("Query execution failed: {}. Error: {}", query, ex.what());
  }
  return response;
}

auto main(int argc, char **argv) -> int {
  ft_set_u8strwid_func(&GetWidthOfUtf8);

  auto hmssql = std::make_unique<hmssql::HMSSQL>("test.db");

  // Initialize spdlog
  auto logger = spdlog::basic_logger_mt("basic_logger", "logs/daemon.log");
  spdlog::set_default_logger(logger);
  spdlog::set_level(spdlog::level::info); // Set global log level to info
  spdlog::flush_on(spdlog::level::info);

  spdlog::info("Starting HMSSQL daemon...");

  // Start HTTP server
  httplib::Server svr;

  // Add route for SQL queries
  svr.Post("/query", [&](const httplib::Request &req, httplib::Response &res) {
    auto query = req.body;
    spdlog::info("Received query: {}", query);
    auto response = HandleSqlQuery(*hmssql, query);
    res.set_content(response.dump(), "application/json");
  });

  // Set up static file serving
  svr.set_mount_point("/", "./tools/web/static");

  // Add CORS headers
  svr.set_default_headers({
    {"Access-Control-Allow-Origin", "*"},
    {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
    {"Access-Control-Allow-Headers", "Content-Type"}
  });

  // Start listening
  svr.listen("0.0.0.0", 8080);

  return 0;
}