/*
    DYNDATA.C
    
    this serves as a placeholder in the near data segment

    alll data herein goes to special segment 
        DYN_DATA AFTER BSS, but immediately before HMA_TEXT
*/    
#include "portab.h"
#include "init-mod.h"
#include "dyndata.h"

struct DynS Dyn;
