#include "../../inc/linker/Linker.hpp"
#include "../../inc/linker/File.hpp"
#include "../../inc/linker/Error.hpp"
#include <algorithm>
#include <set>

void Linker::processArgument(string arg){
  if(arg == "-o") {
    waitingForOutoutArg = true;
  }
  else if(waitingForOutoutArg){
    outputFileName = arg;
    waitingForOutoutArg = false;
  }
  else if(arg == "-hex"){
    modeHEX = true;
  }
  else if(arg == "-relocatable"){
    modeRELOCATABLE = true;
  }

  else if(arg.substr(0, 7) == "-place=") {
    string value = arg.substr(7);

    // -place=text@0x40000000

    size_t atPos = value.find('@');
    if (atPos == string::npos) {
        throw invalid_argument("Missing '@' in option value");
    }

    string section = value.substr(0, atPos);
    string address = value.substr(atPos + 1);
    uint32_t addr = (address.substr(0, 2) == "0x" ? stoul(address, nullptr, 16) : stoul(address));
    placements[section] = addr;
    places.push_back(Place(section, addr));
  } 
  else{
    inputFiles.push_back(File(arg));
  }
}

void Linker::processHEX(){
  placeSections();
  updateSymbols();
  collectSymbols();
  solveRelocations();
  generateHex();

  writeToFile(outputFileName, memoryData);

  string textFileName = outputFileName.substr(0, outputFileName.size() - 4) + ".txt";
  ofstream ofs(textFileName);
  printHex(ofs);
  //printSections();
}

void Linker::start(){
  if(modeHEX && modeRELOCATABLE){
    throw("Error: Only one linker mode allowed");
  }
  else if(!modeHEX && !modeRELOCATABLE){
    throw("Error: Linker mode not specified");
  }
  else if(modeHEX){
    processHEX();
  }
  else if(modeRELOCATABLE){
    processREL();
  }
}

void Linker::processREL(){
  mergeSections();
  updateSymbols();
  mergeSymbolTables();
  updateRelocations();
  mergeRelocations();
  
  vector<Section_> secs;
  for(string name: sectionNames){
    secs.push_back(sections[name]);
  }
  vector<Symbol_> syms;
  for(string name: symbolNames){
    syms.push_back(symbolTable[name]);
  }

  writeToFile(outputFileName, secs, syms, relocations);
  string textFileName = outputFileName.substr(0, outputFileName.size() - 2) + ".txt";
  ofstream ofs(textFileName);
  printInternSections(ofs);
  printInternSymbols(ofs);
  printInternRels(ofs);
}



void Linker::placeSections(){
  uint32_t currentOffset = 0;
  sort(places.begin(), places.end(), [](const Place& p1, const Place& p2){return p1.address < p2.address;});

  for(Place& place: places){
    if(currentOffset > place.address) throw SectionOverlapping(place.sectionName);
    this->sectionNames.push_back(place.sectionName);
    symbolValues[place.sectionName] = place.address;

    currentOffset = place.address;
    for(File& input: inputFiles){
      for(Section_& section: input.getSections()){
        if(section.name == place.sectionName){
          section.offset = currentOffset;
          currentOffset += section.size();
          break;
        }
      }
    }
  }
  vector<string> secs;
  set<string> seen;
  for(File& input: inputFiles){
    for(Section_& section: input.getSections()){
      if(placements.count(section.name) == 0 && seen.count(section.name) == 0){
        secs.push_back(section.name);
        seen.insert(section.name);
      }
    }
  }
  for(string sectionName: secs){
    sectionNames.push_back(sectionName);
    symbolValues[sectionName] = currentOffset;

    for(File& input: inputFiles){
      for(Section_& section: input.getSections()){
        if(section.name == sectionName){
          section.offset = currentOffset;

          currentOffset += section.size();
          break;
        }
      }
    }
  }
}

void Linker::mergeSections(){
  int currentId = 0;
  for(File& input: inputFiles){
    for(Section_& section: input.getSections()){
      if(this->sections.count(section.name) == 0){
        sectionNames.push_back(section.name);
        sections[section.name].name = section.name;
        sections[section.name].id = currentId++;
      }
      section.offset = sections[section.name].size();
      sections[section.name].concat(section);
    }
  }
}

