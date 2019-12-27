#include "simplehttpclient.h"

#include <opentracing/propagation.h>
#include <opentracing/tracer.h>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <optional>
#include <string>

#include "beast_carrier.h"

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;    // from <boost/beast/http.hpp>
namespace net = boost::asio;     // from <boost/asio.hpp>
using tcp = net::ip::tcp;        // from <boost/asio/ip/tcp.hpp>

using namespace std;
using namespace opentracing;

SimpleHttpClient::SimpleHttpClient(const string& host, unsigned short port)
    : d_host(host), d_port(port) {}

Response SimpleHttpClient::make_request(const Request& request) {
  // The io_context is required for all I/O
  net::io_context ioc;

  // These objects perform our I/O
  tcp::resolver resolver(ioc);
  beast::tcp_stream stream(ioc);

  // Look up the domain name
  auto const results = resolver.resolve(d_host, std::to_string(d_port));

  // Make the connection on the IP address we get from a lookup
  stream.connect(results);

  // Set up an HTTP GET request message
  http::request<http::string_body> http_req{http::verb::get, request.path, 11};
  http_req.set(http::field::host, d_host);
  http_req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  // Embed the tracing context into HTTP Header
  auto tracer = opentracing::Tracer::Global();
  auto span = tracer->ScopeManager().ActiveSpan();
  span->tracer().Inject(span->context(),
                        make_boost_beast_http_headers_writer(http_req));

  // Send the HTTP request to the remote host
  http::write(stream, http_req);

  // This buffer is used for reading and must be persisted
  beast::flat_buffer buffer;

  // Declare a container to hold the response
  http::response<http::string_body> http_res;

  // Receive the HTTP response
  http::read(stream, buffer, http_res);

  // Gracefully close the socket
  beast::error_code ec;
  stream.socket().shutdown(tcp::socket::shutdown_both, ec);

  // not_connected happens sometimes
  // so don't bother reporting it.
  if (ec && ec != beast::errc::not_connected) throw beast::system_error{ec};

  // If we get here then the connection is closed gracefully

  return {http_res.result_int(), http_res.body().data()};
}
