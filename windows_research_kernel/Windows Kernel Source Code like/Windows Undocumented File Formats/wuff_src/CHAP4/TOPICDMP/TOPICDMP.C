/**********************************************************************
 *
 * PROGRAM: TOPICDMP.C
 *
 * PURPOSE: Dump the topic file from a Windows .HLP or .MVB file
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 4, Windows Help File Format, from Undocumented Windows
 * File Formats, published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <limits.h>

#pragma pack(1)   /* Make sure we get byte alignment */
#include "whstruct.h"

HELPHEADER        HelpHeader;        /* Header for Help file.       */
WHIFSBTREEHEADER  WHIFSHeader;       /* WHIFS Header record         */
int               WHIFSLeafOne = -1; /* First WHIFS Leaf Node       */
long              FirstPageLoc;      /* Used by macros for b-trees  */
char              *PhrasesPtr; 
int               Compressed;        /* Is there compression?       */

#define MSG(s)              { puts(s); return; }
#define FAIL(s)             { puts(s); exit(1); }

#define GET_STRING(f, s) \
	{ char *p = (char *)(s); while (*p++ = fgetc(f)) ; *p = 0; }

#define BIT_SET(map, bit)   (((map) & (1 << (bit))) ? 1 : 0)

// Finds the first leaf in the WHIFS B-Tree
void WHIFSGetFirstLeaf(FILE *HelpFile) {
    int               CurrLevel = 1; /* Current Level in B-Tree */
    BTREEINDEXHEADER  CurrNode;      /* Current Node in B-Tree  */
    int               NextPage = 0;  /* Next Page to go to      */

    /* Go to the beginning of WHIFS B-Tree */
    fseek(HelpFile, HelpHeader.WHIFS, SEEK_SET);
    fread(&WHIFSHeader, sizeof(WHIFSHeader), 1, HelpFile);
    FirstPageLoc = HelpHeader.WHIFS + sizeof(WHIFSHeader);
    GotoWHIFSPage(WHIFSHeader.RootPage);  // macro in WHSTRUCT.H

    /* Find First Leaf */
    while (CurrLevel < WHIFSHeader.NLevels) {
       fread(&CurrNode, sizeof(CurrNode), 1, HelpFile);

       /* Next Page is conveniently the first byte of the page */
       fread(&NextPage, sizeof(int), 1, HelpFile);
       GotoWHIFSPage(NextPage);
       CurrLevel++;
    }
    /* First Leaf page is here */
    WHIFSLeafOne = NextPage;
}

// Get a WHIFS file by file number; returns offset and filename
void GetFile(FILE *HelpFile, DWORD Number, long *Offset, char *Name) {
    BTREENODEHEADER CurrentNode;      
    DWORD           CurrPage, counter = 0;
    char            c, TempFile[19];
    
    /* Skip pages we don't need */
    CurrentNode.NextPage = WHIFSLeafOne;
    do {
        CurrPage = CurrentNode.NextPage;
        GotoWHIFSPage(CurrPage);
        fread(&CurrentNode, sizeof(CurrentNode), 1, HelpFile);
        counter += CurrentNode.NEntries;
    } while (counter < Number);

    for (counter -= CurrentNode.NEntries; counter <= Number; counter++) {
        GET_STRING(HelpFile, TempFile);
        fread(Offset, sizeof(long), 1, HelpFile);
    }
    strcpy(Name, TempFile);
}

// Get SysHeader to see if compression used on help file
void SysLoad(FILE *HelpFile, long FileStart) {
   SYSTEMHEADER    SysHeader;
   FILEHEADER      FileHdr;
   fseek(HelpFile, FileStart, SEEK_SET);
   fread(&FileHdr, sizeof(FileHdr), 1, HelpFile);
   fread(&SysHeader, sizeof(SysHeader), 1, HelpFile);
   if (SysHeader.Revision != 21)
       FAIL("Sorry, TOPICDMP only works with Windows 3.1 help files");
   Compressed = (SysHeader.Flags & COMPRESSION_310) ||
                (SysHeader.Flags & COMPRESSION_UNKN);
}

