/**********************************************************************
 *
 * PROGRAM: DECOMP.C
 *
 * PURPOSE: Decompress a file compressed with Microsoft's COMPRESS.EXE utility.
 * Functionally equivalent to EXPAND.EXE.
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 6, Compression Algorithm and File Formats, from Undocumented Windows
 * File Formats, published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decomp.h"

#define WINSIZE  4096
#define LENGTH(x) ((((x) & 0x0F)) + 3)
#define OFFSET(x1, x2) ((((((x2 & 0xF0) >> 4) * 0x0100) + x1) & 0x0FFF) \
                       + 0x0010)

#define FAKE2REAL_POS(x)   ((x) & (WINSIZE - 1))
#define BITSET(byte, bit)  (((byte) & (1<<bit)) > 0)

/* This is our Compression Window */
char Window[WINSIZE];

/*************************************************
  Decides how many bytes to read, depending on the
  number of bits set in the Bitmap
**************************************************/
int BytesToRead(unsigned char BitMap) {

    int TempSum, counter, c;

    TempSum = 8;
    for (counter = 0; counter < 8; counter ++) {
        c = BITSET(BitMap, counter);
        TempSum += (1 - BITSET(BitMap, counter));
    }

    return TempSum;

} /* BytesToRead - end */


/*************************************************
  Decompresses the data using Microsoft's LZ77
derivative.
**************************************************/ 
void Decompress(FILE *infile, FILE *outfile, long CompSize) {

    unsigned char BitMap, byte1, byte2;
    int Length, counter;
    long Offset, CurrPos=0;

    for (counter = 0; counter < WINSIZE; counter ++)
        Window[counter] = ' ';

    /* Go through until we're done */
    while (CurrPos < CompSize) {

        /* Get BitMap and data following it */
        BitMap = fgetc(infile); 
        if (feof(infile)) return;
        (void) BytesToRead(BitMap);

        /* Go through and decode data */
        for (counter = 0; counter < 8; counter++) {

            /* It's a code, so decode it and copy the data */
            if (!BITSET(BitMap, counter)) {
                byte1 = fgetc(infile); 
                if (feof(infile)) return;
                byte2 = fgetc(infile); 
                Length = LENGTH(byte2);
                Offset = OFFSET(byte1, byte2);

                /* Copy data from 'window' */
                while (Length) {
                    byte1 = Window[FAKE2REAL_POS(Offset)];
                    Window[FAKE2REAL_POS(CurrPos)] = byte1;
                    fputc(byte1, outfile);
                    CurrPos++; 
                    Offset++; 
                    Length--;
                }
            }/* if */

            else {
                byte1 = fgetc(infile);
                Window[FAKE2REAL_POS(CurrPos)] = byte1;
                fputc(byte1, outfile); 
                CurrPos++;
            }
            if (feof(infile)) return;

        } /* for */

    } /* while  */

} /* Decompress - end */


/***************************************************
  Read the header at the beginning of the compressed
  file and verity it is a valid input file.
****************************************************/
void ReadHeader(FILE *infile, FILE *outfile) {

    COMPHEADER CompHeader;
    long       CompSize;

    fseek(infile, 0, SEEK_END);
    CompSize = ftell(infile);
    fseek(infile, 0, SEEK_SET);
    fread(&CompHeader, sizeof(CompHeader), 1, infile);
    if ((CompHeader.Magic1 != MAGIC1) || (CompHeader.Magic2 != MAGIC2)) {
        printf("Fatal Error:\n");
        printf("  Not a valid Compressed file file!\n");
        return;
    }

    printf("Decompressing file from %lu bytes to %lu bytes\n",
          CompSize, CompHeader.DecompSize);

    Decompress(infile, outfile, CompHeader.DecompSize);

    printf("Done!\n");

} /* ReadHeader - end */


/***************************************************
  Show usage.
****************************************************/
void Usage( void ) {

    printf("Usage:\n");
    printf(" DECOMP file1.ext file2.ext\n\n");
    printf("   file1.ext - Name of compressed file\n");
    printf("   file2.ext - Name of decompressed file\n\n");

} /* Usage - end */


/***************************************************
  Open the file and dump it.
****************************************************/
int main(int argc, char *argv[]) {

    char filename[128];
    FILE *infile, *outfile;

    if (argc < 3) {
        Usage();
        return EXIT_FAILURE;
    }

    strcpy(filename, argv[1]);
    if ((infile = fopen(filename, "rb")) == NULL) {
        printf("%s does not exist!", filename);
        return EXIT_FAILURE;
    }

    strcpy(filename, argv[2]);
    if ((outfile=fopen(filename, "wb")) == NULL) {
        printf("Error opening destination file!");
        return(EXIT_FAILURE);
    }

    ReadHeader(infile, outfile);
    fclose(infile);
    fclose(outfile);

    return(EXIT_SUCCESS);

} /* main - end */

/* decomp.c - end */

