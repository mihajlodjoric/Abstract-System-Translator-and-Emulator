#include "../../inc/emulator/Emulator.hpp"
#include <fstream>
#include <iomanip>
#include "../../inc/emulator/Error.hpp"


void readFromFile(const std::string& filename, std::map<uint32_t, uint8_t>& memory) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        throw std::ios_base::failure("Failed to open file for reading");
    }

    // Read the size of the map
    uint32_t mapSize;
    inFile.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));
    if (!inFile) {
        throw std::ios_base::failure("Failed to read map size");
    }

    // Read each key-value pair
    memory.clear();  // Clear the map to ensure it's empty before reading
    for (uint32_t i = 0; i < mapSize; ++i) {
        uint32_t key;
        uint8_t value;
        inFile.read(reinterpret_cast<char*>(&key), sizeof(key));
        inFile.read(reinterpret_cast<char*>(&value), sizeof(value));
        if (!inFile) {
            throw std::ios_base::failure("Failed to read key-value pair");
        }
        memory[key] = value;
    }

    inFile.close();
}


void Emulator::start(){
    while(running){
      try
      {
        Instruction ins = readInstruction(pc);
        pc += 4;
        executeInstruction(ins);
        terminal.update();
        handleInterrupt();
      }
      catch(exception& e)
      {
        pushWord(status);
        pushWord(pc);
        cause = 1;
        status = status & (~0x1);
        pc = handler;
      }
    }
    printProcessorState();
  }

void Emulator::printMemory(){
  for(auto& d: memory){
    uint32_t addr = d.first;
    uint32_t value = d.second;

    if(addr % 8 == 0){
      cout << endl << hex << setw(4) << setfill('0') << addr << dec << setfill(' ') << ": "; 
    }
    cout << hex << setw(2) << setfill('0') << value << dec << setfill(' ') << ' ';
  }
  cout << endl;
}

Instruction Emulator::readInstruction(uint32_t address){
    Instruction ins(memory[address], memory[address+1], memory[address+2], memory[address+3]);
    return ins;
  }

int Emulator::readWord(uint32_t address){
  if(address == 0xFFFFFF04){
    return terminal.term_in;
  }
  else{
    return (static_cast<int>(memory[address])     |
    static_cast<int>(memory[address + 1]) << 8  |
    static_cast<int>(memory[address + 2]) << 16 |
    static_cast<int>(memory[address + 3]) << 24);

  }
}

int Emulator::readByte(uint32_t address){
  return memory[address];
}

void Emulator::writeByte(uint32_t address, uint8_t byte){
  memory[address] = byte;
}

void Emulator::writeWord(uint32_t address, int value){
  if (address + 3 >= 0xFFFFFFFF) {
        return;
  }
  if(address == 0xFFFFFF00){
    terminal.write(value);
  }
  else{
    memory[address]     = static_cast<uint8_t>(value);
    memory[address + 1] = static_cast<uint8_t>(value >> 8);
    memory[address + 2] = static_cast<uint8_t>(value >> 16);
    memory[address + 3] = static_cast<uint8_t>(value >> 24);
  }
}

int Emulator::readCSR(int reg){
  if(reg == 0) return status;
  if(reg == 1) return handler;
  if(reg == 2) return cause;
  throw InvalidCSRRegister(reg);
}

void Emulator::writeCSR(int reg, int value){
  if(reg == 0) status = value;
  else if(reg == 1) handler = value;
  else if(reg == 2) cause = value;
  else throw InvalidCSRRegister(reg);
}

int Emulator::readGPR(int reg){
  if(reg < 0 || reg > 15) throw InvalidGPRRegister(reg);
  if(reg == 0) return 0;
  if(reg == 15) return pc;
  return GPR[reg];
}

void Emulator::writeGPR(int reg, int value){
  if(reg < 0 || reg > 15) throw InvalidGPRRegister(reg);
  if(reg == 0) return;
  if(reg == 15) pc = value;
  GPR[reg] = value;
}

void Emulator::pushWord(int value){
  //if((uint32_t) readGPR(14) < 4) throw StackOverflow();
  writeGPR(14, readGPR(14) - 4);
  writeWord(readGPR(14), value);
}

int Emulator::popWord(){
  if((uint32_t) readGPR(14) > 0xFFFFFFFF - 4) throw StackOverflow();
  int value = readWord(readGPR(14));
  writeGPR(14, readGPR(14) + 4);
  return value;
}

