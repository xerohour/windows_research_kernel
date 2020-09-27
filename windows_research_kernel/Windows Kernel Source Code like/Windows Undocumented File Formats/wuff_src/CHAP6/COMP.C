/**********************************************************************
 *
 * PROGRAM: COMP.C
 *
 * PURPOSE: Compress a file using something like Microsoft's derivative on LZ77
 * (i.e., it can be uncompressed using Microsoft's EXPAND.EXE)
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
#define MAXLEN   18

#define COMP_CODE(x,y) ((((x-3) & 0x0F) << 8) + (((y - 0x10) & \
                       0x0F00) << 4) + ((y - 0x10) & 0x00FF))

#define LOBYTE(x) ((unsigned char)(x))
#define HIBYTE(x) ((unsigned char)(((unsigned short)(x) >> 8) & 0xFF))

#define DROP_INDEX(x) (x == 0) ? (WINSIZE - 1) : (x - 1)
#define ADD_INDEX(x)  ((x + 1) == WINSIZE) ? 0 : (x + 1)

/* This is our Compression Window */
unsigned char Window[WINSIZE];

/***************************************************
  Set bit number "bit" in byte "byte"
****************************************************/
void BitSet(int bit, char *byte)
{

   short result = 1;

   /* make sure bit range is 0,..,7 */
   if (bit < 0) bit = 0;
   else if (bit > 7) bit = 7;

   while (bit--)
      result *= 2;

   *byte = result | *byte;

} /* BitSet - end */


/*************************************************
  InBetween - detects if lower <= target <= higher
**************************************************/ 
int InBetween(int lower, int higher, int target)
{

  if(higher < lower)
     higher += WINSIZE;
  if((lower <= target) && (target <= higher))
     return 1;
  else
     return 0;

} /* InBetween - end */


/*************************************************
  Force FlagByte and DataBytes to print out
**************************************************/ 
void WriteFlagByte(FILE *outfile)
{

   int index = 0;

   if(FlagCount > 0) {
      DataBytes[DataCount] = '\0';
      fputc(FlagByte, outfile);
      for (; index < DataCount ; ++index)
         fprintf(outfile, "%c", DataBytes[index]);
      DataCount = FlagCount = 0;
      FlagByte = '\0';
      for (index = 0; index < 17; ++index)
         DataBytes[index] = ' ';
   }

} /* WriteFlagByte - end */


/*************************************************
  Check if FlagByte is full and should be printed out
**************************************************/ 
void CheckFlagByte(FILE *outfile)
{

   ++DataCount;
   if(++FlagCount == 8)
     WriteFlagByte( outfile );

} /* CheckFlagByte - end */


/*************************************************
  Saves an uncompressed data byte
**************************************************/ 
void GetNextChar( int *CurrPos, FILE *infile)
{

   unsigned char ch;

   fread(&ch, sizeof(char), 1, infile);
   if (!feof(infile))
      Window[*CurrPos = ADD_INDEX(*CurrPos)] = ch;

} /* GetNextChar - end */


/*************************************************
  Unreads the last character read
**************************************************/ 
void UnreadChar(unsigned char ch, FILE *infile, int *CurrPos, 
                unsigned char ch2)
{

   ungetc( ch, infile);
   Window[*CurrPos] = ch2;
   *CurrPos = DROP_INDEX(*CurrPos);

} /* UnreadChar - end */


/*************************************************
  Saves an uncompressed data byte
**************************************************/ 
void SaveUncompByte(unsigned char ch, FILE *outfile)
{

   BitSet(FlagCount, &FlagByte);
   DataBytes[DataCount] = ch;
   CheckFlagByte( outfile );

} /* SaveUncompByte - end */


