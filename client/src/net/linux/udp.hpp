#include "../udp.hpp"
#include <thread>
#include <vector>

namespace net {
class LinuxUDP : public UDPVirtual {
  uint16_t port;
  int recvfd, connfd = 0;
  unsigned int len;
  struct sockaddr_in recvaddr, servaddr, cli;
  struct msghdr msg;
  struct iovec iov[1];
  struct cmsghdr *cmptr;
  struct timeval timeout;

  char *buf;
  std::vector<std::thread> threads = std::vector<std::thread>();

public:
  LinuxUDP(uint16_t port);
  void bind();
  std::string address();
  void recv(std::function<void(std::string)>);
  ~LinuxUDP();
};

} // namespace net