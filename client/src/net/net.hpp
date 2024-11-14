#ifndef __NET_HPP
#define __NET_HPP
#ifdef __linux__
#include "linux/tcp.hpp"
#include "linux/udp.hpp"
#endif

#ifdef __linux__
namespace net {
typedef LinuxTCP TCP;
typedef LinuxUDP UDP;
} // namespace net
#endif
#endif // __NET_HPP