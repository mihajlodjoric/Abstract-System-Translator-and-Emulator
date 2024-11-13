#ifndef TERMINAL_CPP
#define TERMINAL_CPP
#include <iostream>
#include <termios.h>

class Terminal {
public:
  bool interrupt = false;
  termios t;
  tcflag_t oldFlags;
  uint32_t term_in;
  Terminal();
  void update();
  void write(uint32_t);
  ~Terminal();
};

#endif //TERMINAL_HPP