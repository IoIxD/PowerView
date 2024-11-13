#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

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

void LinuxTCP::listen() {
  try {
    thread = std::thread([=]() {
      int recvfd, connfd = 0;
      unsigned int len;
      struct sockaddr_in recvaddr, servaddr, cli;

      // socket create and verification
      recvfd = socket(AF_INET, SOCK_STREAM, 0);
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

      // Now server is ready to listen and verification
      if ((::listen(recvfd, 5)) != 0) {
        RUNTIME_ERROR(strerror(errno));
      }
      len = sizeof(cli);

      // Accept the data packet from client and verification
      while (connfd >= 0) {
        connfd = ::accept(recvfd, (struct sockaddr *)&cli, &len);
        char buf = '\0';
        std::string fin = std::string();
        while (buf != '\n') {
          if (recv(connfd, &buf, 1, 0) == -1) {
            RUNTIME_ERROR(strerror(errno));
          } else {
            fin.push_back(buf);
          }
        }
        printf("%s", fin.c_str());
        ::close(connfd);
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