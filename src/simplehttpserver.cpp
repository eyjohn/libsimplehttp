#include "simplehttpserver.h"

#include <opentracing/propagation.h>
#include <opentracing/tracer.h>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "beast_carrier.h"

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;    // from <boost/beast/http.hpp>
namespace asio = boost::asio;    // from <boost/asio.hpp>
using tcp = asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using namespace std;
using namespace opentracing;

namespace simplehttp {

namespace {
void fail(beast::error_code ec, char const* what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(const SimpleHttpServer::Callback& callback, tcp::socket socket)
      : callback_{callback}, stream_{move(socket)} {
    stream_.expires_after(std::chrono::seconds(30));
  }

  void run() { do_read(); }

 private:
  void do_close() { stream_.socket().shutdown(tcp::socket::shutdown_send); };

  void on_write(beast::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
      return fail(ec, "write");
    }

    bool close = response_.need_eof();

    // Reset the response
    response_ = {};

    if (close) {
      return do_close();
    }

    do_read();
  };

  // Requests a write on the session for the handler
  void do_write() {
    http::async_write(stream_, response_,
                      [self = shared_from_this()](
                          beast::error_code ec, std::size_t bytes_transferred) {
                        self->on_write(ec, bytes_transferred);
                      });
  };

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    // This means they closed the connection
    if (ec == http::error::end_of_stream) {
      return do_close();
    }

    if (ec) {
      return fail(ec, "read");
    }

    // Handle Request
    handle_request();

    // Reset the request
    request_ = {};

    do_write();
  }

  // Requests a read on the session for the handler
  void do_read() {
    http::async_read(stream_, buffer_, request_,
                     [self = shared_from_this()](
                         beast::error_code ec, std::size_t bytes_transferred) {
                       self->on_read(ec, bytes_transferred);
                     });
  };

  // Handle the HTTP request
  void handle_request() {
    // For easier readability
    auto& http_req = request_;
    auto& http_resp = response_;

    // Extract tracing context from request
    auto tracer = opentracing::Tracer::Global();
    auto span_context =
        tracer->Extract(make_boost_beast_http_headers_reader(http_req));

    // Start the span (possibly child of request trace)
    std::shared_ptr<Span> span;
    if (span_context) {
      span = std::shared_ptr<Span>{
          tracer->StartSpan("simplehttpserver.handlerequest",
                            {opentracing::ChildOf(span_context->get())})};
    } else {
      span = std::shared_ptr<Span>{
          tracer->StartSpan("simplehttpserver.handlerequest")};
    }
    span->SetTag("target", http_req.target().data());
    auto scope = tracer->ScopeManager().Activate(span);

    http_resp.version(http_req.version());
    http_resp.keep_alive(http_req.keep_alive());
    http_resp.set(http::field::server, BOOST_BEAST_VERSION_STRING);

    // Check the request
    if (http_req.method() == http::verb::get) {
      // Convert Request
      const Request req{string(http_req.target()), http_req.body().data()};

      // Invoke the callback
      // Going to be lazy and not check for exceptions for now
      const Response resp = callback_(req);

      // Generate the HTTP Response
      http_resp.set(http::field::content_type, "text/plain");
      // Perhaps change to "application/octet-stream"

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
  }

  const SimpleHttpServer::Callback& callback_;
  beast::tcp_stream stream_;
  http::request<http::string_body> request_;
  http::response<http::string_body> response_;
  beast::flat_buffer buffer_;
};

}  // namespace

SimpleHttpServer::SimpleHttpServer(const string& address, unsigned short port)
    : d_io_context(),
      d_address(asio::ip::make_address(address)),
      d_port(port) {}

void SimpleHttpServer::run(Callback cb) {
  // The acceptor receives incoming connections
  tcp::acceptor acceptor{d_io_context, {d_address, d_port}};

  // Accept connections for handler
  auto do_accept = [&](auto&& handler) -> void {
    acceptor.async_accept(asio::make_strand(d_io_context),
                          [&handler](beast::error_code ec, tcp::socket socket) {
                            handler(ec, std::move(socket), handler);
                          });
  };

  // Self referencing accept handler
  auto accept_handler = [&](beast::error_code ec, tcp::socket socket,
                            auto&& self) -> void {
    if (ec) {
      fail(ec, "accept");
    } else {
      // Required to serialize execution when running multithreaded
      asio::dispatch(socket.get_executor(),
                     [&, socket = std::move(socket)]() mutable {
                       auto session = make_shared<Session>(cb, move(socket));
                       session->run();
                     });
    }
    do_accept(self);
  };

  // Accept connections
  do_accept(accept_handler);

  // Run the server
  d_io_context.run();
}

void SimpleHttpServer::stop() { d_io_context.stop(); }

}  // namespace simplehttp
