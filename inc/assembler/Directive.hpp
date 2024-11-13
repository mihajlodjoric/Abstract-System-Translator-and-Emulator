#include <iostream>
#include <vector>
using namespace std;

#include <variant>
#include "SectionTable.hpp"
#include "SymbolTable.hpp"
#include "RelocationTable.hpp"

class Directive{
protected:
  SectionTable& sectionTable = SectionTable::getInstance();
  SymbolTable& symbolTable = SymbolTable::getInstance();
  RelocationTable& relocationTable = RelocationTable::getInstance();
public:
  virtual void process(){}
};

class DirectiveGlobal: public Directive{
private:
  vector<string>* symbols;
public:
  DirectiveGlobal(vector<string>* symbols){
    this->symbols = symbols;
  }
  void process() override;
};

class DirectiveExtern: public Directive{
private:
  vector<string>* symbols;
public:
  DirectiveExtern(vector<string>* symbols){
    this->symbols = symbols;
  }
  void process() override;
};

class DirectiveSection: public Directive{
private:
  string name;
public:
  DirectiveSection(string s){
    this->name = s;
  }
  void process() override;
};

class DirectiveWord: public Directive{
private:
  vector<variant<int, string>> *arguments;
public:
  DirectiveWord(vector<variant<int, string>>* symbols){
    this->arguments = symbols;
  }
  void process() override;
};

class DirectiveSkip: public Directive{
private:
  int size;
public:
  DirectiveSkip(int size){
    this->size = size;
  }
  void process() override;
};

class DirectiveAscii: public Directive{
private:
  string characters;
public:
  DirectiveAscii(string characters){
    int n = characters.length();
    this->characters = characters.substr(1, n-2);
  }
  void process() override;
};

class DirectiveEqu: public Directive{
private:
  int value;
  string symbol;
public:
  DirectiveEqu(string symbol, int value){
    this->symbol = symbol;
    this->value = value;
  }  
  void process() override;
};

class DirectiveEnd: public Directive{
public:
  DirectiveEnd(){
  }
  void process() override;
};