// Decides how many bytes to read, depending on number of bits set
int BytesToRead(BYTE BitMap) {
    int TempSum, counter;
    TempSum = 8;
    for (counter = 0; counter < 8; counter ++)
       TempSum += BIT_SET(BitMap, counter);
    return TempSum;
}

// Decompresses the data using Microsoft's LZ77 derivative.
long Decompress(FILE *HelpFile, long CompSize, char *Buffer) {
   long InBytes = 0;        /* How many bytes read in                    */
   WORD OutBytes = 0;       /* How many bytes written out                */
   BYTE BitMap, Set[16];    /* Bitmap and bytes associated with it       */
   long NumToRead;          /* Number of bytes to read for next group    */
   int  counter, Index;     /* Going through next 8-16 codes or chars    */
   int  Length, Distance;   /* Code length and distance back in 'window' */
   char *CurrPos;           /* Where we are at any given moment          */
   char *CodePtr;           /* Pointer to back-up in LZ77 'window'       */

   CurrPos = Buffer;
   while (InBytes < CompSize) {
      BitMap = (BYTE) fgetc(HelpFile);
      NumToRead = BytesToRead(BitMap);

      if ((CompSize - InBytes) < NumToRead) 
          NumToRead = CompSize - InBytes;   // only read what we have left
      fread(Set, 1, (int) NumToRead, HelpFile);    
      InBytes += NumToRead + 1;

      /* Go through and decode data */
      for (counter = 0, Index = 0; counter < 8; counter++) {
         /* It's a code, so decode it and copy the data */
         if (BIT_SET(BitMap, counter)) {
            Length = ((Set[Index+1] & 0xF0) >> 4) + 3;
            Distance = (256 * (Set[Index+1] & 0x0F)) + Set[Index] + 1;
            CodePtr = CurrPos - Distance;   // ptr into decompress window
            while (Length)
               { *CurrPos++ = *CodePtr++; OutBytes++; Length--; } 
            Index += 2;  /* codes are 2 bytes */
         }
         else 
            { *CurrPos++ = Set[Index++]; OutBytes++; }
      }
   }
   return OutBytes;
} 
   
// Prints a Phrase from the Phrase table
void PrintPhrase(char *Phrases, int PhraseNum) {
    int *Offsets = (int *)Phrases;
    char *p = Phrases+Offsets[PhraseNum];
    while (p < Phrases + Offsets[PhraseNum + 1])
        { putchar(*p); p++; }
}

// Build up a table of phrases
void PhrasesLoad(FILE *HelpFile, long FileStart) {
   FILEHEADER      FileHdr;
   PHRASEHDR       PhraseHdr;
   int             *Offsets;
   char            *Phrases;
   long            DeCompSize;

   /* Go to the phrases file and get the headers */
   fseek(HelpFile, FileStart, SEEK_SET);
   fread(&FileHdr, sizeof(FileHdr), 1, HelpFile);
   fread(&PhraseHdr, sizeof(PhraseHdr), 1, HelpFile);

   /* Allocate space and decompress if it's compressed, else read in. */
   if (Compressed) {
      if ((Offsets = malloc((unsigned) (PhraseHdr.PhrasesSize + 
          (PhraseHdr.NumPhrases + 1) * 2))) == NULL)
        MSG("No room to decompress |Phrases");
      Phrases = Offsets + fread(Offsets,2,PhraseHdr.NumPhrases+1, HelpFile);
      DeCompSize = Decompress(HelpFile, (long)FileHdr.FileSize - 
          (sizeof(PhraseHdr) + 2 * (PhraseHdr.NumPhrases+1)), Phrases);
      if (DeCompSize != PhraseHdr.PhrasesSize) {
         printf("\n");
      }
   }
   else {
      if (!(Offsets=malloc((unsigned)(FileHdr.FileSize-sizeof(PhraseHdr)))))
         MSG("No room to decompress |Phrases");
      /* Backup 4 bytes for uncompressed Phrases (no PhrasesSize) */
      fseek(HelpFile, -4, SEEK_CUR);
      fread(Offsets, (unsigned) (FileHdr.FileSize - 4), 1, HelpFile);
   }
   PhrasesPtr = Phrases = (char *) Offsets;
}

