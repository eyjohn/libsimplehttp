#pragma once

#include <opentracing/propagation.h>

#include <boost/algorithm/string/trim.hpp>
#include <boost/beast/http.hpp>

namespace simplehttp {

template <class Body, class Fields>
class BoostBeastHTTPHeadersWriter : public opentracing::HTTPHeadersWriter {
 public:
  BoostBeastHTTPHeadersWriter(
      boost::beast::http::request<Body, Fields>& request)
      : d_request(request) {}

  opentracing::expected<void> Set(
      opentracing::string_view key,
      opentracing::string_view value) const override {
    d_request.set(key.data(), value.data());
    return {};
  }

 private:
  boost::beast::http::request<Body, Fields>& d_request;
};

template <class Body, class Fields>
BoostBeastHTTPHeadersWriter<Body, Fields> make_boost_beast_http_headers_writer(
    boost::beast::http::request<Body, Fields>& request) {
  return {request};
}

template <class Body, class Fields>
class BoostBeastHTTPHeadersReader : public opentracing::HTTPHeadersReader {
 public:
  BoostBeastHTTPHeadersReader(
      const boost::beast::http::request<Body, Fields>& request)
      : d_request(request) {}

  opentracing::expected<opentracing::string_view> LookupKey(
      opentracing::string_view key) const override {
    auto it = d_request.find(key.data());
    if (it != d_request.end()) {
      return {read_value(it->value())};
    }
    return opentracing::make_unexpected(opentracing::key_not_found_error);
  }

  opentracing::expected<void> ForeachKey(
      std::function<opentracing::expected<void>(opentracing::string_view key,
                                                opentracing::string_view value)>
          f) const override {
    for (const auto& keyval : d_request) {
      f(read_key(keyval.name_string()), read_value(keyval.value()));
    }
    return {};
  }

 private:
  std::string& read_value(boost::string_view val) const {
    // Beast returns "val\n"
    d_value = std::string{val};
    boost::algorithm::trim(d_value);
    return d_value;
  }
  std::string& read_key(boost::string_view key) const {
    // Beast returns "key: val\n"
    d_key = std::string{key};
    d_key = d_key.substr(0, d_key.find(":"));
    return d_key;
  }
  const boost::beast::http::request<Body, Fields>& d_request;
  mutable std::string d_value;
  mutable std::string d_key;
};

template <class Body, class Fields>
BoostBeastHTTPHeadersReader<Body, Fields> make_boost_beast_http_headers_reader(
    const boost::beast::http::request<Body, Fields>& request) {
  return {request};
}

}  // namespace simplehttp
