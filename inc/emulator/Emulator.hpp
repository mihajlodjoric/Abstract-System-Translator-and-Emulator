#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include <iostream>
#include <map>
#include "Instruction.hpp"
#include "Terminal.hpp"
using namespace std;

void readFromFile(const std::string& filename, std::map<uint32_t, uint8_t>& memory);

class Emulator{
private:
  map<uint32_t, uint8_t> memory;
  int pc = 0x40000000;
  int GPR[16] = {0};
  int status = 0, handler =  0, cause = 0;
  bool running = true;

  Terminal terminal;
public:
  Emulator(string inputName){
    readFromFile(inputName, memory);
  }

  Instruction readInstruction(uint32_t address);

  int readWord(uint32_t address);

  int readByte(uint32_t address);

  void writeByte(uint32_t address, uint8_t byte);

  void writeWord(uint32_t address, int value);

  int readGPR(int reg);

  void writeGPR(int reg, int value);

  int readCSR(int reg);

  void writeCSR(int reg, int value);

  void pushWord(int value);
  
  int popWord();

  void printMemory();

  void printProcessorState();

  void executeInstruction(Instruction ins);

  void executeInterruptInstruction();
  void executeCallInstruction(Instruction ins);
  void executeJumpInstruction(Instruction ins);
  void executeSwapInstruction(Instruction ins);
  void executeAritheticInstruction(Instruction ins);
  void executeLogicInstruction(Instruction ins);
  void executeShiftInstruction(Instruction ins);
  void executeLoadInstruction(Instruction ins);
  void executeStoreInstruction(Instruction ins);

  bool interruptsMaksed();
  bool terminalMaksed();
  bool timerMasked();
  void handleInterrupt();


  void start();


};




#endif //EMULATOR_HPP