#ifndef SYMBOLTABLE_HPP
#define SYMBOLTABLE_HPP

#include "Assembler.hpp"
#include "RelocationTable.hpp"
#include "SectionTable.hpp"
#include "Instruction.hpp"

#include <iostream>
#include <vector>
using namespace std;


class SymbolTable{
private:
  struct ForwardRef{
  int section;
  int offset;
  Instruction* instruction;
  
  ForwardRef(int section, int offset, Instruction* instruction = nullptr) : section(section), offset(offset), instruction(instruction){}
  };

  enum binding {LOCAL, GLOBAL, EXTERN};

  struct Symbol{
    string name;
    int value;
    int section;
    enum binding binding;
    bool isAbsolute = false;
    vector<ForwardRef> forwardRefs;
    Symbol(string name, int value, int section, enum binding binding = LOCAL, bool isAbsolute = false) :
                      name(name), value(value), section(section), binding(binding), isAbsolute(isAbsolute){}
    bool isDefined(){
      return section > 0;
    }
  };
  vector<Symbol*> symbols;

  int solveForwardRefs(Symbol* symobol);

  SymbolTable() = default;
public:
  SymbolTable(const SymbolTable&) = delete;
  SymbolTable& operator=(const SymbolTable&) = delete;

  static SymbolTable& getInstance(){
    static SymbolTable instance;
    return instance;
  }

  int getSymbolId(string name) const;

  bool isDeclared(int symbolId) const {
    if(symbolId < 0 || symbolId > symbols.size()) return false;
    return true;
  }
  bool isDeclared(string name) const {
    int symbolId = getSymbolId(name);
    if(!isDeclared(symbolId)) return false;
    return true; 
  }

  string getSymbolName(int symbolId) const {
    if(!isDeclared(symbolId)) return "";
    return symbols[symbolId]->name;
  }


  int getSymbolValue(int symbolId) const {
    if(!isDeclared(symbolId)) return 0;
    return symbols[symbolId]->value;
  } 
  int getSymbolValue(string name) const {
    int symbolId = getSymbolId(name);
    return getSymbolValue(symbolId);
  } 

  int getSymbolSection(int symbolId) const {
    if(!isDeclared(symbolId)) return 0;
    return symbols[symbolId]->section;
  }
  int getSymbolSectionSymbol(int symbolId) const {
    if(!isDeclared(symbolId)) return 0;
    string sectionName = SectionTable::getInstance().getSectionName(symbols[symbolId]->section);
    int id = getSymbolId(sectionName);
    return id; // skull emoji
  }
  int getSymbolSection(string name) const {
    int symbolId = getSymbolId(name);
    return getSymbolSection(symbolId);
  }

  bool isGlobal(int symbolId) const {
    if(!isDeclared(symbolId)) return false;
    return symbols[symbolId]->binding == GLOBAL;
  }
  bool isGlobal(string name) const {
    int symbolId = getSymbolId(name);
    return isGlobal(symbolId);
  }

  bool isLocal(int symbolId) const {
    if(!isDeclared(symbolId)) return false;
    return symbols[symbolId]->binding == LOCAL;
  }
  bool isLocal(string name) const {
    int symbolId = getSymbolId(name);
    return isLocal(symbolId);
  }

  bool isExtern(int symbolId) const {
    if(!isDeclared(symbolId)) return false;
    return symbols[symbolId]->binding == EXTERN;
  }
  bool isExtern(string name) const {
    int symbolId = getSymbolId(name);
    return isExtern(symbolId);
  }

  bool isDefined(int symbolId) const {
    if(!isDeclared(symbolId)) return false;
    return symbols[symbolId]->section > 0;
  }
  bool isDefined(string name) const {
    int symbolId = getSymbolId(name);
    return isDefined(symbolId);
  }

  bool isAbsolute(int symbolId) const {
    if(!isDeclared(symbolId)) return false;
    return symbols[symbolId]->isAbsolute;
  }
  bool isAbsolute(string name) const {
    int symbolId = getSymbolId(name);
    return isAbsolute(symbolId);
  }



  void insertSymbol(string name);
  int insertSymbol(string name, int section);
  void insertAbsoluteSymbol(string name, int value);
  void insertUndefinedSymbol(string name);

  void setGlobal(string name);
  void setExtern(string name);


  void insertForwardRef(string symbol, Instruction* ins);
  void insertForwardRef(string symbol);

  int backpatch();

  void print();
  void print(ofstream&);
  vector<Symbol_> exportSymbols();
};
#endif //SYMBOLTABLE_HPP