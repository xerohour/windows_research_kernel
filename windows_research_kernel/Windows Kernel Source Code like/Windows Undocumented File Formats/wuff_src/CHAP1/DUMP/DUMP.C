/**********************************************************************
 *
 * PROGRAM: DUMP.C
 *
 * PURPOSE: Produce a simple hex dump of a file.
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 1, Introduction and Overview, from Undocumented Windows
 * File Formats, published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned char     BYTE;

/***************************************************
  Performs a Hex/ASCII dump of a file.
****************************************************/
void HexDump(FILE *File) {

   char        Buffer[16];
   long        counter, FileSize;
   int         BytesToPrint, Index;

   fseek(File, 0, 2);
   FileSize = ftell(File);
   fseek(File, 0, 0);
   printf("Offset                   Hex Values");
   printf("                           Ascii\n");
   printf("--------------------------------------");
   printf("-----------------------------------\n");

   for (counter = 0; counter < FileSize; counter+=16) {

      printf("0x%08lX: ", counter);
      BytesToPrint = fread(Buffer, 1, 16, File);
      for (Index=0; Index < BytesToPrint; Index++) 
          printf("%02X ", (BYTE) Buffer[Index]);
      for (Index=0; Index < 16-BytesToPrint; Index++) 
          printf("   ");
      for (Index=0; Index < BytesToPrint; Index++) 
          putchar( isprint( Buffer[Index] ) ? Buffer[Index] : '.' );
      putchar('\n');
   }
}

/***************************************************
  Show usage.
****************************************************/
void Usage() {

  printf("Usage:\n");
  printf(" DUMP filename\n\n");
  printf("   filename - Name of file to dump\n");
 
}

/***************************************************
  Open the file and dump it.
****************************************************/
int main(int argc, char *argv[]) {

    char filename[40];
    FILE *File;

    if (argc != 2) {
       Usage();
       return EXIT_FAILURE;
    }

    strcpy(filename, argv[1]);
    _strupr(filename);

    if ((File = fopen(filename, "rb")) == NULL) {
       printf("%s does not exist!", filename);
       return EXIT_FAILURE;
    }

    HexDump(File);
    fclose(File);

    return EXIT_SUCCESS;
}
