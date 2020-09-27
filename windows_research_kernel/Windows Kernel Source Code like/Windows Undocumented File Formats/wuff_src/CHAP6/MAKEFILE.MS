############################################################
#
# Makefile for comp.c and decomp.c using the Microsoft compiler/linker
# Chap. 6, Undocumented Windows File Formats, published by
# R&D Books, an imprint of Miller Freeman, Inc.
#
# Copyright 1997, Mike Wallace and Pete Davis
#
# Environment variable requirements:
#
#   1) 'cl' and 'link' must be in your path
#   2) INCLUDE and LIB = paths to inc/ and lib/ subdirectories
#                        under the compiler's directory
#
# Compile the program with: nmake /f makefile.ms
#
############################################################

PROG1   = COMP
PROG2   = DECOMP
HEADERS = DECOMP.H

all: $(PROG1).EXE $(PROG2).EXE

$(PROG1).EXE:	$(PROG1).OBJ
		link /nologo $(PROG1),,$(PROG1)/li/CO,mlibce,,

$(PROG1).OBJ:	$(PROG1).C $(HEADERS)
		cl /nologo /W4 /AM /Zdpei /Od /c $(PROG1).C

$(PROG2).EXE:	$(PROG2).OBJ
		link /nologo $(PROG2),,$(PROG2)/li/CO,mlibce,,

$(PROG2).OBJ:	$(PROG2).C $(HEADERS)
		cl /nologo /W4 /AM /Zdpei /Od /c $(PROG2).C

