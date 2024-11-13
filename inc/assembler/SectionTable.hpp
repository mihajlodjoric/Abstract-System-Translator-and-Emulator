#ifndef SECTIONTABLE_HPP
#define SECTIONTABLE_HPP

#include "Assembler.hpp"
#include "Pool.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
using namespace std;


class SectionTable{
private:

  struct Section{
    string name;
    vector<uint8_t> data;
    Pool pool;
    int symbolTableEntry;

    Section(string name) : name(name), pool(name){}
    void writeByte(uint8_t byte) {data.push_back(byte); }
    int size() {return data.size(); }

  };

  vector<Section*> sections;

  SectionTable(){
    sections.push_back(new Section(""));
  }

public:
  SectionTable(const SectionTable&) = delete;
  SectionTable& operator=(const SectionTable&) = delete;

  static SectionTable& getInstance() {
    static SectionTable instance;
    return instance;
  }


  void createSection(string name);
  
  int getCurrentSectionId() const {
    return sections.size() - 1;
  }

  int getSectionId(string name);

  string getSectionName(int id){
    return sections[id]->name;
  }

  int getSectionSize(int id){
    return sections[id]->size();
  }

  size_t getCurrentOffset() const {
    return sections[getCurrentSectionId()]->size();
  }

  int getSectionSymTableEntry(int id) const {
    return sections[id]->symbolTableEntry;
  }

  void writeCurrentSection(uint64_t value, size_t size);

  void writeSection(int section, int offset, uint64_t value, size_t size);

  void writeSection(int section, uint64_t value, size_t size);

  void insertCurrentSectionPool(uint32_t literal);

  void insertPool(int section, string symbol);

  uint32_t getOffsetToPoolSymbol(int section, int myOffset, string symbol);

  void backpatch();




  void print();

  void print(ofstream& ofs);

  vector<Section_> exprotSections();
};


#endif //SECTIONTABLE_HPP