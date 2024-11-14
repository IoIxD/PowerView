#include "udp.hpp"
#include "../../macros.hpp"
#include <sys/socket.h>
#include <thread>

namespace net {

LinuxUDP::LinuxUDP(uint16_t port) {
  this->port = port;
  // socket create and verification
  recvfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (recvfd == -1) {
    RUNTIME_ERROR(strerror(errno));
  }
  bzero(&servaddr, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  buf = (char *)malloc(1024);
  iov[0].iov_base = buf;
  iov[0].iov_len = 1024;
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
}

void LinuxUDP::bind() {
  if ((::bind(recvfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
    RUNTIME_ERROR(strerror(errno));
  }
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;
  if (setsockopt(recvfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) <
      0) {
    RUNTIME_ERROR(strerror(errno));
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

void LinuxUDP::recv(std::function<void(std::string)> on_recv) {
  // threads.push_back(std::thread([&] {
  // Accept the data packet from client and verification

  // printf("recvmsg start\n");
  len = recvmsg(recvfd, &msg, MSG_DONTWAIT);
  // printf("recvmsg end\n");

  if (len == -1) {
    return;
  }
  if (len < 0) {
    RUNTIME_ERROR(strerror(errno));
  } else {
    if (len != 0 && len != -1) {
      on_recv(std::string(buf, buf + len));
    }
  }
  // }));
}

LinuxUDP::~LinuxUDP() { ::close(recvfd); }
} // namespace net