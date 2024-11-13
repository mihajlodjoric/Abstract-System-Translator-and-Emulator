#include "../../inc/assembler/SymbolTable.hpp"
#include <fstream>

int SymbolTable::getSymbolId(string name) const{
  for(int i = 0; i < symbols.size(); i++){
    if(symbols[i]->name == name) return i;
  }
  return -1;
}

void SymbolTable::insertSymbol(string name){
  SectionTable& sectionTable = SectionTable::getInstance();

  int ind = getSymbolId(name);

  if(ind  == -1){
    symbols.push_back(new Symbol(name, sectionTable.getCurrentOffset(), sectionTable.getCurrentSectionId()));
  }
  else{
    if(getSymbolSection(ind) == 0 && !isExtern(ind)){
      symbols[ind]->value = sectionTable.getCurrentOffset();
      symbols[ind]->section = sectionTable.getCurrentSectionId();
    }
    else if(isExtern(ind)){
      cout << "Error: Extern symbol " << name << " definition found" << endl;
      stopAssembling();
    }
    else{
      cout << "Error: Multiple definitions of symbol " << name << endl;
      stopAssembling();
    }
  }
}

int SymbolTable::insertSymbol(string name, int section){
  SectionTable& sectionTable = SectionTable::getInstance();

  int ind = getSymbolId(name);

  if(ind  == -1){
    symbols.push_back(new Symbol(name, 0, section, GLOBAL));
    return symbols.size() - 1;
  }
  else{
    if(getSymbolSection(ind) == 0 && !isExtern(ind)){
      symbols[ind]->value = 0;
      symbols[ind]->section = section;
    }
    else if(isExtern(ind)){
      cout << "Error: Extern symbol " << name << " definition found" << endl;
      stopAssembling();
    }
    else{
      cout << "Error: Multiple definitions of symbol " << name << endl;
      stopAssembling();
    }
    return ind;
  }
}

void SymbolTable::insertUndefinedSymbol(string name){
  SectionTable& sectionTable = SectionTable::getInstance();
  int ind = getSymbolId(name);
  if(ind  == -1){
    symbols.push_back(new Symbol(name, 0, 0));
  }
}

//ovo je samo za tetiranje
void SymbolTable::insertAbsoluteSymbol(string name, int value){
  SectionTable& sectionTable = SectionTable::getInstance();

  int ind = getSymbolId(name);

  if(ind  == -1){
    symbols.push_back(new Symbol(name, value, sectionTable.getCurrentSectionId(), LOCAL, true));
  }
  else{
    if(getSymbolSection(ind) == 0){
      symbols[ind]->value = value;
      symbols[ind]->section = sectionTable.getCurrentSectionId();
      symbols[ind]->isAbsolute = true;
    }
    else{
      cout << "Error: Multiple definitions of symbol " << name << endl;
      stopAssembling();
    }
  }
}

void SymbolTable::setGlobal(string name){
  int ind = getSymbolId(name);
  if(ind != -1){
    symbols[ind]->binding = GLOBAL;
  }
  else{
    symbols.push_back(new Symbol(name, 0, 0, GLOBAL));
  }
}

void SymbolTable::setExtern(string name){
  int ind = getSymbolId(name);
  if(ind != -1){
    symbols[ind]->binding = EXTERN;
  }
  else{
    symbols.push_back(new Symbol(name, 0, 0, EXTERN));
  }
}

void SymbolTable::insertForwardRef(string symbol){
  SectionTable& sectionTable = SectionTable::getInstance();
  int section = sectionTable.getCurrentSectionId();
  int offset = sectionTable.getCurrentOffset();
  symbols[getSymbolId(symbol)]->forwardRefs.push_back(ForwardRef(section, offset));
}

void SymbolTable::insertForwardRef(string symbol, Instruction* ins){
  SectionTable& sectionTable = SectionTable::getInstance();
  int section = sectionTable.getCurrentSectionId();
  int offset = sectionTable.getCurrentOffset();
  symbols[getSymbolId(symbol)]->forwardRefs.push_back(ForwardRef(section, offset, ins));
}

int SymbolTable::backpatch(){
  for(int i = 0; i < symbols.size(); i++){
    Symbol* symbol = symbols[i];

    if(symbol->binding == GLOBAL && symbol->section == 0){
      symbol->binding = EXTERN;
    }
    if(symbol->forwardRefs.empty()) continue;

    if(solveForwardRefs(symbol) < 0) return -1;

  }
  return 0;
}


