#include "tcp.hpp"
#include <format>

void TCPVirtual::launch(std::string str) {
  this->connect();
  this->write(std::format("LAUNCH;{}", str).c_str());
  this->close();
}

void TCPVirtual::data(std::string str) {
  this->connect();
  this->write(std::format("DATA;{}", str).c_str());
  this->close();
}