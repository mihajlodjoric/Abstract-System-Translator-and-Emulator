#include "../../inc/assembler/SectionTable.hpp"
#include "../../inc/assembler/SymbolTable.hpp"
#include <fstream>

int SectionTable::getSectionId(string name){
  for(int i = 1; i < sections.size(); i++){
    if(sections[i]->name == name) return i;
  }
  return 0;
}

void SectionTable::createSection(string name){
  if(getSectionId(name) == 0){
    Section* newSection = new Section(name);
    newSection->symbolTableEntry = SymbolTable::getInstance().insertSymbol(name, sections.size());
    sections.push_back(newSection);
  }
  else{
    cout << "Error: section " << name << " already defined" << endl;
    stopAssembling();
  }
}

void SectionTable::writeCurrentSection(uint64_t value, size_t size){
  for(size_t i = 0; i < size; i++){
    sections[getCurrentSectionId()]->writeByte(static_cast<uint8_t>(value >> (i * 8)));
  }
}

void SectionTable::writeSection(int section, uint64_t value, size_t size){
  for(size_t i = 0; i < size; i++){
    sections[section]->writeByte(static_cast<uint8_t>(value >> (i * 8)));
  }
}

void SectionTable::writeSection(int section, int offset, uint64_t value, size_t size){
  for(size_t i = 0; i < size; i++){
    sections[section]->data[offset + i] = static_cast<uint8_t>(value >> (i * 8));
  }
}

uint32_t SectionTable::getOffsetToPoolSymbol(int section, int myOffset, string symbol){
  return getSectionSize(section) + sections[section]->pool.getOffset(symbol) - myOffset - 4;
}

void SectionTable::insertCurrentSectionPool(uint32_t literal){
  Pool& myPool = sections[getCurrentSectionId()]->pool;
  
  myPool.insertLiteral(literal);
  myPool.insertPatch(getCurrentOffset(), literal);
}


void SectionTable::insertPool(int secId, string symbol){
  sections[secId]->pool.insertSymbol(symbol);
}

void SectionTable::backpatch(){
  for(int id = 0; id < sections.size(); id++){
    if(!sections[id]->pool.isEmpty()){
      for(PoolPatch patch: sections[id]->pool.getPatches()){
        int DDD = sections[id]->size() - patch.offset + 4 * patch.index - 4;
        for(size_t i = 0; i < 2; i++){
          sections[id]->data[patch.offset + i] ^= static_cast<uint8_t>(DDD >> (i * 8));
        }
      }
    }

    sections[id]->pool.addToSection();
  }
}

void SectionTable::print(){
  for(int i = 1; i < sections.size(); i++){
    Section* s = sections[i];
    cout << ".section " << s->name << endl;
    vector<uint8_t> data = s->data;
    for(int j = 0; j < s->size(); j++){
      cout << hex << setw(2) << setfill('0') << (int)data[j] << dec;
      if(j % 4 == 3) cout << ' ';
      if(j % 16 == 15 || j == s->size() - 1) cout << endl;
    }
    s->pool.print();
    cout << endl;
  }
}

void SectionTable::print(ofstream& ofs){
  for(int i = 1; i < sections.size(); i++){
    Section* s = sections[i];
    ofs << ".section " + s->name << endl;
    ofs << "-----------------------------------------------------" << endl;
    ofs << hex << 0 << dec << ":\t\t\t";
    vector<uint8_t> data = s->data;
    for(int j = 0; j < s->size(); j++){
      ofs << hex << setw(2) << setfill('0') << (int)data[j] << dec;
      if(j % 4 == 3) ofs << "  ";
      if(j % 16 == 15 || j == s->size() - 1){
        ofs << endl;
        if(j != s->size() - 1) ofs << hex << j+1 << dec << ":\t\t\t";
      } 
    }
    ofs << endl << endl;
  }
}

vector<Section_> SectionTable::exprotSections(){
  vector<Section_> output;
  for(Section* sec: sections){
    output.push_back(Section_(sec->name, sec->data));
  }
  return output;
}
