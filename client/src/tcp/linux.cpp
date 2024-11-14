#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

#ifdef __linux__
#include "tcp.hpp"
#define MAXDATASIZE 100 // max number of bytes we can get at once

#define RUNTIME_ERROR(st)                                                      \
  throw std::runtime_error(std::format("At {}:{}, {}: {}",                     \
                                       strrchr("/" __FILE__, '/') + 1,         \
                                       __LINE__, __FUNCTION__, st))

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

LinuxTCP::LinuxTCP(std::string addr) {
  this->addr = addr;
  if ((send_ip =
           getaddrinfo(addr.c_str(), SEND_PORT, &hints, &send_servinfo)) != 0) {
    RUNTIME_ERROR(gai_strerror(send_ip));
  }
  if ((recv_ip = getaddrinfo("127.0.0.1", RECV_PORT, &hints, &recv_servinfo)) !=
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

void LinuxTCP::listen(
    std::function<void(std::string, std::vector<char>)> on_read) {
  try {
    thread = std::thread([=]() {
      int recvfd, connfd = 0;
      unsigned int len;
      struct sockaddr_in recvaddr, servaddr, cli;

      // socket create and verification
      recvfd = socket(AF_INET, SOCK_DGRAM, 0);
      if (recvfd == -1) {
        RUNTIME_ERROR(strerror(errno));
      }
      bzero(&servaddr, sizeof(servaddr));

      // assign IP, PORT
      servaddr.sin_family = AF_INET;
      servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
      servaddr.sin_port = htons(9091);

      // Binding newly created socket to given IP and verification
      if ((bind(recvfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
        // When running this on Linux, if we start the progrma too soon after
        // having last started it, there's a complaint about the address being
        // in use; the one that we can verify with netstat is not being used.
        // Waiting up to a minute fixes this, which I'm fine with since most
        // people other then developers will be using two different computers
        // anyways.
        if (errno == 98) {
          for (int i = 0; i <= 10; i++) {
            int sleep_time = (i + 1) * 5;
            printf("[%i secs]\t%s, this is a known (weird) Linux bug "
                   "that goes away eventually\n",
                   sleep_time, strerror(errno));
            if ((bind(recvfd, (struct sockaddr *)&servaddr,
                      sizeof(servaddr))) == 0) {
              break;
            }
            sleep(sleep_time);
            if (i == 10) {
              RUNTIME_ERROR(strerror(errno));
            }
          }
        } else {
          RUNTIME_ERROR(strerror(errno));
        }
      }

      struct sockaddr_in addr;
      socklen_t socklen = sizeof(addr);

      bzero(&addr, sizeof(addr));
      if (getsockname(recvfd, (struct sockaddr *)&addr, &socklen) == -1) {
        RUNTIME_ERROR(strerror(errno));
      };
      char ip[17];
      inet_ntop(AF_INET, &addr.sin_addr, (char *)&ip, 16);
      ip[16] = '\0';
      printf("Listening on %s:%d\n", &ip, ntohs(addr.sin_port));

      struct msghdr msg;
      struct iovec iov[1];
      struct cmsghdr *cmptr;
      char *buf = (char *)malloc(1024);
      iov[0].iov_base = buf;
      iov[0].iov_len = 1024;
      msg.msg_iov = iov;
      msg.msg_iovlen = 1;

      std::string cmd = std::string();
      std::string buf_str = std::string();
      std::string len_str = std::string();
      std::string data_str = std::string();
      int buf_len = 0;
      std::vector<char> data = std::vector<char>();
      // Accept the data packet from client and verification
      while (connfd >= 0) {

        len = recvmsg(recvfd, &msg, 0);
        if (len < 0) {
          RUNTIME_ERROR(strerror(errno));
        } else {
          if (len != 0) {
            buf_str = std::string(buf, buf + len);
            if (buf_str.rfind("0:", 0) == 0) {
              cmd = std::string(buf, buf + len);
            }

            if (buf_str.rfind("1:", 0) == 0) {
              len_str = buf_str;
              len_str.erase(0, 2);
              buf_len = atoi(len_str.c_str());
            }

            if (buf_str.rfind("2:", 0) == 0) {
              data_str = buf_str;
              data_str.erase(0, 2);
              data.insert(data.end(), data_str.begin(), data_str.end());
              if (data.size() >= buf_len) {
                on_read(cmd, data);
                data.erase(data.begin(), data.end());
              }
            }
          }
        }
      }
      ::close(recvfd);
    });
  } catch (std::runtime_error &ex) {
    printf("%s\n", ex.what());
  } catch (std::exception &ex) {
    printf("Unhandled Exception: %s\n", ex.what());
  }
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

#endif