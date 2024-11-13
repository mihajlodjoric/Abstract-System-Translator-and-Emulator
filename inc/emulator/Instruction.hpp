#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP


#include <iostream>
using namespace std;

struct Instruction{
private:
  uint32_t code;
public:
  Instruction(int byte1, int byte2, int byte3, int byte4){
    code = (byte4 << 24) ^ (byte3 << 16) ^ (byte2 << 8) ^ byte1;
  }
  int oc() {return code >> 28; }
  int mod() {return code >> 24 & 0xF; }
  int regA() {return code >> 20 & 0xF; }
  int regB() {return code >> 16 & 0xF; }
  int regC() {return code >> 12 & 0xF; }
  int disp() {
    if(code & 0x800){
      return code | 0xFFFFF000;
    }
    else{
      return code & 0x00000FFF;
    }
  }

  void print(){
    cout << hex << code << dec << endl;
  }
};

#endif //INSTRUCTION_HPP