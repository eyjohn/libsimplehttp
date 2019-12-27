#pragma once

#include <optional>
#include <string>

struct Request {
  std::string path;
  std::optional<std::string> data;
};

struct Response {
  unsigned int code;
  std::optional<std::string> data;
};
