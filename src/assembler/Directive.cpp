#include "../../inc/assembler/Directive.hpp"
#include "../../inc/assembler/Assembler.hpp"
#include <fstream>
#include <math.h>

void DirectiveGlobal::process(){
  for(string name: *symbols){
    symbolTable.setGlobal(name);
  }
}

void DirectiveExtern::process(){
  for(string name: *symbols){
    symbolTable.setExtern(name);
  }
}

void DirectiveSection::process(){
  sectionTable.createSection(this->name);
}

void DirectiveWord::process(){
  for(variant<int, string> arg: *this->arguments){
    if(holds_alternative<int>(arg)){

      sectionTable.writeCurrentSection(get<int>(arg), 4);

    }
    if(holds_alternative<string>(arg)){

      string symbol = get<string>(arg);

      if(symbolTable.isAbsolute(symbol)){
        sectionTable.writeCurrentSection(symbolTable.getSymbolValue(symbol), 4);
      }
      else if(symbolTable.isExtern(symbol) || symbolTable.isDefined(symbol)){
        relocationTable.createRelocation(
          symbolTable.getSymbolId(symbol),
          sectionTable.getCurrentSectionId(), 
          sectionTable.getCurrentOffset()
        );
        sectionTable.writeCurrentSection(0, 4);
          
      }
      else {
        if(symbolTable.isDeclared(symbol) == false){
          symbolTable.insertUndefinedSymbol(symbol);
        }
        symbolTable.insertForwardRef(symbol);
        sectionTable.writeCurrentSection(0, 4);
      }
    }
  }
}

void DirectiveSkip::process(){
  int n = (size % 4 == 0? size : (size/4 + 1)*4);
  for(int i = 0; i < n; i++){
    sectionTable.writeCurrentSection(0, 1);
  }
}

void DirectiveAscii::process(){
  int size = characters.length();
  int p = (size % 4 == 0? 0 : (size/4 + 1)*4 - size);
  for(char c: characters){
    sectionTable.writeCurrentSection(c, 1);
  }
  /* for(int i = 0; i < p; i++){
    sectionTable.writeCurrentSection(0, 1);
  } */
}

void DirectiveEqu::process(){
  symbolTable.insertAbsoluteSymbol(symbol, value);
}

void DirectiveEnd::process(){
  if(symbolTable.backpatch() < 0){
    stopAssembling();
    return;
  };
  sectionTable.backpatch();
  relocationTable.backpatch();  

  string textFile = outputFile.substr(0, outputFile.size()-2).append(".txt");
  std::ofstream out(textFile);
  sectionTable.print(out);
  symbolTable.print(out);
  relocationTable.print(out);

  vector<Section_> sections = sectionTable.exprotSections();
  vector<Symbol_> symbols = symbolTable.exportSymbols();
  vector<Relocation_> relocations = relocationTable.exportRelocations();

  writeToFile(outputFile, sections, symbols, relocations);

  stopAssembling();
}

