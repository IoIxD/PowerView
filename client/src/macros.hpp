#include <format>

#define RUNTIME_ERROR(st)                                                      \
  throw std::runtime_error(                                                    \
      std::format("At {}:{}, {}: {}", __FILE__, __LINE__, __FUNCTION__, st))
