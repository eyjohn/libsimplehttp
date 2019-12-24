#include "simplehttpserver.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <string>

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;    // from <boost/beast/http.hpp>
namespace asio = boost::asio;    // from <boost/asio.hpp>
using tcp = asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using namespace std;

SimpleHttpServer::SimpleHttpServer(const string &address, unsigned short port,
                                   unsigned int thread_count)
    : d_pool(thread_count),
      d_address(asio::ip::make_address(address)),
      d_port(port) {}

void SimpleHttpServer::run(Callback cb) {
  // The io_context is required for all I/O
  asio::io_context ioc{1};

  // The acceptor receives incoming connections
  tcp::acceptor acceptor{ioc, {d_address, d_port}};
  for (;;) {
    // This will receive the new connection
    tcp::socket socket{ioc};

    // Block until we get a connection
    acceptor.accept(socket);

    // Process request in thread pool, transferring ownership of the socket
    asio::post(d_pool, [&cb, socket = std::move(socket)]() mutable {
      beast::error_code ec;
      beast::flat_buffer buffer;

      for (;;) {
        // Read the HTTP Request
        http::request<http::string_body> http_req;
        http::read(socket, buffer, http_req, ec);
        if (ec == http::error::end_of_stream) {
          break;
        }
        if (ec) {
          return;
        }

        http::response<http::string_body> http_resp;
        http_resp.version(http_req.version());
        http_resp.keep_alive(http_req.keep_alive());
        http_resp.set(http::field::server, BOOST_BEAST_VERSION_STRING);

        // Check the request
        if (http_req.method() == http::verb::get) {
          // Convert Request
          const Request req{string(http_req.target()), http_req.body().data()};

          // Invoke the callback
          // Going to be lazy and not check for exceptions for now
          const Response resp = cb(req);

          // Generate the HTTP Response
          http_resp.set(http::field::content_type,
                        "text/plain");  // "application/octet-stream"
          http_resp.result(http::int_to_status(resp.code));

          if (resp.data.has_value()) {
            http_resp.body() = resp.data.value();
            http_resp.prepare_payload();
          }
        } else {
          http_resp.set(http::field::content_type, "text/plain");
          http_resp.result(http::status::bad_request);
          http_resp.body() = "Unknown HTTP-method";
          http_resp.prepare_payload();
        }

        http::write(socket, http_resp, ec);
        if (ec) {
          return;
        }

        if (http_resp.need_eof()) {
          // This means we should close the connection, usually because
          // the response indicated the "Connection: close" semantic.
          break;
        }
      }

      // Send a TCP shutdown
      socket.shutdown(tcp::socket::shutdown_send, ec);
    });
  }
}