/* Because the topic file is broken into 4k blocks, we'll have to handle
all the reads.  The idea is to filter out the TOPICBLOCKHEADERs and
do any decompression that needs doing. */
long TopicRead(BYTE *Dest, long NumBytes, FILE *HelpFile) {
   static long        CurrBlockLoc = 0;   /* Where we are in the block  */
   static BYTE        *DCmpBlock = NULL;  /* Block of uncompressed data */
   static long        DecompSize;         /* Size of block after decomp */
   static long        TopicStart, BlkNum; /* Start of |TOPIC file       */
   long               BytesLeft;          /* # Bytes left to return     */
   TOPICBLOCKHEADER   BlockHeader;
   TOPICLINK          *TempLink;
   long               EndOffset;

   /* If NumBytes = 0, then we're done and need to free memory */
   if (NumBytes == -1) { free(DCmpBlock); return 0; }

   if (!DCmpBlock) {
      if (Compressed) {
         if (! (DCmpBlock = malloc((unsigned) (4 * TopicBlockSize))))
             FAIL("Not enough memory to decompress |TOPIC file");
         TopicStart = ftell(HelpFile);
         BlkNum = 0;
      }
      else if (! (DCmpBlock = malloc((unsigned) TopicBlockSize)))
          FAIL("Not enough memory to handle |TOPIC file");
      DecompSize = 0;   /* Set initial size to 0 */
      /* Don't really need first block header, so get it out of the way */
      fread(&BlockHeader, sizeof(BlockHeader), 1, HelpFile);
   }

   BytesLeft = NumBytes;
   while (BytesLeft) {
      if (DecompSize == CurrBlockLoc) {
         BlkNum++;

         if (Compressed) {
            DecompSize = Decompress(HelpFile, (long)TopicBlockSize-1, 
				(char *)DCmpBlock);
            /* Align ourselves at next 4k block */
            fseek(HelpFile, TopicStart + (4096L * BlkNum), SEEK_SET);
         }
         else
            DecompSize=fread(DCmpBlock,1,(unsigned) TopicBlockSize, HelpFile);

         CurrBlockLoc = 0;
         fread(&BlockHeader, sizeof(BlockHeader), 1, HelpFile);

         // Get offset of last topic link. (Don't need block #, hence 3FFFh)
         EndOffset = BlockHeader.LastTopicLink & 0x3FFF;
         TempLink = (TOPICLINK*)(DCmpBlock + EndOffset-sizeof(BlockHeader));

         /* Actual end of the data (Don't include header) */
         EndOffset += (TempLink->BlockSize - sizeof(BlockHeader));

         // If end shorter than topic block use it; else topic block full
         if (EndOffset > DecompSize) {
             /* Adjust DecompSize if crossing 4k boundary */
             EndOffset = TempLink->BlockSize-((TempLink->NextBlock) & 0x3FFF);
             DecompSize = (BlockHeader.LastTopicLink & 0x3FFF) + EndOffset;
         }
         else DecompSize = EndOffset;
     } /* If */

     *(Dest++) = *(DCmpBlock + (CurrBlockLoc++) );
      BytesLeft--;
   } /* While (BytesLeft) */
   return NumBytes;
}

// Displays a string from a topic link record. Checks for Phrase
// replacement and non-printable chars
void TopicStringPrint(char *String, long Length) {
   BYTE            Byte1, Byte2;
   int             CurChar, PhraseNum;
   long            counter;

   for (counter = 0; counter < Length; counter++) {
      CurChar = * ((char *) (String + counter));

      /* Check for Phrase replacement! */
      if ((CurChar > 0) && (CurChar < 10)) {
         Byte1 = (BYTE) CurChar;
         counter++;
         CurChar = * ((char *) (String + counter));
         Byte2 = (BYTE) CurChar;
         PhraseNum = (256 * (Byte1 - 1) + Byte2);

         /* If there's a remainder, we have a space after the phrase */
         PrintPhrase(PhrasesPtr, PhraseNum / 2);
         if (PhraseNum % 2) putchar(' ');
      }
      else if (isprint(CurChar)) putchar(CurChar);
      else putchar(' ');	// could do newline for 0x00 0x00
   }
}

