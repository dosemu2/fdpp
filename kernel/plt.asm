%macro asmcfunc 1
    global _%1
    _%1:
    nop
%endmacro

%include "plt.inc"