void Linker::mergeSymbolTables(){
  int currentId = 0;
  for(File& input: inputFiles){
    for(Symbol_& symbol: input.getSymbols()){
      if(this->symbolTable.count(symbol.name) == 0){
        symbolTable[symbol.name].name = symbol.name;
        symbolTable[symbol.name].sectionName = symbol.sectionName;
        symbolTable[symbol.name].value = symbol.value;
        symbolTable[symbol.name].id = currentId++;
        symbolTable[symbol.name].isSection = symbol.isSection;
        symbolNames.push_back(symbol.name);

      }
      else{
        if(symbolTable[symbol.name].section != 0 && symbol.section != 0 && !symbol.isSection){
          throw MultipleDefinitions(symbol.name);
        }
        if(symbolTable[symbol.name].section == 0 && symbol.section != 0){
          symbolTable[symbol.name].sectionName = symbol.sectionName;
          symbolTable[symbol.name].value = symbol.value;
        }
      }
    }
  }
  for(string symbolName: symbolNames){
    if(symbolTable[symbolName].isSection){
      symbolTable[symbolName].value = 0;
    }
    symbolTable[symbolName].section = sections[symbolTable[symbolName].sectionName].id;
  }
}

void Linker::updateRelocations(){
  for(File& input: inputFiles){
    input.updateRelocations();
  }
}

void Linker::mergeRelocations(){
  for(File& input: inputFiles){
    vector<Relocation_> inputRels = input.getRelocations();
    relocations.insert(relocations.end(), inputRels.begin(), inputRels.end());
  }

  for(Relocation_& rel: relocations){
    rel.section = sections[rel.sectionName].id;
    rel.symbol = symbolTable[rel.symbolName].id;
  }
}


void Linker::updateSymbols(){
  for(File& input: inputFiles){
    input.updateSymbols();
  }
}

void Linker::collectSymbols(){
  for(File& input: inputFiles){
    for(Symbol_& symbol: input.getGlobalSymbols()){
      if(symbolValues.count(symbol.name) == 0){
        symbolValues[symbol.name] = symbol.value;
      }
      else{
        throw MultipleDefinitions(symbol.name);
      }
    }
  }
}

void Linker::solveRelocations(){
  for(File& input: inputFiles){
    input.solveRelocations(symbolValues);
  }
}


void Linker::generateHex(){
  for(string& sectionName: sectionNames){
    for(File& input: inputFiles){
      for(Section_& fileSection: input.getSections()){
        if(fileSection.name == sectionName){
          for(int i = 0; i < fileSection.size(); i++){
            memoryData[fileSection.offset + i] = fileSection.data[i];
          }
        }
      }
    }
  }
}

void Linker::printHex(ofstream& ofs){
  for(auto& d: memoryData){
    uint32_t addr = d.first;
    uint32_t value = d.second;

    if(addr % 8 == 0){
      ofs << endl << hex << setw(4) << setfill('0') << addr << dec << setfill(' ') << ": "; 
    }
    ofs << hex << setw(2) << setfill('0') << value << dec << setfill(' ') << ' ';
  }
}




void Linker::printHex(){
  for(auto& d: memoryData){
    uint32_t addr = d.first;
    uint32_t value = d.second;

    if(addr % 8 == 0){
      cout << endl << hex << setw(4) << setfill('0') << addr << dec << setfill(' ') << ": "; 
    }
    cout << hex << setw(2) << setfill('0') << value << dec << setfill(' ') << ' ';
  }
}

void Linker::printSections(){
  for(File& input: inputFiles){
    input.printSections();
  }
}

void Linker::printSymbols(){
  for(File& input: inputFiles){
    input.printSymbols();
  }
}

void Linker::printRelocations(){
  for(File& input: inputFiles){
    input.printRelocations();
  }
}

void Linker::printData(){
  for(File& input: inputFiles){
    input.printData();
  }
}


void Linker::printInternSections(ofstream& ofs){
  for(string sectionName: sectionNames){
    if(sectionName == "") continue;
    Section_ section = sections[sectionName];
    
    ofs << ".section " << section.name << endl;
    ofs << "-----------------------------------------------------" << endl;
    ofs << hex << 0 << dec << ":\t\t\t";
    for(int i = 0; i < section.size(); i++){
      ofs << hex << setw(2) << setfill('0') << (int)(section.data[i]);
      if(i % 4 == 3) ofs << ' ';
      if(i % 16 == 15){
        ofs << endl;
        if(i != section.size() - 1) ofs << hex << i+1 << dec << ":\t\t\t";
      }
    }
    ofs << endl << endl << endl;
  }
  ofs << dec << setfill(' ');
}

