#
# TURBOC.MAK - kernel copiler options for TURBOC
#

# Use these for Turbo C 2.01

COMPILERPATH=$(TC2_BASE)
COMPILERBIN=$(COMPILERPATH)
CC=$(COMPILERBIN)\tcc
INCLUDEPATH=$(COMPILERPATH)\include
LIBUTIL=$(COMPILERBIN)\tlib
LIBPATH=$(COMPILERPATH)\lib
LIBTERM=  
LIBPLUS=+

CFLAGST=-L$(LIBPATH) -mt -lt -a- -k- -f- -ff- -O -Z -d
CFLAGSC=-L$(LIBPATH) -a- -mc

TARGET=KTC

# used for building the library

CLIB=$(COMPILERPATH)\lib\cs.lib
MATH_EXTRACT=*LDIV *LXMUL *LURSH *LLSH *LRSH
MATH_INSERT=+LDIV +LXMUL  +LURSH +LLSH +LRSH

#
# heavy stuff - building the kernel
# Compiler and Options for Borland C++
# ------------------------------------
#
#  -zAname       � � Code class
#  -zBname       � � BSS class
#  -zCname       � � Code segment
#  -zDname       � � BSS segment
#  -zEname       � � Far segment
#  -zFname       � � Far class
#  -zGname       � � BSS group
#  -zHname       � � Far group
#  -zPname       � � Code group
#  -zRname       � � Data segment
#  -zSname       � � Data group
#  -zTname       � � Data class
#  -zX           ��� Use default name for "X"

#
# ALLCFLAGS specified by turbo.cfg and config.mak
#
ALLCFLAGS=$(TARGETOPT) $(ALLCFLAGS)
INITCFLAGS=$(ALLCFLAGS) -zAINIT -zCINIT_TEXT -zDIB -zRID -zTID -zPI_GROUP -zBIB -zGI_GROUP -zSI_GROUP
CFLAGS=$(ALLCFLAGS) -zAHMA -zCHMA_TEXT
