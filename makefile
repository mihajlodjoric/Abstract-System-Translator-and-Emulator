ASSEMBLER_REQ = src/assembler/Assembler.cpp\
								src/assembler//SymbolTable.cpp\
								src/assembler//Pool.cpp\
								src/assembler//SectionTable.cpp\
								src/assembler//RelocationTable.cpp\
								src/assembler//Directive.cpp\
								src/assembler//Operand.cpp\
								misc/lexer.cpp\
								misc/parser.cpp\

LINKER_REQ = 		src/linker/Main.cpp\
								src/linker/Linker.cpp\
								src/linker/File.cpp\

EMULATOR_REQ = 	src/emulator/Main.cpp\
								src/emulator/Emulator.cpp\
								src/emulator/Terminal.cpp\


all: assembler linker emulator

flex: bison
	flex misc/flex.l 

bison:
	bison -t -d misc/bison.y

assembler: flex bison 
	g++ -std=c++17 -o ${@} ${ASSEMBLER_REQ} 

linker:
	g++ -std=c++17 -o ${@} ${LINKER_REQ} 

emulator:
	g++ -std=c++17 -o ${@} ${EMULATOR_REQ} 

clean:
	rm -f assembler
	rm -f linker
	rm -f emulator