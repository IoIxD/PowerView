#include "net.hpp"
#include <mutex>

namespace net {
class Connection {
  TCP *tcp;
  UDP *udp;

  std::mutex lock = std::mutex();
  std::string cmd = std::string();
  std::string len_str = std::string();
  std::string data_str = std::string();
  int buf_len = 0;
  std::vector<char> buf = std::vector<char>();

public:
  Connection();
  void recvblock(std::function<void(std::string, std::vector<char>)> on_read);
  void launch(std::string str);
  void data(std::string str);
};

} // namespace net
