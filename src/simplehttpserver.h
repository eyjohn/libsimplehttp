#include <boost/asio/ip/address.hpp>
#include <boost/asio/thread_pool.hpp>
#include <functional>
#include <optional>
#include <string>

using namespace std;

class SimpleHttpServer {
 public:
  struct Request {
    string path;
    optional<string> data;
  };

  struct Response {
    unsigned int code;
    optional<string> data;
  };

  using Callback = function<Response(const Request &)>;

  SimpleHttpServer(const string &address = "0.0.0.0", unsigned short port = 80,
                   unsigned int thread_count = 1);

  void run(Callback cb);

 private:
  boost::asio::thread_pool d_pool;
  boost::asio::ip::address d_address;
  unsigned short d_port;
};