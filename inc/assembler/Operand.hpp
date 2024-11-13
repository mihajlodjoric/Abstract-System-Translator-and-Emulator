#ifndef OPERAND_HPP
#define OPERAND_HPP

#include "SectionTable.hpp"

class Instruction;
class LoadInstruction;
class StoreInstruction;

class Operand {
public:
  virtual void process(Instruction* ins){};
};

class OperandJump : public Operand {
public:
  virtual void process(Instruction* ins){};
};

class LiteralDirect : public OperandJump {
private:
  int literal;
public:
  LiteralDirect(int literal) : literal(literal) {}
  void process(Instruction* ins) override;
};

class LiteralIndirect : public Operand {
private:
  int literal;
public:
  LiteralIndirect(int literal) : literal(literal){}
  void process(Instruction* ins) override;

};

class SymbolDirect : public OperandJump {
private:
  string symbol;
public:
  SymbolDirect(string symbol) : symbol(symbol){}
  void process(Instruction* ins) override;
};

class SymbolIndirect : public Operand {
private:
  string symbol;
public:
  SymbolIndirect(string symbol) : symbol(symbol){}
  void process(Instruction* ins);
};

class RegisterDirect : public Operand {
private: 
  int reg;
public:
  RegisterDirect (int reg) : reg(reg){}
  void process(Instruction* ins);
};

class RegisterIndirect : public Operand {
private: 
  int reg;
public:
  RegisterIndirect(int reg) : reg(reg) {}
  void process(Instruction* ins);
};

class RegisterLiteral : public Operand {
private:
  int reg;
  int literal;
public:
  RegisterLiteral(int reg, int literal) : reg(reg), literal(literal) {}
  void process(Instruction* ins);
};

class RegisterSymbol : public Operand {
private:
  int reg;
  string symbol;
public:
  RegisterSymbol(int reg, string symbol) : reg(reg), symbol(symbol) {}
  void process(Instruction* ins);
};

#endif //OPERAND_HPP