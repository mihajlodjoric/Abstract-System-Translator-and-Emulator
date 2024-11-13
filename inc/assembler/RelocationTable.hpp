#ifndef RELOCATIONTABLE_HPP
#define RELOCATIONTABLE_HPP

#include <iostream>
#include <vector>
#include "SectionTable.hpp"
#include "SymbolTable.hpp"
using namespace std;

class RelocationTable{
private:
  struct Relocation{
    int section;
    int offset;
    int symbol;
    int addent;
    Relocation(int section, int offset, int symbol, int addent = 0):
       section(section), offset(offset), symbol(symbol), addent(addent){}
  };

  vector<Relocation*> relocations;

  RelocationTable() = default;
public:
  RelocationTable(const RelocationTable&) = delete;
  RelocationTable& operator=(const RelocationTable&) = delete;
  static RelocationTable& getInstance(){
    static RelocationTable instance;
    return instance;
  }

  void createRelocation(int symbol);
  void createRelocation(int symbol, int section, int offset);

  void createRelocation(string symbol);
  void createRelocation(string symbol, int section, int offset);

  void backpatch();

  void print(ofstream&);
  vector<Relocation_> exportRelocations();

};


#endif //RELOCATIONTABLE_HPP