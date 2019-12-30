#include <string>

#include "common.h"

namespace simplehttp {

class SimpleHttpClient {
 public:
  SimpleHttpClient(const std::string& host, unsigned short port = 80);

  // Make a HTTP request, returning the body of the payload
  Response make_request(const Request& request);

 private:
  std::string d_host;
  unsigned short d_port;
};

}  // namespace simplehttp
