#include "../../inc/assembler/Operand.hpp"
#include "../../inc/assembler/Instruction.hpp"
#include "../../inc/assembler/SymbolTable.hpp"

SymbolTable& symT = SymbolTable::getInstance();
SectionTable& secT = SectionTable::getInstance();


void LiteralDirect::process(Instruction* ins){
  if(literal >= -2047 && literal <= 2048){
    ins->setImmed(literal);
  }
  else{
    SectionTable::getInstance().insertCurrentSectionPool(literal);
    ins->setPcRel(true); ins->setIndirect(true);
  }
}

void LiteralIndirect::process(Instruction* ins){

  if(literal >= -2047 && literal <= 2048){
    ins->setImmed(literal);
    ins->setIndirect(true);
  }
  else{
    SectionTable::getInstance().insertCurrentSectionPool(literal);
    ins->setPcRel(true); ins->setIndirect(true);
    ins->setFollowUp(); 
  }
}

void SymbolDirect::process(Instruction* ins){
  if(symT.isAbsolute(symbol)){
    LiteralDirect* temp = new LiteralDirect(symT.getSymbolValue(symbol));
    temp->process(ins); delete temp;
  }
  else if(symT.isDefined(symbol) && symT.getSymbolSection(symbol) == secT.getCurrentSectionId()){
    int immed = symT.getSymbolValue(symbol) - secT.getCurrentOffset() - 4;
    ins->setPcRel(true); ins->setIndirect(false);
    ins->setImmed(immed);
  }
  else {
    if(symT.isDeclared(symbol) == false){
      symT.insertUndefinedSymbol(symbol);
    }
    symT.insertForwardRef(symbol, ins->clone());
  }
}

void SymbolIndirect::process(Instruction* ins){
  if(symT.isAbsolute(symbol)){
    LiteralIndirect* temp = new LiteralIndirect(symT.getSymbolValue(symbol));
    temp->process(ins); delete temp;
  }
  else if(symT.isDefined(symbol) && symT.getSymbolSection(symbol) == secT.getCurrentSectionId()){
    int immed = symT.getSymbolValue(symbol) - secT.getCurrentOffset() - 4;
    ins->setPcRel(true); ins->setIndirect(true);
    ins->setImmed(immed);
  }
  else{
    if(symT.isDeclared(symbol) == false){
      symT.insertUndefinedSymbol(symbol);
    }
    ins->setFollowUp();
    symT.insertForwardRef(symbol, ins->clone());
  }
}

void RegisterDirect::process(Instruction* ins){
  ins->setReg(reg);
}

void RegisterIndirect::process(Instruction* ins){
  ins->setReg(reg);
  ins->setIndirect(true);
}

void RegisterLiteral::process(Instruction* ins){
  if(literal >= -2047 && literal <= 2048){
    ins->setReg(reg);
    if(ins->getOC() == 0x9) ins->setIndirect(true);
    ins->setImmed(literal);
  }
  else{
    cout << "Value " << literal << " is to big for register-indirect adressing type" << endl;
    stopAssembling();
  }
}

void RegisterSymbol::process(Instruction* ins){
  //ako je apsolutni simbol onda treba jos nesto
  if(symT.isDefined(symbol) && symT.getSymbolValue(symbol) <= 0xFFF){
    ins->setReg(reg);
    if(ins->getOC() == 0x9) ins->setIndirect(true);
    ins->setImmed(symT.getSymbolValue(symbol));
  }
  else{
    cout << "Value of symbol " << symbol << " can't be used for register-indirect adressing type" << endl;
    stopAssembling();
  }
}