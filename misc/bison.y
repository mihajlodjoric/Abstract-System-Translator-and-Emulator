%{
  #include "../inc/assembler/Assembler.hpp"
  #include "../inc/assembler/Directive.hpp"
  #include "../inc/assembler/Instruction.hpp"
  #include "../inc/assembler/Operand.hpp"
  #include "../inc/assembler/SymbolTable.hpp"

  extern int yylex();
  void yyerror(const char* c);
  bool assembling = true;

  extern int yylineno;
  extern char *yytext;
%}

%output "misc/parser.cpp"
%defines "misc/parser.hpp"

%union {
  int num;
  char* str; 
  void* obj;
}

%token EOL ERROR
%token COLLON DOLLAR L_BRACKET R_BRACKET PLUS COMMA QUOTE

%token <str> STRING SYMBOL
%token <num> NUM_DEC NUM_HEX GPR CSR

%token DIR_GLOBAL DIR_EXTERN DIR_SECTION DIR_WORD DIR_SKIP DIR_ASCII DIR_EQU DIR_END
%token INS_HALT INS_INT INS_IRET INS_RET
       INS_JMP INS_CALL
       INS_BEQ INS_BNE INS_BGT
       INS_PUSH INS_POP INS_NOT
       INS_XCHG INS_ADD INS_SUB INS_MUL INS_DIV INS_AND INS_OR INS_XOR INS_SHL INS_SHR
       INS_LD INS_ST INS_CSRRD INS_CSRWR

%type <str> label
%type <num> literal
%type <num> ins_noarg ins_jump ins_branch ins_arilog ins_stack
%type <obj> directive instruction symlit_list symbol_list operand_jmp operand operand_st



%%
line: |
  line directive EOL {
    ((Directive*) $2)->process();
    delete (Directive*) $2;
    if(!assembling) YYABORT;
  }|
  line label directive EOL {
    SymbolTable::getInstance().insertSymbol($2); 
    ((Directive*) $3)->process();
    delete (Directive*) $3;
    if(!assembling) YYABORT;
  }|
  line instruction EOL {
    ((Instruction*) $2)->process();
    delete (Instruction*)$2;
    if(!assembling) YYABORT;
  }|
  line label instruction EOL {
    SymbolTable::getInstance().insertSymbol($2);
    ((Instruction*) $3)->process();
    delete (Instruction*)$3;
    if(!assembling) YYABORT;
  }|
  line EOL {
    if(!assembling) YYABORT;
  }
  |                 
  line label EOL {
    SymbolTable::getInstance().insertSymbol($2);
    if(!assembling) YYABORT;
  };

directive:
  DIR_GLOBAL symbol_list             {$$ = new DirectiveGlobal((vector<string>*) $2);}|       
  DIR_EXTERN symbol_list             {$$ = new DirectiveExtern((vector<string>*) $2);}| 
  DIR_SECTION SYMBOL                 {$$ = new DirectiveSection($2);}|
  DIR_WORD symlit_list               {$$ = new DirectiveWord((vector<variant<int, string>>*) $2);}| 
  DIR_SKIP literal                   {$$ = new DirectiveSkip($2);}| 
  DIR_ASCII STRING                   {$$ = new DirectiveAscii($2);}| 
  DIR_EQU SYMBOL COMMA literal       {$$ = new DirectiveEqu($2, $4);}| 
  DIR_END                            {$$ = new DirectiveEnd(); };

instruction:
  ins_noarg                                  {$$ = new NoArgInstruction($1); }|
  ins_jump operand_jmp                       {$$ = new JumpInstruction($1, (OperandJump*)$2); delete (OperandJump*)$2;}|
  ins_branch GPR COMMA GPR COMMA operand_jmp {$$ = new BranchInstruction($1, $2, $4, (OperandJump*)$6); delete (OperandJump*)$6; }|
  ins_stack GPR                              {$$ = new StackInstruction($1, $2 ); }|
  INS_NOT GPR                                {$$ = new AriLogInstruction(NOT, 0, $2); }|
  ins_arilog GPR COMMA GPR                   {$$ = new AriLogInstruction($1, $2, $4); }|
  INS_LD operand COMMA GPR                   {$$ = new LoadInstruction(LD, $4, (Operand*)$2); delete (Operand*)$2;}|
  INS_ST GPR COMMA operand_st                {$$ = new StoreInstruction(ST, $2, (Operand*)$4); delete (Operand*)$4; }|
  INS_CSRRD CSR COMMA GPR                    {$$ = new CSRInstruction(CSRRD, $2, $4); }|
  INS_CSRWR GPR COMMA CSR                    {$$ = new CSRInstruction(CSRWR, $2, $4); };

