#include <cstdlib>
#include <iostream>

#include "simplehttpclient.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: testclient <host> <port> <path>\n"
              << "Example:\n"
              << "    testclient localhost 8080 /\n";
    return EXIT_FAILURE;
  }
  string host = argv[1];
  int port = std::atoi(argv[2]);
  string path = argv[3];
  auto client = SimpleHttpClient(host, port);
  auto resp = client.get(path);
  std::cout << "Response code: " << resp.code
            << " data: " << resp.data.value_or("(None)") << std::endl;
  return EXIT_SUCCESS;
}
