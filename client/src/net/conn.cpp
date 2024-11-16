#include "conn.hpp"
#include "net.hpp"
#include <format>
#include <iostream>
#include <string>

namespace net {
Connection::Connection() {
  this->tcp = new TCP("127.0.0.1");
  this->udp = new UDP(9091);
  this->udp->bind();

  printf("UDP listening on %s\n", this->udp->address().c_str());
}

void Connection::launch(std::string prog) {
  this->prog = prog;
  this->tcp->connect();
  this->tcp->write(std::format("LAUNCH;{}", prog).c_str());
  this->tcp->close();
}

void Connection::data() {
  this->tcp->connect();
  this->tcp->write(std::format("DATA;{}", prog).c_str());
  this->tcp->close();
}

void Connection::resend(size_t offset) {
  this->tcp->connect();
  this->tcp->write(std::format("DATA;{};{}", prog, offset).c_str());
  this->tcp->close();
}

void Connection::recvblock(
    std::function<void(std::string, size_t, std::vector<uint8_t>)> on_read) {
  try {
    bool read = true;
    // printf("read start\n");
    bool needs_retry =
        this->udp->recv([&](std::vector<uint8_t> value, bool *reading) {
          *reading = read;
          lock.lock();
          std::string str =
              std::string(value.data(), value.data() + value.size());

          std::string id_str = str;

          if (id_str.rfind("DATA:", 5) >= 0) {
            // get packet number
            id_str.erase(id_str.begin(), id_str.begin() + 5);
            int id = atoi(id_str.c_str());

            // printf("ID: %d\n", id);

            // get packet length
            len_str = str;
            len_str.erase(0, len_str.find(";") + 1);
            auto data_start = len_str.find(";");
            len_str.erase(data_start);
            // std::cout << "GOT LENGTH " << len_str << std::endl;
            buf_len = atoi(len_str.c_str());

            // append the whole packet to the buffer.
            data_str = value;
            data_str.erase(data_str.begin(), data_str.begin() + data_start);
            // printf("data_str.size(): %ld\n", data_str.size());

            for (int i = 0; i < 25; i++) {
              std::cout << std::to_string(data_str.at(i)) << ",";
            }
            std::cout << std::endl;
            // printf("\n%ld\n", data_str.size());
            ordered_buf.insert_or_assign(id, data_str);
          }
          lock.unlock();
        });
    // printf("%ld >= %d\n ", ordered_buf.size(), buf_len);

    if (needs_retry && ordered_buf.size() >= 1) {
      for (it = ordered_buf.begin(); it != ordered_buf.end(); ++it) {
        buf.insert(buf.end(), it->second.begin(), it->second.end());
      }

      on_read(cmd, buf_len, buf);
      buf.erase(buf.begin(), buf.end());
      ordered_buf.erase(ordered_buf.begin(), ordered_buf.end());
    }

    // printf("finished read\n");
  } catch (std::runtime_error &ex) {
    printf("%s\n", ex.what());
  } catch (std::exception &ex) {
    printf("Unhandled Exception: %s\n", ex.what());
  }
}

} // namespace net