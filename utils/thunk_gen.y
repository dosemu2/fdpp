%{
/*
 * function prototype parser
 * Author: Stas Sergeev
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define YYDEBUG 1

static void yyerror(const char* s);
int yylex(void);
extern char *yytext;
extern FILE *yyin;

static int thunk_type;

static int arg_num;
static int arg_offs;
static int arg_size;
static int is_ptr;
static int is_far;
static char rbuf[256];
static char abuf[256];
static char atype[256];
static char rtbuf[256];


static void beg_arg(void)
{
    is_far = 0;
    is_ptr = 0;
    atype[0] = 0;
}

static void fin_arg(int last)
{
    if (!atype[0])
	return;
    if (is_ptr) {
	if (is_far) {
	    strcat(abuf, "_ARG_PTR_FAR(");
	    arg_size = 4;
	} else {
	    strcat(abuf, "_ARG_PTR(");
	    arg_size = 2;
	}
    } else {
	strcat(abuf, "_ARG(");
    }
    sprintf(abuf + strlen(abuf), "%i, %s, _SP)", arg_offs, atype);
    if (!last) {
        assert(arg_size != -1);
        arg_offs += arg_size;
    }
    if (arg_size)
	arg_num++;
}

%}

%token LB RB SEMIC COMMA ASMCFUNC FAR ASTER NEWLINE STRING NUM
%token VOID WORD UWORD BYTE UBYTE STRUCT

%define api.value.type union
%type <int> num lnum
%type <char *> str fname sname tname

%%

input:		  input line NEWLINE
		|
;

line:		lnum rdecls fname lb args rb SEMIC
			{
			  switch (thunk_type) {
			  case 0:
			    printf("\tcase %i:\n%s%s(%s);\n\t\tbreak;\n",
				$1, rbuf, $3, abuf);
			    break;
			  case 1:
			    if (rtbuf[0])
			      printf("_THUNK%i(%i, %s, %s", arg_num, $1, rtbuf, $3);
			    else
			      printf("_THUNK%i_v(%i, %s", arg_num, $1, $3);
			    if (arg_num)
			      printf(", %s", abuf);
			    printf(")\n");
			    break;
			  }
			}
;

lb:		LB	{ arg_offs = 0; arg_num = 0; }
;
rb:		RB	{ fin_arg(1); }
;

lnum:		num
;
num:		NUM	{ $$ = atoi(yytext); }

fname:		str
;
sname:		str
;
tname:		str
;
str:		STRING	{ $$ = strdup(yytext); }
;

decls:		  ASMCFUNC decls
		| FAR decls	{ is_far = 1; }
		| ASTER decls	{ is_ptr = 1; }
		|
;

rtype:		  VOID		{ strcpy(rbuf, "\t\t_RSZ = 0;\n\t\t");
				  rtbuf[0] = 0;
				}
		| WORD		{ strcpy(rbuf, "\t\t_RSZ = 2;\n\t\t_RET = ");
				  strcpy(rtbuf, "WORD");
				}
		| UWORD		{ strcpy(rbuf, "\t\t_RSZ = 2;\n\t\t_RET = ");
				  strcpy(rtbuf, "UWORD");
				}
		| BYTE		{ strcpy(rbuf, "\t\t_RSZ = 1;\n\t\t_RET = ");
				  strcpy(rtbuf, "BYTE");
				}
		| UBYTE		{ strcpy(rbuf, "\t\t_RSZ = 1;\n\t\t_RET = ");
				  strcpy(rtbuf, "UBYTE");
				}
;

atype:		  VOID		{ beg_arg(); arg_size = 0; }
		| WORD		{ beg_arg(); arg_size = 2; strcpy(atype, "WORD"); }
		| UWORD		{ beg_arg(); arg_size = 2; strcpy(atype, "UWORD"); }
		| BYTE		{ beg_arg(); arg_size = 1; strcpy(atype, "BYTE"); }
		| UBYTE		{ beg_arg(); arg_size = 1; strcpy(atype, "UBYTE"); }
		| STRUCT sname	{ beg_arg(); arg_size = -1; sprintf(atype, "struct %s", $2); }
		| tname		{ beg_arg(); arg_size = -1; sprintf(atype, "%s", $1); }
;

rdecls:		rtype decls	{ abuf[0] = 0; }
;

adecls:		atype decls
;

argsep:		COMMA		{ fin_arg(0); strcat(abuf, ", "); }

args:		  args argsep arg
		| arg
;

arg:		  adecls STRING
		| adecls
;

%%

int main(int argc, char *argv[])
{
    yydebug = 0;

    if (argc >= 2)
	thunk_type = atoi(argv[1]);
    yyparse();
    return 0;
}

void yyerror(const char* s)
{
    fprintf(stderr, "Parse error: %s\n", s);
    exit(1);
}