int SymbolTable::solveForwardRefs(Symbol* symbol){
  SectionTable& sectionTable = SectionTable::getInstance();
  RelocationTable& relocationTable = RelocationTable::getInstance();
  
  if(symbol->binding == EXTERN){
    for(ForwardRef ref: symbol->forwardRefs){
      if(ref.instruction == nullptr){
        relocationTable.createRelocation(getSymbolId(symbol->name), ref.section, ref.offset);
      }
      else{
        sectionTable.insertPool(ref.section, symbol->name);  
        uint32_t insOffset = sectionTable.getOffsetToPoolSymbol(ref.section, ref.offset, symbol->name);
        if(insOffset <= 0xFFF){
          ref.instruction->setPcRel(true); ref.instruction->setIndirect(true);
          ref.instruction->setImmed(insOffset);
          sectionTable.writeSection(ref.section, ref.offset, ref.instruction->getCode(), 4);
        }
        else return -1;
      }
    }
  }
  else if(symbol->isDefined() && !symbol->isAbsolute){
    for(ForwardRef ref: symbol->forwardRefs){
      if(ref.instruction == nullptr){
        relocationTable.createRelocation(getSymbolId(symbol->name), ref.section, ref.offset);
      }
      else{
        if(ref.section == symbol->section){
          ref.instruction->setPcRel(true); ref.instruction->setIndirect(false);
          ref.instruction->setImmed(symbol->value - ref.offset - 4);
          sectionTable.writeSection(ref.section, ref.offset, ref.instruction->getCode(), 4);
        }
        else{
          sectionTable.insertPool(ref.section, symbol->name);  
          uint32_t insOffset = sectionTable.getOffsetToPoolSymbol(ref.section, ref.offset, symbol->name);
          if(insOffset <= 0xFFF){
            ref.instruction->setPcRel(true); ref.instruction->setIndirect(true);
            ref.instruction->setImmed(insOffset);
            sectionTable.writeSection(ref.section, ref.offset, ref.instruction->getCode(), 4);
          }
          else return -1;
        }
      }
    }
  }
  else if(symbol->isAbsolute){
    for(ForwardRef ref: symbol->forwardRefs){
      if(ref.instruction == nullptr){
        sectionTable.writeSection(ref.section, ref.offset, symbol->value, 4);
      }
      else{
        if((uint32_t)symbol->value <= 0xFFF){
          sectionTable.writeSection(ref.section, ref.offset, symbol->value, 2);
        }
        else{
          sectionTable.insertPool(ref.section, symbol->name);  
          uint32_t insOffset = sectionTable.getOffsetToPoolSymbol(ref.section, ref.offset, symbol->name);
          if(insOffset <= 0xFFF){
            ref.instruction->setPcRel(true);
            ref.instruction->setIndirect(true);
            ref.instruction->setImmed(insOffset);
            sectionTable.writeSection(ref.section, ref.offset, ref.instruction->getCode(), 4);
          }
          else return -1;
        }
      }
    }
  }
  else{
    cout << "Error: Symbol " << symbol->name << " is neither defined nor extern" << endl;
    return -1;
  }
return 0;
}

void SymbolTable::print(){
  string bindings[3] = {"LOC", "GLOB", "EXT"};
  cout << "Name\t" << "Section\t" << "Value\t" << "Binding\t" << "Absolute" << endl;
  for(int i = 0; i < symbols.size(); i++){
    Symbol* symbol = symbols[i];
    cout << symbol->name << '\t' << symbol->section << '\t' 
    << symbol->value << '\t' << bindings[symbol->binding] << '\t'
    << (symbol->isAbsolute ? "yes" : "no") << endl;
  }
}

void SymbolTable::print(ofstream& ofs){
  SectionTable& secT = SectionTable::getInstance();
  string bindings[3] = {"LOC", "GLOB", "EXT"};
  const int width = 16;



  ofs << "Symbol Table" << endl;
  ofs << setfill(' ');
  ofs << left
      << setw(8) << "ID"
      << setw(16) << "Name" 
      << setw(16) << "Section" 
      << setw(16) << "Value" 
      //<< setw(12) << "Binding" 
      //<< setw(12) << "Absolute" 
      << endl;
  ofs << "-----------------------------------------------------------------------------" << endl;
  for(int i = 0; i < symbols.size(); i++){
    Symbol* symbol = symbols[i];
    if(true){
      ofs << left
      << setw(8) << i
      << setw(16) << symbol->name 
      << setw(16) << (symbol->section ? secT.getSectionName(symbol->section) : "-")
      << setw(16) << hex << symbol->value << dec 
      //<< setw(12) << bindings[symbol->binding]
      //<< setw(12) << (symbol->isAbsolute ? "yes" : "no") 
      << endl;
    }
  }
}

vector<Symbol_> SymbolTable::exportSymbols(){
  vector<Symbol_> output;

  for(Symbol* sym: symbols){
    if(sym->binding != LOCAL)
      output.push_back(Symbol_(sym->name, sym->value, sym->section));
  }

  return output;
}