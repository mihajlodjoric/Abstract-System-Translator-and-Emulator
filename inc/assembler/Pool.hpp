#ifndef POOL_HPP
#define POOL_HPP

#include <iostream>
#include <vector>
#include <map>
using namespace std;
#include <iomanip>

struct PoolPatch{
  int offset;
  int index;
  PoolPatch(int offset, int index) : offset(offset), index(index){}
};

class Pool {
private:
  string sectionName;
  vector<uint32_t> data;
  map<string, int> symbolLiterals;

  vector<PoolPatch> patches;
public:
  Pool(string sectionName){
    this->sectionName = sectionName;
    this->data.push_back(0);
  }

  int size(){
    return data.size() * 4;
  }

  bool isEmpty() const {
    return data.size() == 0;
  }

  int findLiteral(uint32_t literal) const{
    for(int i = 0; i < data.size(); i++){
      if(data[i] == literal) return i;
    }
    return - 1;
  }

  int findSymbol(string symbol){
    if(symbolLiterals.count(symbol) == 0) return -1;
    else return symbolLiterals[symbol];
  }

  int getOffset(string symbol){
    int i = findSymbol(symbol);
    if(i < 0) return 0;
    return 4 * i;
  }
  int getOffset(uint32_t literal){
    int i = findLiteral(literal);
    if(i < 0) return 0;
    return 4 * i;
  }

  void insertLiteral(uint32_t literal);

  void insertSymbol(string symbol);

  void insertPatch(int offset, int literal){
    int index = findLiteral(literal);
    patches.push_back(PoolPatch(offset, index));
  }

  void insertPatch(int offset, string symbol){
    int index = findSymbol(symbol);
    patches.push_back(PoolPatch(offset, index));
  }

  const vector<PoolPatch>& getPatches(){
    return patches;
  }

  void setData(int ind, uint32_t value){
    data[ind] = value;
  }

  void print();

  void addToSection();
};

#endif //POOL_HPP