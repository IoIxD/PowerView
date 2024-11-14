#include <arpa/inet.h>
#include <cstring>
#include <functional>
#include <netdb.h>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace net {
class UDPVirtual {
public:
  virtual void bind() = 0;
  virtual std::string address() = 0;
  virtual void recv(std::function<void(std::string)>) = 0;
};
} // namespace net