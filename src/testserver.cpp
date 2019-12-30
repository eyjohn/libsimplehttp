#include <opentracing/mocktracer/json_recorder.h>
#include <opentracing/mocktracer/tracer.h>

#include <csignal>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>

#include "common.h"
#include "simplehttpserver.h"

using namespace opentracing;
using namespace opentracing::mocktracer;

// Capturing signal handler
class SignalHandler {
 public:
  static void signal(int sig, std::function<void(int)> handler) {
    s_handlers[sig] = handler;
    std::signal(sig, [](int sig) { SignalHandler::s_handlers[sig](sig); });
  }

 private:
  static std::map<int, std::function<void(int)>> s_handlers;
};
std::map<int, std::function<void(int)>> SignalHandler::s_handlers;

std::shared_ptr<Tracer> create_file_json_tracer(const char *filename) {
  // OpenTracing is very specific about the types of pointers.
  return std::shared_ptr<Tracer>{new MockTracer{MockTracerOptions{
      std::unique_ptr<Recorder>{new JsonRecorder{std::unique_ptr<std::ostream>{
          new ofstream(filename, ios::out | ios::app)}}}}}};
}

int main(int argc, char **argv) {
  // Check and consume args
  if (argc != 3) {
    std::cerr << "Usage: testserver <address> <port>\n"
              << "Example:\n"
              << "    testclient 0.0.0.0 8080\n";
    return EXIT_FAILURE;
  }
  string address = argv[1];
  int port = std::atoi(argv[2]);

  // Instantiate global tracer
  auto tracer = create_file_json_tracer("testserver.traces.json");
  Tracer::InitGlobal(tracer);

  // Create the HTTP server
  auto server = SimpleHttpServer(address, port);

  // Handle SIGINT, SIGTERM to stop the server
  auto signal_handler = [&server](int sig) { server.stop(); };
  SignalHandler::signal(SIGINT, signal_handler);
  SignalHandler::signal(SIGTERM, signal_handler);

  // Start the HTTP server
  server.run([](const auto &request) {
    std::cout << "Handling Request: " << request.path
              << " data: " << request.data.value_or("(None)") << std::endl;

    string response = "You've requested the path: " + request.path;
    return Response{200, response};
  });
  tracer->Close();
  return EXIT_SUCCESS;
}
