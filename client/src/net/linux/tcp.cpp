#ifdef __linux__

#include "tcp.hpp"
#include "../../macros.hpp"

#include <cerrno>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace net {

#define MAXDATASIZE 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

LinuxTCP::LinuxTCP(std::string addr) {
  this->addr = addr;
  if ((send_ip = getaddrinfo(addr.c_str(), "9090", &hints, &send_servinfo)) !=
      0) {
    RUNTIME_ERROR(gai_strerror(send_ip));
  }
  if ((recv_ip = getaddrinfo("127.0.0.1", "9091", &hints, &recv_servinfo)) !=
      0) {
    RUNTIME_ERROR(gai_strerror(recv_ip));
  }
}

void LinuxTCP::connect(std::string addr) {
  if ((sockfd = socket(send_servinfo->ai_family, send_servinfo->ai_socktype,
                       send_servinfo->ai_protocol)) == -1) {
    RUNTIME_ERROR(strerror(errno));
  }

  if (::connect(sockfd, send_servinfo->ai_addr, send_servinfo->ai_addrlen) ==
      -1) {
    ::close(sockfd);
    RUNTIME_ERROR(strerror(errno));
  }

  inet_ntop(send_servinfo->ai_family,
            get_in_addr((struct sockaddr *)send_servinfo->ai_addr), s,
            sizeof s);
}

void LinuxTCP::close() { ::close(sockfd); }

LinuxTCP::~LinuxTCP() {
  freeaddrinfo(send_servinfo);
  freeaddrinfo(recv_servinfo);
}

std::string LinuxTCP::read() {
  std::string buf = std::string();
  char tmp = '\n';
  while (recv(sockfd, &tmp, 1, MSG_WAITFORONE) > 0 && tmp != '\0') {
    printf("%c\n", tmp);
    buf.push_back(tmp);
  }
  return buf;
}

void LinuxTCP::write(const char *buf) {
  if (send(sockfd, buf, strlen(buf), 0) == -1) {
    RUNTIME_ERROR(strerror(errno));
  };
}

} // namespace net
#endif
