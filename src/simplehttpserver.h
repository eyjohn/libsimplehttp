#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <functional>
#include <string>

#include "common.h"

namespace simplehttp {

class SimpleHttpServer {
 public:
  using Callback = std::function<Response(const Request &)>;

  SimpleHttpServer(const std::string &address = "0.0.0.0",
                   unsigned short port = 80);

  void run(Callback cb);
  void stop();

 private:
  boost::asio::io_context d_io_context;
  boost::asio::ip::address d_address;
  unsigned short d_port;
};

}  // namespace simplehttp