void Linker::printInternSymbols(ofstream& ofs){
  ofs << "Symbol Table" << endl;
  ofs << setfill(' ');
  ofs << left
      << setw(8) << "ID"
      << setw(16) << "Name" 
      << setw(16) << "Section" 
      << setw(16) << "Value" 
      << endl;
  ofs << "-----------------------------------------------------------------------------" << endl;
  
  for(string symbolName: symbolNames){
    Symbol_ symbol = symbolTable[symbolName];
    ofs << left
      << setw(8) << symbol.id
      << setw(16) << symbol.name 
      << setw(16) << symbol.sectionName
      << setw(16) << hex << symbol.value << dec 
      << endl;
  }
  ofs << right;
  ofs << endl << endl;
}

void Linker::printInternRels(ofstream& ofs){

  vector<vector<Relocation_>> sectionRels(sectionNames.size());
  for(Relocation_& rel: relocations){
    sectionRels[rel.section].push_back(rel);
  }

  for(int i = 1; i < sectionNames.size(); i++){

    ofs << sectionNames[i] << ".rel" << endl;

    ofs << left
      << setw(8) << "Offset" 
      << setw(8) << "Symbol" 
      << setw(8) << "Addent" 
      << endl;
    ofs << "------------------------------" << endl;
    
    for(Relocation_ rel: sectionRels[i]){
      ofs << left
        << setw(8) << hex << rel.offset << dec
        << setw(16) << rel.symbolName
        << setw(8) << hex << rel.addent << dec 
        << endl;
    }
    ofs << right;
    ofs << endl << endl;
  }
}



void writeToFile(const std::string& filename, const std::vector<Section_>& sections, const std::vector<Symbol_>& symbols, const std::vector<Relocation_>& relocations) {
        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile) {
            throw std::ios_base::failure("Failed to open file for writing");
        }

        FileHeader fileHeader(sections.size(), symbols.size(), relocations.size());
        outFile.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));

        // Write sections
        for (const auto& section : sections) {
            uint32_t nameSize = section.name.size();
            outFile.write(reinterpret_cast<const char*>(&nameSize), sizeof(nameSize));
            outFile.write(section.name.data(), nameSize);

            uint32_t dataSize = section.data.size();
            outFile.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
            outFile.write(reinterpret_cast<const char*>(section.data.data()), dataSize);
        }

        // Write symbols
        for (const auto& symbol : symbols) {
            uint32_t nameSize = symbol.name.size();
            outFile.write(reinterpret_cast<const char*>(&nameSize), sizeof(nameSize));
            outFile.write(symbol.name.data(), nameSize);

            outFile.write(reinterpret_cast<const char*>(&symbol.value), sizeof(symbol.value));
            outFile.write(reinterpret_cast<const char*>(&symbol.section), sizeof(symbol.section));
        }

        // Write relocations
        for (const auto& relocation : relocations) {
            outFile.write(reinterpret_cast<const char*>(&relocation.section), sizeof(relocation.section));
            outFile.write(reinterpret_cast<const char*>(&relocation.offset), sizeof(relocation.offset));
            outFile.write(reinterpret_cast<const char*>(&relocation.symbol), sizeof(relocation.symbol));
            outFile.write(reinterpret_cast<const char*>(&relocation.addent), sizeof(relocation.addent));
        }

        outFile.close();
    }


void writeToFile(const std::string& filename, const map<uint32_t, uint8_t> memory) {
  std::ofstream outFile(filename, std::ios::binary);
  if (!outFile) {
      throw std::ios_base::failure("Failed to open file for writing");
  }

  // Write the size of the map
  uint32_t mapSize = static_cast<uint32_t>(memory.size());
  outFile.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));
  
  // Write each key-value pair
  for (const auto& pair : memory) {
      outFile.write(reinterpret_cast<const char*>(&pair.first), sizeof(pair.first));
      outFile.write(reinterpret_cast<const char*>(&pair.second), sizeof(pair.second));
  }
  
  outFile.close();
}