// Dump |TOPIC file, doing decompression and phrase substitution
void TopicDump(FILE *HelpFile, long FileStart) {
   FILEHEADER      FileHdr;
   TOPICHEADER     *TopicHdr;
   TOPICLINK       TopicLink;

   /* Go to the TOPIC file and get the headers */
   fseek(HelpFile, FileStart, SEEK_SET);
   fread(&FileHdr, sizeof(FileHdr), 1, HelpFile);

   do {
      TopicRead((BYTE *) &TopicLink, sizeof(TopicLink) - 4, HelpFile);

      if (Compressed)
         TopicLink.DataLen2 = TopicLink.BlockSize - TopicLink.DataLen1;

      TopicLink.LinkData1=(BYTE *) malloc((unsigned)(TopicLink.DataLen1-21));
      if(!TopicLink.LinkData1)
          MSG("Error allocating TopicLink.LinkData1");
      TopicRead(TopicLink.LinkData1, TopicLink.DataLen1 - 21, HelpFile);
      if (TopicLink.DataLen2 > 0) {
          TopicLink.LinkData2=(BYTE*)malloc((unsigned)(TopicLink.DataLen2+1));
          if(!TopicLink.LinkData2)
             MSG("Error allocating TopicLink.LinkData2");
          TopicRead(TopicLink.LinkData2, TopicLink.DataLen2, HelpFile);
      }

      /* Display a Topic Header record */
      if (TopicLink.RecordType == TL_TOPICHDR) {
         TopicHdr = (TOPICHEADER *)TopicLink.LinkData1;
         printf("================ Topic Block Data ====================\n");
         printf("Topic#: %ld - ", TopicHdr->TopicNum);

         if (TopicLink.DataLen2 > 0)
            TopicStringPrint(TopicLink.LinkData2, (long) TopicLink.DataLen2);
         else printf("\n");
      }

      /* Show a 'text' type record. */
      else if (TopicLink.RecordType == TL_DISPLAY) {
         printf("-- Topic Link Data\n");
         TopicStringPrint(TopicLink.LinkData2, (long) TopicLink.DataLen2);
      }
      printf("\n\n");
      free(TopicLink.LinkData1);
      if (TopicLink.DataLen2 > 0) free(TopicLink.LinkData2);
   } while(TopicLink.NextBlock != -1);
}

void DumpFile(FILE *HelpFile) {
    long    FileOffset, PhraseOffset, TopicOffset;
    DWORD   i;
    char    FileName[32];

    fread(&HelpHeader, sizeof(HelpHeader), 1, HelpFile);
    if (HelpHeader.MagicNumber != 0x35F3FL)
        MSG("Fatal Error:  Not a valid WinHelp file");
    WHIFSGetFirstLeaf(HelpFile);
    TopicOffset = PhraseOffset = 0;

    for (i=0; i<WHIFSHeader.TotalWHIFSEntries; i++) {
       GetFile(HelpFile, i, &FileOffset, FileName);
       if (! strcmp(FileName, "|SYSTEM")) SysLoad(HelpFile, FileOffset);
       else if (! strcmp(FileName, "|Phrases")) PhraseOffset = FileOffset;
       else if (! strcmp(FileName, "|TOPIC")) TopicOffset = FileOffset;
       }
       if (PhraseOffset) PhrasesLoad(HelpFile, PhraseOffset);
       if (TopicOffset) TopicDump(HelpFile, TopicOffset);
       else MSG("No Topic file found!");
}

int main(int argc, char *argv[]) {
    char filename[40];
    FILE *HelpFile;

    if (argc < 2) { 
       printf("Usage: TOPICDMP helpfile[.hlp]\n\n");
       printf("   helpfile      - Name of help file (.HLP or .MVB)\n\n");
       return EXIT_FAILURE;
       }

    if (! strchr(strcpy(filename, strupr(argv[1])), '.'))
       strcat(filename, ".HLP");

    if ((HelpFile = fopen(filename, "rb")) == NULL) {
       printf("Can't open %s!", filename);
       return EXIT_FAILURE;
    }

    DumpFile(HelpFile);
    fclose(HelpFile);
    return EXIT_SUCCESS;
}