label:
  SYMBOL COLLON {$$ = $1; }

operand:
  DOLLAR literal                       {$$ = new LiteralDirect($2);}|
  literal                              {$$ = new LiteralIndirect($1); }|
  DOLLAR SYMBOL                        {$$ = new SymbolDirect($2); }|
  SYMBOL                               {$$ = new SymbolIndirect($1); }|
  GPR                                  {$$ = new RegisterDirect($1); }|
  L_BRACKET GPR R_BRACKET              {$$ = new RegisterIndirect($2); }|
  L_BRACKET GPR PLUS literal R_BRACKET {$$ = new RegisterLiteral($2, $4); }|
  L_BRACKET GPR PLUS SYMBOL R_BRACKET  {$$ = new RegisterSymbol($2, $4); };
operand_st:
  literal                              {$$ = new LiteralDirect($1); }|
  SYMBOL                               {$$ = new SymbolDirect($1); }|
  L_BRACKET GPR R_BRACKET              {$$ = new RegisterDirect($2); }|
  L_BRACKET GPR PLUS literal R_BRACKET {$$ = new RegisterLiteral($2, $4); }|
  L_BRACKET GPR PLUS SYMBOL R_BRACKET  {$$ = new RegisterSymbol($2, $4); };
operand_jmp:
  literal {$$ = new LiteralDirect($1); }|
  SYMBOL {$$ = new SymbolDirect($1); };

literal:
  NUM_DEC {$$ = $1; }|
  NUM_HEX {$$ = $1; };

symbol_list:
  SYMBOL                      {$$ = new vector<string>(); ((vector<string>*)$$)->push_back($1); }|
  symbol_list COMMA SYMBOL    {((vector<string>*)$$)->push_back($3); };
  
symlit_list:
  literal                     {$$ = new vector<variant<int, string>>(); ((vector<variant<int, string>>*)$$)->push_back($1); }|
  SYMBOL                      {$$ = new vector<variant<int, string>>(); ((vector<variant<int, string>>*)$$)->push_back($1); }|
  symlit_list COMMA literal   {((vector<variant<int, string>>*)$$)->push_back($3); }|
  symlit_list COMMA SYMBOL    {((vector<variant<int, string>>*)$$)->push_back($3); };


ins_noarg:
  INS_HALT {$$ = HALT; }|
  INS_INT {$$ = INT; }|
  INS_IRET {$$ = IRET; }|
  INS_RET {$$ = RET; };

ins_jump:
  INS_JMP {$$ = JMP; }|
  INS_CALL {$$ = CALL; };

ins_branch:
  INS_BEQ {$$ = BEQ; }|
  INS_BNE {$$ = BNE; }|
  INS_BGT {$$ = BGT; };

ins_stack:
  INS_PUSH {$$ = PUSH; }|
  INS_POP {$$ = POP; };

ins_arilog:
  INS_XCHG {$$ = XCHG; }|
  INS_ADD {$$ = ADD; }|
  INS_SUB {$$ = SUB; }|
  INS_MUL {$$ = MUL; }|
  INS_DIV {$$ = DIV; }|

  INS_AND {$$ = AND; }|
  INS_OR {$$ = OR; }|
  INS_XOR {$$ = XOR; }|

  INS_SHL {$$ = SHL; }|
  INS_SHR {$$ = SHR; };

%%

void yyerror(const char* c) {
  fprintf(stderr, "Error: %s at line %d\n", c, yylineno);
  fprintf(stderr, "Unexpected token: %s\n", yytext);
  stopAssembling();
}