void Emulator::executeInstruction(Instruction instruction){
  int oc = instruction.oc();
  switch (oc) {
  case 0x0:
    //halt
    running = false;
    break;
  case 0x1:
    executeInterruptInstruction();
    break;
  case 0x2:
    executeCallInstruction(instruction);
    break;
  case 0x3:
    executeJumpInstruction(instruction);
    break;
  case 0x4:
    executeSwapInstruction(instruction);
    break;
  case 0x5:
    executeAritheticInstruction(instruction);
    break;
  case 0x6:
    executeLogicInstruction(instruction);
    break;
  case 0x7:
    executeShiftInstruction(instruction);
    break;
  case 0x8:
    executeStoreInstruction(instruction);
    break;
  case 0x9:
    executeLoadInstruction(instruction);
    break;
  default:
    throw InvalidCode();
    break;
  }
}

void Emulator::executeSwapInstruction(Instruction instruction){
  int mod = instruction.mod();
  int regB = instruction.regB();
  int regC = instruction.regC();
  int temp;

  switch (mod)
  {
  case 0x0:
    // XCHG
    temp = readGPR(regB);
    writeGPR(regB, readGPR(regC));
    writeGPR(regC, temp);
    break;
  default:
    throw InvalidCode();
    break;
  }
}

void Emulator::executeInterruptInstruction(){
  pushWord(status);
  pushWord(pc);
  cause = 4;
  status = status & (~0x1);
  pc = handler;
}

void Emulator::executeAritheticInstruction(Instruction instruction){
  int mod = instruction.mod();
  int regA = instruction.regA();
  int regB = instruction.regB();
  int regC = instruction.regC();
  switch (mod)
  {
  case 0x0:
    // ADD
    writeGPR(regA, readGPR(regB) + readGPR(regC));
    break;
  case 0x1:
    // SUB
    writeGPR(regA, readGPR(regB) - readGPR(regC));
    break;
  case 0x2:
    // MUL
    writeGPR(regA, readGPR(regB) * readGPR(regC));
    break;
  case 0x3:
    // DIV
    if(readGPR(regC) == 0){
      throw DivisionByZero();
    }
    writeGPR(regA, readGPR(regB) / readGPR(regC));
    break;
  default:
    throw InvalidCode();
    break;
  }
}

void Emulator::executeLogicInstruction(Instruction instruction){
  int mod = instruction.mod();
  int regA = instruction.regA();
  int regB = instruction.regB();
  int regC = instruction.regC();
  switch (mod)
  {
  case 0x0:
    // NOT
    writeGPR(regA, ~readGPR(regB));
    break;
  case 0x1:
    // AND
    writeGPR(regA, readGPR(regB) & readGPR(regC));
    break;
  case 0x2:
    // OR
    writeGPR(regA, readGPR(regB) | readGPR(regC));
    break;
  case 0x3:
    // XOR
    writeGPR(regA, readGPR(regB) ^ readGPR(regC));
    break;
  default:
    throw InvalidCode();
    break;
  }
}

void Emulator::executeShiftInstruction(Instruction instruction){
  int mod = instruction.mod();
  int regA = instruction.regA();
  int regB = instruction.regB();
  int regC = instruction.regC();
  switch (mod)
  {
  case 0x0:
    // SHR
    writeGPR(regA, readGPR(regB) << readGPR(regC));
    break;
  case 0x1:
    // SHL
    writeGPR(regA, readGPR(regB) >> readGPR(regC));
    break;
  default:
    throw InvalidCode();
    break;
  }
}

void Emulator::executeCallInstruction(Instruction instruction){
  int mod = instruction.mod();
  int regA = instruction.regA();
  int regB = instruction.regB();
  int regC = instruction.regC();
  int disp = instruction.disp();


  switch (mod)
  {
  case 0x0:
    // push pc; pc <= gpr[A] + gpr[B] + D
    pushWord(pc);
    pc = readGPR(regA) + readGPR(regB) + disp;
    break;
  case 0x1:
    // push pc; pc <= gpr[A] + gpr[B] + D
    pushWord(pc);
    pc = readWord(readGPR(regA) + readGPR(regB) + disp);
    break;
  default:
    throw InvalidCode();
    break;
  }
}

void Emulator::executeJumpInstruction(Instruction instruction){
  int mod = instruction.mod();
  int regA = instruction.regA();
  int regB = instruction.regB();
  int regC = instruction.regC();
  int disp = instruction.disp();

  switch (mod)
  {
  case 0x0:
    // pc <= gpr[A] + D
    writeGPR(15, readGPR(regA) + disp);
    break;
  case 0x1:
    // if (gpr[B] == gpr[C]) pc<=gpr[A]+D;
    if(readGPR(regB) == readGPR(regC)){
      writeGPR(15, readGPR(regA) + disp);
    }
    break;
  case 0x2:
    //  if (gpr[B] != gpr[C]) pc<=gpr[A]+D;
    if(readGPR(regB) != readGPR(regC)){
      writeGPR(15, readGPR(regA) + disp);
    }
    break;
  case 0x3:
    // if (gpr[B] signed> gpr[C]) pc<=gpr[A]+D;
    if(readGPR(regB) > readGPR(regC)){
      writeGPR(15, readGPR(regA) + disp);
    }
    break;
  case 0x8:
    // pc <= mem32[gpt[A] + D]
    writeGPR(15, readWord(readGPR(regA) + disp));
    break;
  case 0x9:
    // if (gpr[B] == gpr[C]) pc<=mem32[gpr[A]+D];
    if(readGPR(regB) == readGPR(regC)){
      writeGPR(15, readWord(readGPR(regA) + disp));
    }
    break;
  case 0xA:
    //  if (gpr[B] != gpr[C]) pc<=mem32[gpr[A]+D];
    if(readGPR(regB) != readGPR(regC)){
      writeGPR(15, readWord(readGPR(regA) + disp));
    }
    break;
  case 0xB:
    // if (gpr[B] signed> gpr[C]) pc<=mem32[gpr[A]+D];
    if(readGPR(regB) > readGPR(regC)){
      writeGPR(15, readWord(readGPR(regA) + disp));
    }
    break;
  default:
    throw InvalidCode();
    break;
  }

}

