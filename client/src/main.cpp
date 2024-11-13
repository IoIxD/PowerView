#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>

#include "tcp/tcp.hpp"

int main(int argc, char *argv[]) {
  try {
    TCP *tcp = new TCP("127.0.0.1");
    tcp->listen();

    while (true) {
      tcp->connect();
      tcp->write("ping");
      tcp->close();
    }
  } catch (std::runtime_error &ex) {
    printf("Runtime Error: %s\n", ex.what());
  } catch (std::exception &ex) {
    printf("Unhandled Exception: %s\n", ex.what());
  }

  return 0;
}