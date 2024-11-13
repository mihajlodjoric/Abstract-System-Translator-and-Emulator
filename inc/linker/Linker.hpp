#ifndef LINKER_HPP
#define LINKER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <fstream>
using namespace std;



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
    uint32_t offset = 0;
    int id;
    Section_(string name) : name(name) {}
    Section_(string name, vector<uint8_t> data) : name(name), data(data) {}
    Section_(string name, uint32_t offset) : name(name), offset(offset){}
    Section_(){}
    void concat(Section_& sec){data.insert(data.end(), sec.data.begin(), sec.data.end()); }
    uint32_t size() const {return data.size(); }
};

struct Symbol_ {
    string name;
    uint32_t value;
    int section;
    int id;
    bool isSection = false;
    string sectionName = "";
    Symbol_(string name, uint32_t value, int section) : name(name), value(value), section(section){}
    Symbol_(){}
    bool isExtern(){return section == 0;}

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


struct Place {
    string sectionName;
    uint32_t address;
    Place(string sectionName, uint32_t address) : sectionName(sectionName), address(address) {}
};

struct SectionH {
    string name;
    uint32_t offset;
    uint32_t size;
    SectionH(string name, uint32_t offset, uint32_t size) : name(name), offset(offset), size(size){}
};

void writeToFile(const std::string& filename, const std::vector<Section_>& sections, const std::vector<Symbol_>& symbols, const std::vector<Relocation_>& relocations);
void writeToFile(const std::string& filename, const map<uint32_t, uint8_t>);

class File;


class Linker {
private:
    vector<File> inputFiles;
    map<string, uint32_t> placements;
    vector<Place> places;
    bool modeHEX = false;
    bool modeRELOCATABLE = false;
    string outputFileName = "linkerIzlaz.hex";
    bool waitingForOutoutArg;

    uint32_t currentOffset = 0;
    map<string, uint32_t> symbolValues;
    map<uint32_t, uint8_t> memoryData;

    vector<string> sectionNames;
    map<string, Section_> sections;
    vector<string> symbolNames;
    map<string, Symbol_> symbolTable;
    vector<Relocation_> relocations;

public:
    void processArgument(string arg);

    void start();
    void processHEX();
    void processREL();


    void placeSections();
    void collectSymbols();
    void solveRelocations();

    void generateHex();
    void printHex(ofstream& ofs);
    void printHex();

    void updateSymbols();
    void printSections();
    void printSymbols();
    void printData();
    void printRelocations();


    void mergeSections();
    void mergeSymbolTables();
    void updateRelocations();
    void mergeRelocations();

    void printInternSections();
    void printInternSymbols();
    void printInternRels();

    void printInternSections(ofstream& ofs);
    void printInternSymbols(ofstream& ofs);
    void printInternRels(ofstream& ofs);

    void printRel();
};




#endif //LINKER_HPP