#ifndef FILE_HPP
#define FILE_HPP


#include "Linker.hpp"
#include <iomanip>

void readFromFile(const std::string& filename, std::vector<Section_>& sections, std::vector<Symbol_>& symbols, std::vector<Relocation_>& relocations);


class File{
private:
  string name;
  vector<Section_> sections;
  vector<Symbol_> symbolTable;
  vector<Relocation_> relocations;

public:
  File(string name) : name(name){
    try
    {
      readFromFile(name, sections, symbolTable, relocations);

      for(Symbol_& sym: symbolTable){
        sym.sectionName = sections[sym.section].name;
      }
      for(Section_& sec: sections){
        for(Symbol_& sym: symbolTable){
          if(sym.name == sec.name){
            sym.isSection = true;
            break;
          }
        }
      }

      for(Relocation_& rel: relocations){
        rel.sectionName = sections[rel.section].name;
        rel.symbolName = symbolTable[rel.symbol].name;
      }
    }
    catch(const std::ios_base::failure& e)
    {
      throw e;
    }
  }

  string getName(){
    return name;
  }

  vector<Section_>& getSections(){
    return sections;
  }

  vector<Symbol_> getGlobalSymbols(){
    vector<Symbol_> temp;
    for(Symbol_ symbol: symbolTable){
      if(symbol.section != 0 && symbol.isSection == false){
        temp.push_back(symbol);
      }
    }
    return temp;
  }

  vector<Symbol_>& getSymbols(){
    return symbolTable;
  }

  vector<Relocation_>& getRelocations(){
    return relocations;
  }

  void updateSymbols();
  void updateRelocations();

  void solveInternalRelocations();

  void solveExternalRelocations(const map<string, uint32_t>& symbolValues);

  void solveRelocations(map<string, uint32_t>& symbolValues);




  void printSections(){
    cout << name << endl;
    for(int i = 1; i < sections.size(); i++){
      Section_& sec = sections[i];
      cout << i << ": " << sec.name << ' ' << hex << sec.offset << ' ' << sec.size() << dec << endl;
    
    }
  }

  void printSymbols(){
    for(Symbol_& sym: symbolTable){
      cout << sym.name << ' ' << hex << sym.value << dec << endl;
    }
  }

  void printData(){
    for(int i = 1; i < sections.size(); i++){
      Section_& sec = sections[i];
      cout << sec.name << endl;
      for(int j = 0; j < sec.size(); j++){
        cout << hex << setw(2) << setfill('0') << (int)sec.data[j] << dec;
        if(j % 4 == 3) cout << ' ';
        if(j % 16 == 15 || j == sec.size() - 1) cout << endl;
    }
    cout << endl;
    }
  }

  void printRelocations(){
    for(Relocation_& rel: relocations){
      cout << rel.section << ':' << hex << rel.offset << dec << ' ' << rel.symbol << endl;
    }
  }
};


#endif //FILE_HPP