#include <arpa/inet.h>
#include <cstring>
#include <functional>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace net {
class TCPVirtual {
public:
  virtual void connect() = 0;
  virtual void close() = 0;
  virtual std::string read() = 0;
  virtual void write(const char *buf) = 0;
};

} // namespace net
