#include "macros.hpp"
#include <GL/freeglut_std.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>

#include "net/conn.hpp"
#include "net/net.hpp"

#include <GL/glut.h>
#include <vector>

using namespace net;

int WIDTH = 320;
int HEIGHT = 240;

Connection *conn = NULL;
std::mutex *conn_mutex = new std::mutex();
TCP *tcp = new TCP("127.0.0.1");
GLuint textureID;
int window;
#define FORMAT GL_RGB
#define INTERNAL_FORMAT GL_RGB
std::mutex datamutex = std::mutex();
uint8_t *d = (uint8_t *)malloc(WIDTH * HEIGHT * 3);
size_t size = 0;
bool sending = false;

void InitTexture(GLsizei width, GLsizei height, uint8_t *data) {
  // glActiveTexture(GL_TEXTURE);
  GLCMD(glGenTextures(1, &textureID));
  GLCMD(glBindTexture(GL_TEXTURE_2D, textureID));
  GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, FORMAT, width, height, 0,
                     INTERNAL_FORMAT, GL_UNSIGNED_BYTE, data));
  GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
  GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

void UpdateTexture(GLsizei width, GLsizei height, uint8_t *data) {
  // GLCMD(glGenTextures(1, &textureID));
  GLCMD(glBindTexture(GL_TEXTURE_2D, textureID));
  GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, FORMAT, width, height, 0,
                     INTERNAL_FORMAT, GL_UNSIGNED_BYTE, data));
  GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
  GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

void updateData() { GLCMD(glutPostRedisplay()); }

// Clears the current window and draws a triangle.
void display() {
  conn->data();

  GLCMD(glClearColor(1.0f, 0.0f, 1.0f, 1.0f));
  GLCMD(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  GLCMD(glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL));
  GLCMD(glPushMatrix());
  GLCMD(glTranslatef(0, 0, 0));

  conn_mutex->lock();
  UpdateTexture(WIDTH, HEIGHT, d);
  conn_mutex->unlock();

  GLCMD(glBindTexture(GL_TEXTURE_2D, textureID));

  GLCMD(glEnable(GL_TEXTURE_2D));

  glBegin(GL_QUADS);

  glTexCoord2f(0.0, 1.0);
  glVertex2f(-1.0, -1.0);
  glTexCoord2f(1.0, 1.0);
  glVertex2f(1.0, -1.0);
  glTexCoord2f(1.0, 0.0);
  glVertex2f(1.0, 1.0);
  glTexCoord2f(0.0, 0.0);
  glVertex2f(-1.0, 1.0);
  glEnd();

  GLCMD(glutSwapBuffers());
  GLCMD(glPopMatrix());
}

void key(unsigned char key, int x, int y) {
  switch (key) {
  case 'a':
    --WIDTH;
    break;
  case 'd':
    ++WIDTH;
    break;
  case 'w':
    --HEIGHT;
    break;
  case 's':
    ++HEIGHT;
    break;
  };
  // printf("%d, %d\n", WIDTH, HEIGHT);
}

int main(int argc, char *argv[]) {
  try {

    conn = new Connection();
    std::thread thread = std::thread([=]() {
      while (true) {
        conn_mutex->lock();
        conn->recvblock([=](std::string cmd, size_t buf_len,
                            std::vector<uint8_t> data) -> void {
          size = data.size();
          // printf("%ld / %ld\n", size, buf_len);
          d = data.data();

          sending = false;
        });
        conn_mutex->unlock();
      }
    });
    GLCMD(glutInit(&argc, argv));

    GLCMD(glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB));

    // Position window at (80,80)-(HEIGHT,380) and give it a title.
    GLCMD(glutInitWindowPosition(80, 80));
    GLCMD(glutInitWindowSize(WIDTH, HEIGHT));
    GLCMD(glutCreateWindow("A Simple Triangle"));

    // Tell GLUT that whenever the main window needs to be repainted that it
    // should call the function display().
    GLCMD(glutIdleFunc(updateData));
    GLCMD(glutDisplayFunc(display));
    GLCMD(glutKeyboardFunc(key));
    conn->launch("glxgears");

    InitTexture(WIDTH, HEIGHT, NULL);

    GLCMD(glutMainLoop());

  } catch (std::runtime_error &ex) {
    printf("Runtime Error: %s\n", ex.what());
  } catch (std::exception &ex) {
    printf("Unhandled Exception: %s\n", ex.what());
  }

  return 0;
}