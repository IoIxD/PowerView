#include "udp.hpp"
#include "../../macros.hpp"
#include <cerrno>
#include <sys/socket.h>
#include <vector>

namespace net {

LinuxUDP::LinuxUDP(uint16_t port) {
  this->port = port;
  this->bind();
}

void LinuxUDP::bind() {
  if (!this->open) {
    recvfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (recvfd == -1) {
      RUNTIME_ERROR(strerror(errno));
    }
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    buf = (char *)malloc(2503);
    iov[0].iov_base = buf;
    iov[0].iov_len = 2504;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    if ((::bind(recvfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
      RUNTIME_ERROR(strerror(errno));
    }
    this->open = true;
  }
}

std::string LinuxUDP::address() {
  struct sockaddr_in addr;
  socklen_t socklen = sizeof(addr);

  bzero(&addr, sizeof(addr));
  if (getsockname(recvfd, (struct sockaddr *)&addr, &socklen) == -1) {
    RUNTIME_ERROR(strerror(errno));
  };
  char ip[17];
  inet_ntop(AF_INET, &addr.sin_addr, (char *)&ip, 16);
  ip[16] = '\0';
  return std::string(ip);
}

bool LinuxUDP::recv(std::function<void(std::vector<uint8_t>, bool *)> on_recv) {
  // Accept the data packet from client and verification
  len = recvmsg(recvfd, &msg, MSG_DONTWAIT);

  if (len == -1) {
    if (reading) {
      return true;
    } else {
      return false;
    }
  } else if (len < 0) {
    reading = false;
    RUNTIME_ERROR(strerror(errno));
  } else if (len >= 1) {
    reading = true;
    on_recv(std::vector<uint8_t>(buf, buf + len), &this->reading);
  }
  return false;
}

void LinuxUDP::close() {
  ::close(recvfd);
  this->open = false;
}

LinuxUDP::~LinuxUDP() { this->close(); }
} // namespace net