void Emulator::executeLoadInstruction(Instruction instruction){
  int mod = instruction.mod();
  int regA = instruction.regA();
  int regB = instruction.regB();
  int regC = instruction.regC();
  int disp = instruction.disp();

  switch (mod)
  {
  case 0x0:
    // gpr[A] <= csr[B]
    writeGPR(regA, readCSR(regB));
    break;
  case 0x1:
    // gpr[A] <= gpr[B] + D
    writeGPR(regA, readGPR(regB) + disp);
    break;
  case 0x2:
    // gpr[A] <= mem32[gpr[B] + gpr[C] + D] 
    writeGPR(regA, readWord(readGPR(regB) + readGPR(regC) + disp));
    break;
  case 0x3:
    // gpr[A] <= mem32[gpr[B]]; gpr[B] += disp
    writeGPR(regA, readWord(readGPR(regB)));
    writeGPR(regB, readGPR(regB) + disp);
    break;
  case 0x4:
    // csr[A] <= gpr[B]
    writeCSR(regA, readGPR(regB));
    break;
  case 0x5:
    // csr[A] <= csr[B] | D
    writeCSR(regA, readCSR(regB) | disp);
    break;
  case 0x6:
    // csr[A] <= mem32[gpr[B] + gpr[C] + D]
    writeCSR(regA, readWord(readGPR(regB) + readGPR(regC) + disp));
    break;
  case 0x7:
    // csr[A] <= mem32[gpr[B]]; gpr[B] += disp
    writeCSR(regA, readWord(readGPR(regB)));
    writeGPR(regB, readGPR(regB) + disp);
    break;
  default:
    throw InvalidCode();
    break;
  }
}

void Emulator::executeStoreInstruction(Instruction instruction){
  int mod = instruction.mod();
  int regA = instruction.regA();
  int regB = instruction.regB();
  int regC = instruction.regC();
  int disp = instruction.disp();
  uint32_t address;

  switch (mod)
  {
  case 0x0:
    // mem32[gpr[A]+gpr[B]+D] <= gpr[C]
    writeWord(readGPR(regA) + readGPR(regB) + disp, readGPR(regC));
    break;
  case 0x2:
    // mem32[mem32[gpr[A]+gpr[B]+D]] <= gpr[C]
    address = readWord(readGPR(regA) + readGPR(regB) + disp);
    writeWord(address, readGPR(regC));
    break;
  case 0x1:
    // gpr[A] <= gpr[A] + D; mem32[gpr[A]] <= gpr[C];
    writeGPR(regA, readGPR(regA) + disp);
    writeWord(readGPR(regA), readGPR(regC));
    break;
  default:
    throw InvalidCode();
    break;
  }
}

bool Emulator::interruptsMaksed() {
  return status & 0b0100;
}

bool Emulator::terminalMaksed() {
  return status & 0b0010;
}

bool Emulator::timerMasked() {
  return status & 0b0001;
}

void Emulator::handleInterrupt() {
  if(!running || interruptsMaksed()) {
    return;
  }

  bool interrupt = false;
  int cause = 0;

  if(terminal.interrupt && !terminalMaksed()){
    terminal.interrupt = false;
    interrupt = true;
    cause = 3;
  }
  if(interrupt){
    pushWord(status);
    pushWord(pc);
    this->cause = cause;
    status = status & (~0x1);
    pc = handler;
  }

}

void Emulator::printProcessorState() {
  cout << "Emulated processor executed halt instruction" << endl;
  cout << "Emulated processor state:" << endl;
  for (int row = 0; row < 4; ++row) {
    for (int col = 0; col < 4; ++col) {
      int idx = row * 4 + col;
      std::cout << "r" << idx << " = 0x" 
                << std::setw(8) << std::setfill('0') 
                << std::hex << readGPR(idx) << dec
                << " ";
    }
    std::cout << std::endl;
    }
}
