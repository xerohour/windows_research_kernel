############################################################
#
# Makefile for topicdmp.c using the Microsoft compiler/linker
# Chap. 4, Undocumented Windows File Formats, published by
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

PROG    = TOPICDMP
HEADERS = WHSTRUCT.H

$(PROG).EXE:  $(PROG).OBJ
    link /nologo $(PROG),,$(PROG)/li/CO,mlibce,,

$(PROG).OBJ:  $(PROG).C $(HEADERS)
    cl /nologo /W4 /AM /Zdpei /Od /c $(PROG).C

