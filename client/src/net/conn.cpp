#include "conn.hpp"
#include "net.hpp"
#include <format>
#include <iostream>
#include <map>
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
          std::string id_str =
              std::string(value.data(), value.data() + value.size());

          if (id_str.find(":") >= 1) {
            id_str.erase(id_str.find(":"));

            int id = atoi(id_str.c_str());
            std::string str =
                std::string(value.data(), value.data() + value.size());
            switch (id) {
            case 0:
              cmd = str;
              cmd.erase(cmd.find(':'));
              break;
            case 1:
              len_str = str;
              len_str.erase(0, id_str.size() + 1);
              buf_len = atoi(len_str.c_str());
              break;
            default:
              data_str = str;
              auto placeholder = id_str.size();
              data_str.erase(0, placeholder + 1);

              ordered_buf.insert_or_assign(
                  id, std::vector<uint8_t>(data_str.begin(), data_str.end()));
              break;
            };
          }
          lock.unlock();
        });
    // printf("%ld >= %d\r", ordered_buf.size() * 1500, buf_len);

    if (needs_retry && ordered_buf.size() >= 1) {
      std::map<int, typeof(buf)> sorted(ordered_buf.begin(), ordered_buf.end());

      for (auto it = sorted.begin(); it != sorted.end(); ++it) {
        buf.insert(buf.end(), it->second.begin(), it->second.end());
      }
      on_read(cmd, buf_len, buf);
      buf.erase(buf.begin(), buf.end());
    }

    // printf("finished read\n");
  } catch (std::runtime_error &ex) {
    printf("%s\n", ex.what());
  } catch (std::exception &ex) {
    printf("Unhandled Exception: %s\n", ex.what());
  }
}

} // namespace net