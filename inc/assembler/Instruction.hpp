#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <iostream>
using namespace std;

#include "SectionTable.hpp"
#include "Operand.hpp"


enum ins {HALT = 0x00000000 , INT = 0x10000000, IRET = 0x970E0004, RET = 0x93FE0004,
          JMP = 0x30, CALL = 0x20,
          BEQ = 0x31, BNE = 0x32, BGT = 0x33,
          PUSH = 0x81E00FFC, POP = 0x930E0004,
          XCHG = 0x40, ADD = 0x50, SUB = 0x51, MUL = 0x52, DIV = 0x53,
          NOT = 0x60, AND = 0x61, OR = 0x62, XOR = 0x63,
          SHL = 0x70, SHR = 0x71,
          LD = 0x91, ST = 0x80, CSRRD = 0x90, CSRWR = 0x94
};

// oc  m   a   b   c   d  d  d
// 28  24  20  16  12  8  4  0

class Instruction{
protected:
  uint8_t oc, mod, regA, regB, regC;
  uint16_t immed;

public:
  Instruction(uint32_t code) {
    oc = code >> 28;
    mod = (code >> 24) &  0xF;
    regA = (code >> 20) & 0xF;
    regB = (code >> 16) & 0xF;
    regC = (code >> 12) & 0xF;
    immed = code & 0xFFF;
  }
  Instruction(uint8_t oc, uint8_t mod, uint8_t regA, uint8_t regB, uint8_t regC, uint16_t immed) 
    : oc(oc & 0xF), mod(mod & 0xF), regA(regA & 0xF), regB(regB & 0xF), regC(regC & 0xF), immed(immed & 0xFFF) {}

  Instruction(uint8_t oc_mod, uint8_t regA, uint8_t regB, uint8_t regC, uint16_t immed) 
    : oc(oc_mod >> 4), mod(oc_mod & 0xF), regA(regA & 0xF), regB(regB & 0xF), regC(regC & 0xF), immed(immed & 0xFFF) {}

  virtual Instruction* clone() = 0;
  
  uint32_t getCode(){
    return oc << 28 ^ mod << 24 ^ regA << 20 ^ regB << 16 ^ regC << 12 ^ (immed & 0xFFF);
  }
  uint8_t getOC(){
    return oc;
  }

  void setImmed(int immed){
    this->immed = immed;
  }

  virtual void setReg(uint8_t reg) {}

  virtual void setFollowUp() {}

  virtual void setPcRel(bool isPcRel){}
  virtual void setIndirect(bool isIndirect){}

  virtual void process(){
    SectionTable::getInstance().writeCurrentSection(getCode(), 4);
  };
};

class NoArgInstruction : public Instruction{
public:
  NoArgInstruction(uint32_t code) : Instruction(code) {}
  void process() override {
    if(getCode() == IRET){
      //status <= mem[sp + 4]
      SectionTable::getInstance().writeCurrentSection(0x960E0004, 4);
      //pc <= mem[sp], sp <= sp + 8
      SectionTable::getInstance().writeCurrentSection(0x93FE0008 , 4);
    }
    else{
      Instruction::process();
    }
  }
  Instruction* clone(){return new NoArgInstruction(*this); }
};

class AriLogInstruction : public Instruction{
public:
  AriLogInstruction(uint8_t oc_m, uint8_t gprS, uint8_t gprD) : Instruction(oc_m, gprD, gprD, gprS, 0){
    if(oc_m == XCHG){
      this->regA = 0;
    }
  }
  Instruction* clone(){return new AriLogInstruction(*this); }
};

class StackInstruction : public Instruction{
public:
  StackInstruction(uint32_t code, uint8_t gpr) : Instruction(code) {
    if(code == PUSH) this->regC = gpr & 0xF;
    if(code == POP) this->regA = gpr & 0xF;
  }
  Instruction* clone(){return new StackInstruction(*this); }
};

class CSRInstruction : public Instruction {
public:
  CSRInstruction(uint8_t oc_m, uint8_t reg1, uint8_t reg2) : Instruction(oc_m, reg2, reg1, 0, 0){}
  Instruction* clone() {return new CSRInstruction(*this); }
};

class JumpInstruction : public Instruction {

public:
  JumpInstruction(int oc_m, OperandJump* operand) : Instruction(oc_m, 0, 0, 0, 0) {
    operand->process(this);
  }
  JumpInstruction* clone(){return new JumpInstruction(*this); }

  void setPcRel(bool isPcRel) {
    if(isPcRel) this->regA = 0xF;
    else this->regA = 0x0;   
  }

  void setIndirect(bool isIndirect) {
    if(isIndirect){
      uint8_t oc_m = (oc << 4) ^ (mod & 0xF);
      if(oc_m == CALL) this->mod = 0x1;
      else if(oc_m == JMP) this->mod = 0x8;
    }
    else{
      mod = 0x0;
    }
  }
};

class BranchInstruction : public Instruction {
public:
  BranchInstruction(int oc_m, int reg1, int reg2, OperandJump* operand) : Instruction(oc_m, 0, reg1, reg2, 0) {
    operand->process(this);
  }
  BranchInstruction* clone(){return new BranchInstruction(*this); }

  void setPcRel(bool isPcRel){
    if(isPcRel) this->regA = 0xF;
    else this->regA = 0x0;
  }
  void setIndirect(bool isIndirect){
    if(isIndirect) this->mod ^= 0x8;
  }
};

class LoadInstruction : public Instruction {
private:
  Instruction* loadReg;
public: 
  LoadInstruction(int oc_m, int reg, Operand* operand) : Instruction(oc_m, reg, 0, 0, 0) {
    operand->process(this);
  }
  LoadInstruction(int oc_m, int regA, int regB) : Instruction(oc_m, regA, regA, 0, 0) {}

  LoadInstruction* clone() override {
    return new LoadInstruction(*this);
  }

  void setReg(uint8_t reg) override {
    regB = reg;
  }

  void setPcRel(bool isPcRel){
    if(isPcRel) this->regB = 0xF;
    else this->regB = 0x0;
  }
  void setIndirect(bool isIndirect){
    if(isIndirect) this->mod = 0x2;
    else this->mod = 0x1;
  }
  void setFollowUp() override{
    loadReg = new LoadInstruction(0x92, regA, regA);
  }

  void process() override {
    Instruction::process();
    if(loadReg != nullptr){
      loadReg->process();
      delete loadReg;
      loadReg = nullptr;
    }
  }
};

class StoreInstruction : public Instruction {
public:
  StoreInstruction(int oc_m, int reg, Operand* operand) : Instruction(oc_m, 0, 0, reg, 0) {
    operand->process(this);
  }
  StoreInstruction* clone(){return new StoreInstruction(*this); }

  void setReg(uint8_t reg) override {
    regA = reg;
  }
  void setPcRel(bool isPcRel){
    if(isPcRel) this->regA = 0xF;
    else this->regA = 0x0;
  }
  void setIndirect(bool isIndirect){
    if(isIndirect) this->mod = 0x2;
    else this->mod = 0x0;
  }
};

#endif //INSTRUCTION_HPP
