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

static int thunk_type;

static int arg_num;
static int arg_offs;
static int arg_size;
static int is_ptr;
static int is_rptr;
static int is_far;
static int is_void;
static int is_rvoid;
static int is_const;
static int is_pas;
static char rbuf[256];
static char abuf[1024];
static char atype[256];
static char atype2[256];
static char rtbuf[256];


static void beg_arg(void)
{
    is_far = 0;
    is_ptr = 0;
    is_void = 0;
    is_const = 0;
    atype[0] = 0;
    atype2[0] = 0;
}

static void do_start_arg(int anum)
{
    if (thunk_type == 1)
	strcat(abuf, "_");
    if (is_ptr) {
	if (is_far) {
	    switch (anum) {
	    case 0:
		strcat(abuf, "_ARG_PTR_FAR(");
		break;
	    case 1:
		strcat(abuf, "_ARG_PTR_FAR_A(");
		break;
	    case 2:
		strcat(abuf, "_CNV_PTR_FAR, 0");
		break;
	    }
	} else {
	    switch (anum) {
	    case 0:
		strcat(abuf, "_ARG_PTR(");
		break;
	    case 1:
		strcat(abuf, "_ARG_PTR_A(");
		break;
	    case 2:
		switch (arg_size) {
		case 0:
		    sprintf(abuf + strlen(abuf), "_CNV_PTR_VOID, %i", arg_num + 2);
		    break;
		case 1:
		    strcat(abuf, "_CNV_PTR_CHAR, 0");
		    break;
		default:
		    strcat(abuf, "_CNV_PTR, 0");
		    break;
		}
		break;
	    }
	}
    } else {
	switch (anum) {
	case 0:
	    strcat(abuf, "_ARG(");
	    break;
	case 1:
	    strcat(abuf, "_ARG_A(");
	    break;
	case 2:
	    strcat(abuf, "_CNV_SIMPLE, 0");
	    break;
	}
    }
}

static void fin_arg(int last)
{
    if (!atype[0])
	return;
    if (!is_ptr && is_void)
	return;
    do_start_arg(0);
    if (is_const)
	strcat(abuf, "const ");
    switch (thunk_type) {
    case 0:
	sprintf(abuf + strlen(abuf), "%i, %s, _SP)", arg_offs, atype);
	break;
    case 1:
	sprintf(abuf + strlen(abuf), "%s)", atype);
	strcat(abuf, ", ");
	do_start_arg(1);
	sprintf(abuf + strlen(abuf), "%s)", (atype2[0] && !is_ptr) ? atype2 : atype);
	strcat(abuf, ", ");
	do_start_arg(2);
	break;
    }
    if (is_ptr) {
	if (is_far)
	    arg_size = 4;
	else
	    arg_size = 2;
    }
    if (!last) {
        assert(arg_size != -1);
        arg_offs += arg_size;
    }
    if (arg_size) {
	arg_num++;
    } else if (arg_num) {
	fprintf(stderr, "parse error, void argument?\n");
	exit(1);
    }
}

%}

%token LB RB SEMIC COMMA ASMCFUNC ASMPASCAL FAR ASTER NEWLINE STRING NUM SEGM
%token VOID WORD UWORD BYTE UBYTE INT UINT LONG ULONG STRUCT CONST

%define api.value.type union
%type <int> num lnum NUM
%type <char *> fname sname tname STRING

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
			    if (!is_rvoid)
			      printf("_THUNK%s%i(%i, %s, %s", is_pas ? "_P_" : "", arg_num, $1,
			          rtbuf, $3);
			    else
			      printf("_THUNK%s%i_v%s(%i, %s",
			          is_pas ? "_P_" : "", arg_num,
			          is_rptr ? "p" : "", $1, $3);
			    if (arg_num)
			      printf(", %s", abuf);
			    printf(")\n");
			    break;
			  }
			}
;

lb:		LB	{ arg_offs = 0; arg_num = 0; is_rptr = is_ptr; beg_arg(); }
;
rb:		RB	{ fin_arg(1); }
;

lnum:		num	{ is_pas = 0; is_rvoid = 0; is_rptr = 0; beg_arg(); }
;
num:		NUM
;
fname:		STRING
;
sname:		STRING
;
tname:		STRING
;

decls:		  ASMCFUNC decls
		| SEGM LB STRING RB decls
		| ASMPASCAL decls	{ is_pas = 1; }
		| FAR decls	{ is_far = 1; }
		| ASTER decls	{ is_ptr = 1; }
		|
;

rtype:		  VOID		{ strcpy(rbuf, "\t\t_RSZ = 0;\n\t\t");
				  strcpy(rtbuf, "void");
				  is_rvoid = 1;
				}
		| LONG		{ strcpy(rbuf, "\t\t_RSZ = 4;\n\t\t_RET = ");
				  strcpy(rtbuf, "long");
				}
		| ULONG		{ strcpy(rbuf, "\t\t_RSZ = 4;\n\t\t_RET = ");
				  strcpy(rtbuf, "unsigned long");
				}
		| INT		{ strcpy(rbuf, "\t\t_RSZ = 2;\n\t\t_RET = ");
				  strcpy(rtbuf, "int");
				}
		| UINT		{ strcpy(rbuf, "\t\t_RSZ = 2;\n\t\t_RET = ");
				  strcpy(rtbuf, "unsigned");
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

atype:		  VOID		{
				   arg_size = 0;
				   strcpy(atype, "VOID");
				   is_void = 1;
				}
		| WORD		{
				  arg_size = 2;
				  strcpy(atype, "WORD");
				}
		| UWORD		{
				  arg_size = 2;
				  strcpy(atype, "UWORD");
				}
		| INT		{
				  arg_size = 2;
				  strcpy(atype, "int");
				  strcpy(atype2, "WORD");
				}
		| UINT		{
				  arg_size = 2;
				  strcpy(atype, "unsigned");
				  strcpy(atype2, "UWORD");
				}
		| LONG		{
				  arg_size = 4;
				  strcpy(atype, "long");
				  strcpy(atype2, "DWORD");
				}
		| ULONG		{
				  arg_size = 4;
				  strcpy(atype, "unsigned long");
				  strcpy(atype2, "UDWORD");
				}
		| BYTE		{
				  arg_size = 1;
				  strcpy(atype, "BYTE");
				}
		| UBYTE		{
				  arg_size = 1;
				  strcpy(atype, "UBYTE");
				}
		| STRUCT sname	{
				  arg_size = -1;
				  sprintf(atype, "struct %s", $2);
				}
		| tname		{
				  arg_size = -1;
				  sprintf(atype, "%s", $1);
				}
;

rdecls:		rtype decls	{ abuf[0] = 0; }
;

adecls:		  atype decls
		| CONST atype decls	{ is_const = 1; }
;

argsep:		COMMA		{ fin_arg(0); strcat(abuf, ", "); beg_arg(); }

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

static void yyerror(const char* s)
{
    fprintf(stderr, "Parse error: %s\n", s);
    exit(1);
}
