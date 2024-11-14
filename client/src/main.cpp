#include <cstdlib>
#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <stdlib.h>
#include <thread>
#include <unistd.h>

#include "net/conn.hpp"
#include "net/net.hpp"

using namespace net;

int main(int argc, char *argv[]) {
  std::thread thread;
  try {
    Connection *conn = new Connection();
    std::mutex *conn_mutex = new std::mutex();
    TCP *tcp = new TCP("127.0.0.1");
    std::thread thread = std::thread([=]() {
      while (true) {
        conn->recvblock([](std::string cmd, std::vector<char> data) -> void {
          std::cout << "length: " << data.size() << std::endl;
        });
      }
    });

    conn->launch("firefox");

    while (true) {
      conn->data("firefox");
    }
  } catch (std::runtime_error &ex) {
    printf("Runtime Error: %s\n", ex.what());
  } catch (std::exception &ex) {
    printf("Unhandled Exception: %s\n", ex.what());
  }

  return 0;
}