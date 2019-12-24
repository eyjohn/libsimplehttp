#include <string>
#include <optional>

using namespace std;

class SimpleHttpClient {
public:

  struct Response {
    unsigned int code;
    optional<string> data;
  };

  SimpleHttpClient(const string &host, unsigned short port = 80);

  // Make a HTTP GET request, returning the body of the payload
  Response get(const string &path);

private:
  string d_host;
  unsigned short d_port;
};