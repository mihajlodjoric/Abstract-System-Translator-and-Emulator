#include "../../inc/emulator/Emulator.hpp"

int main(int argc, char const *argv[]){
  if(argc != 2){
    cerr << "Usage: ./emulator [inputFileName]";
    return 1;
  }
  string inputFile = argv[1];

  Emulator emulator(inputFile);
  emulator.start();


}
