%option noyywrap

%{
/*
 * function prototype parser - lexer
 * Author: Stas Sergeev
 *
 */

#include <stdio.h>

#define YY_DECL int yylex()

#include "thunk_gen.tab.h"

%}

%%

[0-9]+		{
		  yylval.NUM = atoi(yytext);
		  return NUM;
		}
"("		{ return LB; }
")"		{ return RB; }
";"		{ return SEMIC; }
","		{ return COMMA; }
"\n"		{ return NEWLINE; }
"ASMCFUNC"	{ return ASMCFUNC; }
"ASMPASCAL"	{ return ASMPASCAL; }
"ASMFUNC"	{ }
"INITTEXT"	{ return INITTEXT; }
"SEGM"		{ return SEGM; }
"FAR"		{ return FAR; }
"far"		{ return FAR; }
"*"		{ return ASTER; }
"["		{ return LBR; }
"]"		{ return RBR; }
"VOID"		{ return VOID; }
"void"		{ return VOID; }
"WORD"		{ return WORD; }
"COUNT"		{ return WORD; }
"BOOL"		{ return WORD; }
"char"		{ return CHAR; }
"short"		{ return WORD; }
"UCOUNT"	{ return UWORD; }
"UWORD"		{ return UWORD; }
"seg"		{ return UWORD; }
"BYTE"		{ return BYTE; }
"UBYTE"		{ return UBYTE; }
"unsigned char"	{ return UBYTE; }
"unsigned short"	{ return UWORD; }
"ULONG"		{ return UDWORD; }
"DWORD"		{ return DWORD; }
"UDWORD"	{ return UDWORD; }
"struct"	{ return STRUCT; }
"const"		{ return CONST; }
"NORETURN"	{ return NORETURN; }
"/*".*"*/"	{ /* comments */ }
[\t ]+		{ /* skip spaces */ }
[_A-Za-z][_A-Za-z0-9]*	{
			  yylval.STRING = strdup(yytext);
			  return STRING;
			}

%%
