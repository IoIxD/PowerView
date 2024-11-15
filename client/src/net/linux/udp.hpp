#include "../udp.hpp"
#include <thread>
#include <vector>

namespace net {
class LinuxUDP : public UDPVirtual {
  uint16_t port;
  int recvfd, connfd = 0;
  int len;
  struct sockaddr_in recvaddr, servaddr, cli;
  struct msghdr msg;
  struct iovec iov[1];
  struct cmsghdr *cmptr;
  struct timeval timeout;
  bool reading = false;

  char *buf;
  std::vector<std::thread> threads = std::vector<std::thread>();
  bool open = false;

public:
  LinuxUDP(uint16_t port);
  void bind();
  void close();
  std::string address();
  bool recv(std::function<void(std::vector<uint8_t>, bool *)>);
  ~LinuxUDP();
  friend class Connection;
};

} // namespace net