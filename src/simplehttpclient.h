#include <string>

#include "common.h"

using namespace std;

class SimpleHttpClient {
 public:
  SimpleHttpClient(const string& host, unsigned short port = 80);

  // Make a HTTP request, returning the body of the payload
  Response make_request(const Request& request);

 private:
  string d_host;
  unsigned short d_port;
};