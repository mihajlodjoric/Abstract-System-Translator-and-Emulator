%{
  #include "parser.hpp"
  #include <iostream>
%}

%option outfile = "misc/lexer.cpp" header-file = "misc/lexer.hpp"
%option noyywrap


%%
[#].+ {}
[\t ] {}

\.global return DIR_GLOBAL;
\.extern return DIR_EXTERN; 
\.section return DIR_SECTION; 
\.word return DIR_WORD; 
\.skip return DIR_SKIP; 
\.ascii return DIR_ASCII; 
\.equ return DIR_EQU; 
\.end return DIR_END; 

halt return INS_HALT;
int return INS_INT;
iret return INS_IRET;
ret return INS_RET;

call return INS_CALL;
jmp return INS_JMP;

beq return INS_BEQ;
bne return INS_BNE;
bgt return INS_BGT;

push return INS_PUSH;
pop return INS_POP;
not return INS_NOT;

xchg return INS_XCHG;
add return INS_ADD;
sub return INS_SUB;
mul return INS_MUL;
div return INS_DIV;
and return INS_AND;
or return INS_OR;
xor return INS_XOR;
shl return INS_SHL;
shr return INS_SHR;

ld return INS_LD;
st return INS_ST;
csrrd return INS_CSRRD;
csrwr return INS_CSRWR;

\: return COLLON;
\$ return DOLLAR;
\[ return L_BRACKET;
\] return R_BRACKET;
\+ return PLUS;
\, return COMMA;
\" return QUOTE;

[0-9]+ {yylval.num = atoi(yytext); return NUM_DEC;}
0x[0-9a-fA-F]+ {yylval.num = strtol(yytext, NULL, 16); return NUM_HEX; }

\%r[0-9] {yylval.num = atoi(yytext+2); return GPR;}
\%r1[0-5] {yylval.num = atoi(yytext+2); return GPR;}
\%sp {yylval.num = 14; return GPR;}
\%pc {yylval.num = 15; return GPR;}
\%status {yylval.num = 16; return CSR;}
\%handler {yylval.num = 17; return CSR;}
\%cause {yylval.num = 18; return CSR;}

[a-zA-Z][a-zA-Z_0-9]* {yylval.str = strdup(yytext); return SYMBOL;}

\"[a-zA-Z_0-9 !@#$%^&*,.;:]*\" {
  yylval.str = strdup(yytext); return STRING;}

. {return ERROR; }
\n {return EOL;}
%%
