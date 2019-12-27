#include <opentracing/mocktracer/json_recorder.h>
#include <opentracing/mocktracer/tracer.h>

#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "common.h"
#include "simplehttpserver.h"

using namespace opentracing;
using namespace opentracing::mocktracer;

void signal_handler(int) { Tracer::Global()->Close(); }

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

  // Handle SIGINT by dumping trace data
  std::signal(SIGINT, signal_handler);

  // Create and start the HTTP server
  auto server = SimpleHttpServer(address, port);
  server.run([](const auto &request) {
    std::cout << "Handling Request: " << request.path
              << " data: " << request.data.value_or("(None)") << std::endl;

    string response = "You've requested the path: " + request.path;
    return Response{200, response};
  });
  tracer->Close();
  return EXIT_SUCCESS;
}
