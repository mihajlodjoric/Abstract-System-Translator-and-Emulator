#include "../../inc/linker/File.hpp"
#include "../../inc/linker/Error.hpp"

void readFromFile(const std::string& filename, std::vector<Section_>& sections, std::vector<Symbol_>& symbols, std::vector<Relocation_>& relocations) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        throw std::ios_base::failure("Failed to open file " + filename);
    }
    FileHeader fileHeader;
    inFile.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));

    // Read sections
    sections.clear();
    for (int i = 0; i < fileHeader.sectionCount; ++i) {
        Section_ section;

        uint32_t nameSize;
        inFile.read(reinterpret_cast<char*>(&nameSize), sizeof(nameSize));
        section.name.resize(nameSize);
        inFile.read(&section.name[0], nameSize);

        uint32_t dataSize;
        inFile.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
        section.data.resize(dataSize);
        inFile.read(reinterpret_cast<char*>(section.data.data()), dataSize);

        sections.push_back(section);
    }

    // Read symbols
    symbols.clear();
    for (int i = 0; i < fileHeader.symbolCount; ++i) {
        Symbol_ symbol;

        uint32_t nameSize;
        inFile.read(reinterpret_cast<char*>(&nameSize), sizeof(nameSize));
        symbol.name.resize(nameSize);
        inFile.read(&symbol.name[0], nameSize);

        inFile.read(reinterpret_cast<char*>(&symbol.value), sizeof(symbol.value));
        inFile.read(reinterpret_cast<char*>(&symbol.section), sizeof(symbol.section));

        symbols.push_back(symbol);
    }

    // Read relocations
    relocations.clear();
    for (int i = 0; i < fileHeader.relocationCount; ++i) {
        Relocation_ relocation;
        inFile.read(reinterpret_cast<char*>(&relocation.section), sizeof(relocation.section));
        inFile.read(reinterpret_cast<char*>(&relocation.offset), sizeof(relocation.offset));
        inFile.read(reinterpret_cast<char*>(&relocation.symbol), sizeof(relocation.symbol));
        inFile.read(reinterpret_cast<char*>(&relocation.addent), sizeof(relocation.addent));
                 
        relocations.push_back(relocation);
    }

    inFile.close();
    }


void File::updateSymbols(){
  for(Symbol_& sym: symbolTable){
    for(int i = 1; i < sections.size(); i++){
      if(sym.section == i){
        sym.value += sections[i].offset;
        break;
      }
    }
  }
}

void File::updateRelocations(){
  for(Relocation_& rel: relocations){
    for(int i = 1; i < sections.size(); i++){
      if(rel.section == i){
        rel.offset += sections[i].offset;
        if(symbolTable[rel.symbol].isSection){
          rel.addent += symbolTable[rel.symbol].value;
        }
        break;
      }
    }
  }
}

void File::solveRelocations(map<string, uint32_t>& symbolValues){
  for(Relocation_ rel: relocations){
    Symbol_& usedSymbol = symbolTable[rel.symbol];
    Section_& section = sections[rel.section];
    uint32_t value;

    if(usedSymbol.section == 0){
      if(symbolValues.count(usedSymbol.name) == 0){
        throw NotDefined(usedSymbol.name);
      }
      value = symbolValues[usedSymbol.name];
    }
    else{
      value = usedSymbol.value + rel.addent;
    }

    for(size_t i = 0; i < 4; i++){
      section.data[rel.offset + i] = static_cast<uint8_t>(value >> (i * 8));
    }
  }
}

