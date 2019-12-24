#include <cstdlib>
#include <iostream>

#include "simplehttpserver.h"

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: testserver <address> <port>\n"
              << "Example:\n"
              << "    testclient 0.0.0.0 8080\n";
    return EXIT_FAILURE;
  }
  string address = argv[1];
  int port = std::atoi(argv[2]);

  auto server = SimpleHttpServer(address, port);
  server.run([](const auto &request) {
    std::cout << "Handling Request: " << request.path
              << " data: " << request.data.value_or("(None)") << std::endl;

    string response = "You've requested the path: " + request.path;
    return SimpleHttpServer::Response{200, response};
  });
  return EXIT_SUCCESS;
}
