#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <iostream>
#include <vector>
using namespace std;

extern bool assembling;

extern std::string outputFile;

void stopAssembling();

struct FileHeader {
    int sectionCount;
    int symbolCount;
    int relocationCount;
    FileHeader(int nSec, int nSym, int nRel) : sectionCount(nSec), symbolCount(nSym), relocationCount(nRel) {}
    FileHeader() : sectionCount(0), symbolCount(0), relocationCount(0) {}
};

struct Section_ {
    string name;
    vector<uint8_t> data;
    Section_(string name, vector<uint8_t> data) : name(name), data(data) {}
    Section_(){}
};

struct Symbol_ {
    string name;
    uint32_t value;
    int section;
    Symbol_(string name, uint32_t value, int section) : name(name), value(value), section(section){}
    Symbol_(){}

};

struct Relocation_ {
    int section;
    uint32_t offset;
    int symbol;
    uint32_t addent;
    string sectionName = "";
    string symbolName = "";
    Relocation_(int section, uint32_t offset, int symbol, uint32_t addent) : section(section), offset(offset), symbol(symbol), addent(addent){}
    Relocation_(){}
};

void writeToFile(const std::string& filename, const std::vector<Section_>& sections, const std::vector<Symbol_>& symbols, const std::vector<Relocation_>& relocations);
void readFromFile(const std::string& filename, std::vector<Section_>& sections, std::vector<Symbol_>& symbols, std::vector<Relocation_>& relocations);
     

#endif //ASSEMBLER_HPP