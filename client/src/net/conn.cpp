#include "conn.hpp"
#include "net.hpp"
#include <format>

namespace net {
Connection::Connection() {
  this->tcp = new TCP("127.0.0.1");
  this->udp = new UDP(9091);
  this->udp->bind();

  printf("UDP listening on %s\n", this->udp->address().c_str());
}

void Connection::launch(std::string str) {
  this->tcp->connect();
  this->tcp->write(std::format("LAUNCH;{}", str).c_str());
  this->tcp->close();
}

void Connection::data(std::string str) {
  this->tcp->connect();
  this->tcp->write(std::format("DATA;{}", str).c_str());
  this->tcp->close();
}

void Connection::recvblock(
    std::function<void(std::string, std::vector<char>)> on_read) {

  try {
    // printf("read start\n");
    this->udp->recv([&](std::string value) {
      // printf("received\n");
      // lock.lock();
      if (value.rfind("0:", 0) == 0) {
        cmd = value;
        cmd.erase(0, 2);
      }

      if (value.rfind("1:", 0) == 0) {
        len_str = value;
        len_str.erase(0, 2);
        buf_len = atoi(len_str.c_str());
      }

      if (value.rfind("2:", 0) == 0) {
        data_str = value;
        data_str.erase(0, 2);
        buf.insert(buf.end(), data_str.begin(), data_str.end());
        if (buf.size() >= buf_len) {
          on_read(cmd, buf);
          buf.erase(buf.begin(), buf.end());
        }
      }
      // lock.unlock();
    });
    // printf("finished read\n");
  } catch (std::runtime_error &ex) {
    printf("%s\n", ex.what());
  } catch (std::exception &ex) {
    printf("Unhandled Exception: %s\n", ex.what());
  }
}
} // namespace net
