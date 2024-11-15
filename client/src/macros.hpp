#include <GL/gl.h>
#include <format>

#define __THROW_ERROR(err, __error__...) printf(__error__);
#define __THROW_FATAL_ERROR(err, __error__...)                                 \
  printf(__error__);                                                           \
  getchar();                                                                   \
  getchar();                                                                   \
  exit(err);

#define RUNTIME_ERROR(st)                                                      \
  throw std::runtime_error(                                                    \
      std::format("At {}:{}, {}: {}", __FILE__, __LINE__, __FUNCTION__, st))

void PauseIfGLError(const char *file, int line_num, const char *code);

#define GLCMD(cmd)                                                             \
  cmd;                                                                         \
  PauseIfGLError(__FILE__, __LINE__, #cmd)
