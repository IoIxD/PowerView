#include "net.hpp"
#include <cstddef>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace net {
class Connection {
  TCP *tcp;
  UDP *udp;

  int buf_len = -1;

  std::mutex lock = std::mutex();
  std::string cmd = std::string();
  std::string len_str = std::string();
  std::string chunk_str = std::string();
  std::vector<uint8_t> data_str = std::vector<uint8_t>();
  std::vector<uint8_t> buf = std::vector<uint8_t>();

  std::unordered_map<ssize_t, typeof(buf)> ordered_buf =
      std::unordered_map<ssize_t, typeof(buf)>();

  std::unordered_map<ssize_t, typeof(buf)>::iterator it;

  std::string prog = std::string();

public:
  Connection();
  void recvblock(
      std::function<void(std::string, size_t, std::vector<uint8_t>)> on_read);
  void launch(std::string str);
  void data();
  void resend(size_t offset);
};

} // namespace net
