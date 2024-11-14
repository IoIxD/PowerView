#include "../tcp.hpp"

namespace net {
class LinuxTCP : public TCPVirtual {
private:
  int recv_ip, send_ip;
  struct addrinfo hints, *recv_servinfo, *send_servinfo;
  int sockfd;

  char s[INET6_ADDRSTRLEN];
  std::string addr;

public:
  LinuxTCP(std::string addr);
  ~LinuxTCP();

  void connect(std::string buf);
  void connect() { this->connect(this->addr); }
  void close();
  std::string read();
  void write(const char *buf);
};

} // namespace net