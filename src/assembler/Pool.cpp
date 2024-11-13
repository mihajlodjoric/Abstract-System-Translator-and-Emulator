#include "../../inc/assembler/Pool.hpp"
#include "../../inc/assembler/SymbolTable.hpp"
#include "../../inc/assembler/SectionTable.hpp"

void Pool::insertLiteral(uint32_t literal){
   if(findLiteral(literal) < 0){
      data.push_back(literal);
    }
}

void Pool::insertSymbol(string symbol){
  if(symbolLiterals.count(symbol) == 0){
    data.push_back(0);
    symbolLiterals[symbol] = data.size() - 1;

    int secId = SectionTable::getInstance().getSectionId(sectionName);
    uint32_t relOffset = SectionTable::getInstance().getOffsetToPoolSymbol(secId, -4, symbol);          
    RelocationTable::getInstance().createRelocation(symbol, secId, relOffset);
  }
}

void Pool::print(){
    for(int i = 0; i < data.size(); i++){
      unsigned char* bytes = reinterpret_cast<unsigned char*>(&data[i]);
      for(int j = 0; j < 4; j++){
        cout << hex << setw(2) << setfill('0') << (int)bytes[j] << dec;
        if(j % 4 == 3) cout << ' ';
        if(j % 16 == 15) cout << endl;
      }
    }
    if(data.size() > 0) cout << endl;  
}

void Pool::addToSection(){
  //add instruction to jump over the pool
  uint32_t skipPoolIns = 0x30F00000 | (4*data.size() - 4);
  this->data[0] = skipPoolIns;
  //add pool to the end of the section
  if(data.size() > 1){
    SectionTable& secT = SectionTable::getInstance();
    int secId = secT.getSectionId(sectionName);
    for(int i = 0; i < data.size(); i++){
      secT.writeSection(secId, data[i], 4);
  }
  }
}