#include "../../inc/assembler/Assembler.hpp"
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

void stopAssembling(){
  assembling = false; 
}

extern FILE* yyin;
extern int yyparse();

std::string outputFile;


int main(int argc, char const *argv[]){

  outputFile = "output.o";
  string inputFile;

  if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " [-o output_file] input_file" << std::endl;
      return 1;
  }

  for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "-o") {
        if (i + 1 >= argc) {
            std::cerr << "Error: -o option requires an output file argument." << std::endl;
            return 1;
        }
        outputFile = argv[i + 1];
        i++;
      } 
      else {
        inputFile = arg;
      }
  }

  FILE *myfile = fopen(inputFile.c_str(), "r");

  if (!myfile) {
    cout << "Greska pri ucitavanju ulaznog fajla" << endl;
    return -1;
  }

  yyin = myfile;

  while(yyparse() && assembling);


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