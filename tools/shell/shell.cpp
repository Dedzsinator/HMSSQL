#include <iostream>
#include <string>
#include "binder/binder.h"
#include "common/hmssql_instance.h"
#include "common/exception.h"
#include "common/util/string_util.h"
#include "libfort/lib/fort.hpp"
#include "linenoise/linenoise.h"
#include "utf8proc/utf8proc.h"
#include "../../third_party/httplib/httplib.h"
#include "../../third_party/json/json.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

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

// NOLINTNEXTLINE
auto main(int argc, char **argv) -> int {
  ft_set_u8strwid_func(&GetWidthOfUtf8);

  auto hmssql = std::make_unique<hmssql::HMSSQL>("test.db");

  // Initialize spdlog
  auto logger = spdlog::basic_logger_mt("basic_logger", "logs/shell.log");
  spdlog::set_default_logger(logger);
  spdlog::set_level(spdlog::level::info); // Set global log level to info
  spdlog::flush_on(spdlog::level::info);

  spdlog::info("Starting HMSSQL shell...");

  auto default_prompt = "hmssql> ";
  auto emoji_prompt = "\U0001f6c1> ";  // the bathtub emoji
  bool use_emoji_prompt = false;
  bool disable_tty = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--emoji-prompt") == 0) {
      use_emoji_prompt = true;
      break;
    }
    if (strcmp(argv[i], "--disable-tty") == 0) {
      disable_tty = true;
      break;
    }
  }

  hmssql->GenerateMockTable();

  #ifndef ISDEBUG

  if (hmssql->buffer_pool_manager_ != nullptr) {
    hmssql->GenerateTestTable();
  }

  #endif

  std::cout << "Welcome to the HMSSQL shell! Type \\help to learn more." << std::endl << std::endl;

  // Start HTTP server
  httplib::Server svr;

  svr.Post("/query", [&](const httplib::Request &req, httplib::Response &res) {
    auto query = req.body;
    spdlog::info("Received query: {}", query);
    auto response = HandleSqlQuery(*hmssql, query);
    res.set_content(response.dump(), "application/json");
  });

  std::thread server_thread([&svr]() {
    svr.listen("0.0.0.0", 8080);
  });

  linenoiseHistorySetMaxLen(1024);
  linenoiseSetMultiLine(1);

  auto prompt = use_emoji_prompt ? emoji_prompt : default_prompt;

  while (true) {
    std::string query;
    bool first_line = true;
    while (true) {
      auto line_prompt = first_line ? prompt : "... ";
      if (!disable_tty) {
        char *query_c_str = linenoise(line_prompt);
        if (query_c_str == nullptr) {
          svr.stop();
          server_thread.join();
          return 0;
        }
        query += query_c_str;
        linenoiseFree(query_c_str);  // Free the allocated memory
        if (hmssql::StringUtil::EndsWith(query, ";") || hmssql::StringUtil::StartsWith(query, "\\")) {
          break;
        }
        query += " ";
      } else {
        std::string query_line;
        std::cout << line_prompt;
        std::getline(std::cin, query_line);
        if (!std::cin) {
          svr.stop();
          server_thread.join();
          return 0;
        }
        query += query_line;
        if (hmssql::StringUtil::EndsWith(query, ";") || hmssql::StringUtil::StartsWith(query, "\\")) {
          break;
        }
        query += "\n";
      }
      first_line = false;
    }

    if (query == "\\exit") {
      break;
    }

    if (!disable_tty) {
      linenoiseHistoryAdd(query.c_str());
    }

    try {
      auto writer = hmssql::FortTableWriter();
      hmssql->ExecuteSql(query, writer);
      for (const auto &table : writer.tables_) {
        std::cout << table;
      }
      spdlog::info("Query executed successfully: {}", query);
    } catch (hmssql::Exception &ex) {
      std::cerr << ex.what() << std::endl;
      spdlog::error("Query execution failed: {}. Error: {}", query, ex.what());
    }
  }

  svr.stop();
  server_thread.join();
  spdlog::info("Shutting down HMSSQL shell...");
  return 0;
}