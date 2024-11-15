#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <stdexcept>
#include "../../inc/emulator/Terminal.hpp"

using namespace std;

Terminal::Terminal(){
  interrupt = false;
  term_in = 0;
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  if (tcgetattr(STDIN_FILENO, &t) < 0) {
      throw std::runtime_error("Failed to execute tcgetattr during terminal initialization.");
  }
  oldFlags = t.c_lflag;
  t.c_lflag &= ~(ICANON | ECHO);
  if (tcsetattr(STDIN_FILENO, TCSANOW, &t) < 0) {
      throw std::runtime_error("Failed to execute tcsetattr during terminal initialization.");
  }
}

void Terminal::update() {
  char rd;
  if (read(STDIN_FILENO, &rd, 1) >= 0) {
    term_in = rd;
    interrupt = true;
  }
}

void Terminal::write(uint32_t data) {
    cout << char(data & 0xFF) << std::flush;
}

Terminal::~Terminal() {
    t.c_lflag = oldFlags;
    tcsetattr(STDIN_FILENO, TCSADRAIN, &t);
}