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
"SEGM"		{ return SEGM; }
"FAR"		{ return FAR; }
"far"		{ return FAR; }
"*"		{ return ASTER; }
"VOID"		{ return VOID; }
"void"		{ return VOID; }
"WORD"		{ return WORD; }
"COUNT"		{ return INT; }
"BOOL"		{ return INT; }
"char"		{ return BYTE; }
"short"		{ return WORD; }
"int"		{ return INT; }
"unsigned"	{ return UINT; }
"long"		{ return LONG; }
"UCOUNT"	{ return UINT; }
"UWORD"		{ return UWORD; }
"seg"		{ return UWORD; }
"BYTE"		{ return BYTE; }
"UBYTE"		{ return UBYTE; }
"unsigned char"	{ return UBYTE; }
"unsigned short"	{ return UWORD; }
"unsigned long"	{ return ULONG; }
"ULONG"		{ return ULONG; }
"struct"	{ return STRUCT; }
"const"		{ return CONST; }
"/*".*"*/"	{ /* comments */ }
[\t ]+		{ /* skip spaces */ }
[_A-Za-z][_A-Za-z0-9]*	{
			  yylval.STRING = strdup(yytext);
			  return STRING;
			}

%%
