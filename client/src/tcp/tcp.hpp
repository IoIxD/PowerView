#include <thread>
#define SEND_PORT "9090"
#define RECV_PORT "9091"

#include <string>
class TCPVirtual {
  virtual void connect(std::string buf) = 0;
  virtual void listen() = 0;
  virtual void close() = 0;
  virtual std::string read() = 0;
  virtual void write(const char *buf) = 0;
};

#ifdef __linux__
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class LinuxTCP : TCPVirtual {
private:
  int recv_ip, send_ip;
  struct addrinfo hints, *recv_servinfo, *send_servinfo;
  int sockfd;

  std::thread thread;

  char s[INET6_ADDRSTRLEN];
  std::string addr;

public:
  LinuxTCP(std::string addr);
  ~LinuxTCP();

  void connect() { this->connect(this->addr); }
  void connect(std::string buf);
  void listen();
  void close();
  std::string read();
  void write(const char *buf);
};
typedef LinuxTCP TCP;
#endif