/*************************************************
  Compresses the data using Microsoft's LZ77
derivative (Zeck).
**************************************************/ 
void Compress(FILE *infile, FILE *outfile)
{

   int count=0, shifter=0, CurrPos=0;
   int SavePos=0, iCompCode = 0, offset = 0;
   int newPos = 0;
   unsigned char ch;
   int bestcount = 0, bestoffset = 0;
   char oldchars[3];

   FlagCount = 0;
   DataCount = 0;
   FlagByte  = '\0';

   for (count = 0; count < WINSIZE; count ++)
      Window[count] = ' ';

   rewind( infile );

   /* Go through input file until we're done */
   fread(&ch, sizeof(char), 1, infile);
   Window[CurrPos] = ch;
   while (!feof(infile)) {

      /* if less than 3 chars from end, just write out remainder */
      if((InfileSize - ftell(infile)) < 2) {
         SaveUncompByte( Window[CurrPos], outfile);
         GetNextChar( &CurrPos, infile);
         continue;
      } 

      /* Find previous occurrence of character in window */
      for (count = 1, shifter = DROP_INDEX(CurrPos);
         (Window[shifter] != Window[CurrPos]) && (count < WINSIZE);
         ++count, shifter = DROP_INDEX(shifter)) {}

      /* check if char is unique so far in input file */
      if(count == WINSIZE) {
         SaveUncompByte( Window[CurrPos], outfile);
         GetNextChar( &CurrPos, infile);
         continue;
      }
      else {

         /* find out how many characters match */
         SavePos = CurrPos;
         oldchars[2] = oldchars[0] = Window[ADD_INDEX(CurrPos)];
         GetNextChar( &CurrPos, infile);
         for(count = 1, offset=shifter, shifter = ADD_INDEX(shifter);
            (!feof(infile)) && (Window[shifter] == Window[CurrPos]) && 
            (count < MAXLEN);) {
            ++count;
            if(count == 2)
               oldchars[1] = Window[ADD_INDEX(CurrPos)];
            oldchars[2] = Window[ADD_INDEX(CurrPos)];
            GetNextChar( &CurrPos, infile);
            shifter = ADD_INDEX(shifter); 
         }

         /* Since this is the first match, save it as the best so far */
         bestcount  = count;
         bestoffset = offset;

         if(((Window[shifter] != Window[CurrPos]) || (count == MAXLEN)) &&
            (!feof(infile)))
            UnreadChar( Window[CurrPos], infile, &CurrPos, oldchars[2]);

         /* Now find the best match for the string in the window */
         shifter = DROP_INDEX( offset );
         while((shifter != CurrPos) && (bestcount < MAXLEN) &&
            (!InBetween( SavePos, CurrPos, shifter))) {

            for( ; (shifter != CurrPos) && (Window[shifter] != Window[SavePos]);
               shifter = DROP_INDEX(shifter)) {}
            if(shifter == CurrPos)
               continue;
            for(count = 0, offset = shifter, newPos = SavePos;
               (!feof(infile)) && (Window[shifter] == Window[newPos]) &&
               (count < MAXLEN); ++count, newPos = ADD_INDEX(newPos)) {
               if (count >= (bestcount - 1)) {
                  if(count == 1)
                     oldchars[1] = Window[ADD_INDEX(CurrPos)];
                  oldchars[2] = Window[ADD_INDEX(CurrPos)];
                  GetNextChar( &CurrPos, infile );
               }
               shifter = ADD_INDEX(shifter);
            }

            if(((count >= MAXLEN) || ((Window[shifter] != Window[newPos]) && 
               (count >= bestcount))) && (!feof(infile)))
               UnreadChar( Window[CurrPos], infile, &CurrPos, oldchars[2]);

            if(count > bestcount) {
               bestcount = count;
               bestoffset = offset;
            }
            shifter = DROP_INDEX( offset );

         } /* while(shifter != CurrPos) */

         if(!feof(infile))
            GetNextChar( &CurrPos, infile );

         count = bestcount;
         offset = bestoffset;

         /* if count < 3, then not enough chars to compress */
         if (count < 3) {
            SaveUncompByte( Window[SavePos], outfile);
            fseek(infile, ftell(infile) - count, 0);
            Window[SavePos = ADD_INDEX(SavePos)] = oldchars[0];
            CurrPos = DROP_INDEX(CurrPos);
            if (count == 2) {
               Window[ADD_INDEX(SavePos)] = oldchars[1];
               CurrPos = DROP_INDEX(CurrPos);
            }

         }
         else {
            iCompCode = COMP_CODE(count, offset);
            DataBytes[DataCount]   = LOBYTE(iCompCode);
            DataBytes[++DataCount] = HIBYTE(iCompCode);
            CheckFlagByte( outfile );

            if (!feof(infile))
                UnreadChar( Window[CurrPos], infile, &CurrPos, oldchars[2]);
         }

         if((!feof(infile)) && (count <= MAXLEN))
            GetNextChar( &CurrPos, infile);

      }

   } /* while - end */

   WriteFlagByte( outfile );

} /* Compress - end */

/***************************************************
  Write the header that exists at the beginning of 
  every file compressed using MS's Zeck compression
****************************************************/
void WriteHeader(FILE *infile, FILE *outfile)
{

   COMPHEADER CompHeader;

   CompHeader.Magic1 = MAGIC1;
   CompHeader.Magic2 = MAGIC2;
   CompHeader.Is41 = 0x41;
   CompHeader.FileFix = '\0';  /* This stores the original last */
                               /* char. of the input filename   */

   fseek( infile, 0L, 2);
   CompHeader.DecompSize = InfileSize = ftell(infile);
   rewind( infile );

   rewind( outfile);
    
   fwrite(&CompHeader, sizeof(CompHeader), 1, outfile);

   Compress(infile, outfile);

} /* WriteHeader - end */


/***************************************************
  Show usage.
****************************************************/
void Usage( void )
{

   printf("Usage:\n");
   printf(" COMP file1.ext file2.ext\n\n");
   printf("   file1.ext - Name of uncompressed file\n");
   printf("   file2.ext - Name of compressed file\n\n");

} /* Usage - end */


/***************************************************
  Open the input and output files, and call routine
  to compress the data.
****************************************************/
int main(int argc, char *argv[])
{

   char filename[128];
   FILE *infile, *outfile;

   if (argc != 3) {
      Usage();
      return EXIT_FAILURE;
   }

   strcpy(filename, argv[1]);
   if ((infile = fopen(filename, "rb")) == NULL) {
      printf("%s does not exist\n", filename);
      return(EXIT_FAILURE);
   }

   strcpy(filename, argv[2]);
   if ((outfile=fopen(filename, "wb+")) == NULL) {
      printf("Error opening destination file\n");
      return(EXIT_FAILURE);
   }

   WriteHeader(infile, outfile);
   fclose(infile);
   fclose(outfile);

   return(EXIT_SUCCESS);

} /* main - end */

/* comp.c - end */

