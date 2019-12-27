#include <opentracing/mocktracer/json_recorder.h>
#include <opentracing/mocktracer/tracer.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>

#include "common.h"
#include "simplehttpclient.h"

using namespace opentracing;
using namespace opentracing::mocktracer;

std::shared_ptr<Tracer> create_file_json_tracer(const char* filename) {
  // OpenTracing is very specific about the types of pointers.
  return std::shared_ptr<Tracer>{new MockTracer{MockTracerOptions{
      std::unique_ptr<Recorder>{new JsonRecorder{std::unique_ptr<std::ostream>{
          new ofstream(filename, ios::out | ios::app)}}}}}};
}

int main(int argc, char** argv) {
  // Check and consume args
  if (argc != 4) {
    std::cerr << "Usage: testclient <host> <port> <path>\n"
              << "Example:\n"
              << "    testclient localhost 8080 /\n";
    return EXIT_FAILURE;
  }
  string host = argv[1];
  int port = std::atoi(argv[2]);
  string path = argv[3];

  // Instantiate global tracer
  auto tracer = create_file_json_tracer("testclient.traces.json");
  Tracer::InitGlobal(tracer);

  // Instantiate the client
  auto client = SimpleHttpClient(host, port);

  // Make the request
  {
    // Life-time of span is only within this scope.
    auto span = std::shared_ptr<Span>{tracer->StartSpan("testclient.request")};
    auto scope = tracer->ScopeManager().Activate(span);
    auto resp = client.make_request(Request{path});
    std::cout << "Response code: " << resp.code
              << " data: " << resp.data.value_or("(None)") << std::endl;
  }

  // Close the tracer to collect spans
  tracer->Close();
  return EXIT_SUCCESS;
}
