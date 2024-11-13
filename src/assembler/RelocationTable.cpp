#include "../../inc/assembler/RelocationTable.hpp"
#include <fstream>

void RelocationTable::createRelocation(int symbol){
  SectionTable& secTable = SectionTable::getInstance();
  relocations.push_back(
    new Relocation(
      secTable.getCurrentSectionId(),
      secTable.getCurrentOffset(),
      symbol
    )
  );
}

void RelocationTable::createRelocation(int symbol, int section, int offset){
  relocations.push_back(
    new Relocation(section, offset, symbol));
}

void RelocationTable::createRelocation(string symbol){
  int symbolId = SymbolTable::getInstance().getSymbolId(symbol);
  createRelocation(symbolId);
}

void RelocationTable::createRelocation(string symbol, int section, int offset){
  int symbolId = SymbolTable::getInstance().getSymbolId(symbol);
  createRelocation(symbolId, section, offset);
}


void RelocationTable::backpatch(){
  SectionTable& secTable = SectionTable::getInstance();
  SymbolTable& symTable = SymbolTable::getInstance();

  for(auto rel: relocations){
    if(symTable.isLocal(rel->symbol)){
      rel->addent = symTable.getSymbolValue(rel->symbol);
      rel->symbol = secTable.getSectionSymTableEntry(symTable.getSymbolSection(rel->symbol));
    }
  }
}


void RelocationTable::print(ofstream& ofs){
    SectionTable& secTable = SectionTable::getInstance();
    SymbolTable& symTable = SymbolTable::getInstance();

    vector<vector<Relocation*>> rels(secTable.getCurrentSectionId() + 1);
    for(Relocation* rel: relocations){
      rels[rel->section].push_back(rel);
    }

    for(int id = 1; id < rels.size(); id++){
      if(rels[id].size() > 0){
        ofs << "\n\nRelocation Table" << endl;
        ofs << setfill(' ');
        ofs << secTable.getSectionName(id) << ".rel" << endl;
        ofs << left
            << setw(8) << "Offset" 
            << setw(16) << "Symbol" 
            << setw(8) << "Addent" 
            << endl;
        ofs << "--------------------------------" << endl;
        for(Relocation* rel: rels[id]){
          ofs << left 
              << setw(8) << hex << rel->offset << dec
              << setw(16) << symTable.getSymbolName(rel->symbol)
              << setw(8) << hex << rel->addent << dec
              << endl;
        }
      }
    }
  }

vector<Relocation_> RelocationTable::exportRelocations(){
  vector<Relocation_> output;
  SymbolTable& symT = SymbolTable::getInstance();
  vector<Symbol_> symbols = symT.exportSymbols();

  for(Relocation* rel: relocations){
    string symbolName = symT.getSymbolName(rel->symbol);
    int symbolId;
    for(int i = 0; i < symbols.size(); i++){
      if(symbols[i].name == symbolName){
        symbolId = i;
        break;
      }
    }
    output.push_back(Relocation_(rel->section, rel->offset, symbolId, rel->addent));
  }

  return output;
}

