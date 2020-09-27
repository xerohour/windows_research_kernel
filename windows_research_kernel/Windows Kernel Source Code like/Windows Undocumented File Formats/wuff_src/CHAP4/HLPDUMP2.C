/**********************************************************************
 *
 * PROGRAM: HLPDUMP2.C
 *
 * PURPOSE: A dump program that lets you view internal WinHelp files
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 4, Windows Help File Format, from Undocumented Windows
 * File Formats, published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

#define MEM_DEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include "winhelp.h"
#include "hlpdump2.h"

#define HLP_DEBUG 1

#define CHECK_SIGNATURE(is, shouldbe) \
         {if (is != shouldbe) { \
            printf("Signature should be %x, but is %x\n"); \
            return;} }
             

/* Tells us if a particular bit is set or not */
#define BITSET(bitmap, bit) \
        ((bitmap & (1 << bit)) ? 1 : 0 )

/* Sum of set bits in a byte + 8 */
#define BYTESTOREAD(bitmap) \
        ( BITSET(bitmap, 0) + BITSET(bitmap, 1) + \
          BITSET(bitmap, 2) + BITSET(bitmap, 3) + \
          BITSET(bitmap, 4) + BITSET(bitmap, 5) + \
          BITSET(bitmap, 6) + BITSET(bitmap, 7) + 8 )


/***************************************************
  Loads the HFSFileHeader
****************************************************/

long LoadHeader(FILE *HelpFile) 
{
  HFSFILEHEADER fileHeader;
  
  fread(&fileHeader, sizeof(HFSFILEHEADER), 1, HelpFile);
  
#ifdef HLP_DEBUG
  printf("DEBUG -> LoadHeader()\n");
  printf("File plus Header: %ld\n", fileHeader.FilePlusHeader);
  printf("File size: %ld\n", fileHeader.FileSize);
  printf("File type: 0x%02x\n\n", fileHeader.FileType);
#endif
  
  return fileHeader.FileSize;
}


/*************************************************
  Decompresses the data using Microsoft's LZ77
derivative (called Zeck Compression)
**************************************************/ 

long Decompress(FILE *HelpFile, long CompSize, char *Buffer) {

long InBytes = 0;        /* How many bytes read in                    */
long OutBytes = 0;       /* How many bytes written out                */
BYTE BitMap, Set[16];    /* Bitmap and bytes associated with it       */
long NumToRead;          /* Number of bytes to read for next group    */
long counter, Index;     /* Going through next 8-16 codes or chars    */
long Length, Distance;   /* Code length and distance back in 'window' */
char *CurrPos;           /* Where we are at any given moment          */
char *CodePtr;           /* Pointer to back-up in LZ77 'window'       */


  CurrPos = Buffer;

  /* Go through until we're done */
  while (InBytes < CompSize) {

    /* Get BitMap and data following it */
    BitMap = (BYTE) fgetc(HelpFile);
    NumToRead = BYTESTOREAD(BitMap);

    /* If we're trying to read more than we've 
       got left, only read what we have left.  */
    NumToRead = (CompSize - InBytes) < NumToRead ? 
                                    CompSize-InBytes : NumToRead;

    fread(Set, 1, (int) NumToRead, HelpFile);    
    InBytes += NumToRead + 1;

    /* Go through and decode data */
    for (counter = 0, Index = 0; counter < 8; counter++) {

      /* It's a code, so decode it and copy the data */
      if (BITSET(BitMap, counter)) {
        Length = ((Set[Index+1] & 0xF0) >> 4) + 3;
        Distance = ((Set[Index+1] & 0x0F) << 8) + Set[Index] + 1;
        CodePtr = CurrPos - Distance;

        /* Copy data from 'window' */
        while (Length) {
          *CurrPos++ = *CodePtr++;
          OutBytes++;
          Length--;
        } 
        Index += 2;
      } 
      else {
        *CurrPos++ = Set[Index++];
        OutBytes++;
      }
    } /* for */
  } /* while  */
  return OutBytes;
} 


/***************************************************
  Performs a Hex/ASCII dump of an HFS file.
****************************************************/

void HexDumpData(FILE *HelpFile, long fileSize) 
{
  char           Buffer[16];
  long           counter;
  long           BytesToPrint, Index;

  printf("Offset                   Hex Values                           Ascii\n");
  printf("-------------------------------------------------------------------------\n");

  for (counter = 0; counter < fileSize; counter+=16) {

    printf("0x%08lX: ", counter);

    /* If this is the last line, how many bytes are in it? */
    BytesToPrint = ((fileSize - counter) > 16) ? 16 : (fileSize - counter);
    fread(Buffer, BytesToPrint, 1, HelpFile);

    /* Dump Hex */
    for (Index=0; Index < BytesToPrint; Index++)
      printf("%02X ", (BYTE) Buffer[Index]);

    /* If last line, fill in blanks */
    for (Index=0; Index < 16-BytesToPrint; Index++) 
      printf("   ");

    /* Dump Ascii */
    for (Index=0; Index < BytesToPrint; Index++) 
      putchar( isprint( Buffer[Index] ) ? Buffer[Index] : '.' );

    putchar('\n');
  }

  free(Buffer);
}


/***************************************************
  Performs a Hex/ASCII dump of an HFS file.
****************************************************/

void HexDumpFile(FILE *HelpFile, long FileStart) 
{
  long           fileSize;

  fseek(HelpFile, FileStart, SEEK_SET);

  fileSize = LoadHeader(HelpFile);  
  
  printf("File Size: 0x%08lX\n\n", fileSize);
  
  HexDumpData(HelpFile, fileSize);
}

/***************************************************
  Dumps the |SYSTEM info
****************************************************/

void SystemDump(FILE *HelpFile, long FileStart) 
{

  char            HelpFileTitle[33];
  SYSTEMREC       SystemRec;
  long            CurrentLocation;
  struct tm       *TimeRec;
  SECWINDOW       *SWin;     /* Secondary Window record */
  long            fileSize;
  SYSTEMHEADER    SysHeader;

  fseek(HelpFile, FileStart, SEEK_SET);

  fileSize = LoadHeader(HelpFile);

  fread(&SysHeader, sizeof(SysHeader), 1, HelpFile);
  printf("|SYSTEM Dump\n\n\n");

  /* Figure out Version and Revision */
  if (SysHeader.Revision == 0x15) printf("HC.EXE  3.10 Help Compiler used\n");
  else if (SysHeader.Revision == 0x21) printf("HCW.EXE. 4.00 or MVC.EXE\n");

  printf("\nVersion: %d\nRevision: %d\n", SysHeader.Version, SysHeader.Revision);
  printf("Flag: 0x%04X  - ",SysHeader.Flags);

  /* Determine compression, if any. */
  if (SysHeader.Flags == NO_COMPRESSION) printf("No compression\n");
  else if (SysHeader.Flags & COMPRESSION_HIGH) printf("High Compression\n");
  else printf("Unknown Compression: 0x%02x\n", SysHeader.Flags);

  TimeRec=localtime(&SysHeader.GenDate);
  printf("Help File Generated: %s", asctime(TimeRec));

  /* If 3.0 get title */
  CurrentLocation=12;
  if (SysHeader.Revision == 0x0F) {
     fgets(HelpFileTitle, 33, HelpFile);
     printf("Help File Title: %s\n", HelpFileTitle);
  }

  /* Else, get 3.1 System records */
  else {
    while (CurrentLocation < fileSize) {

      /* Read in system record and SystemRec data */
      fread(&SystemRec, 4, 1, HelpFile);
      SystemRec.RData = malloc(SystemRec.DataSize);
      if (SystemRec.RData == NULL)
      {
        printf("Allocation of SystemRec.RData failed.");
        return;
      }
      fread(SystemRec.RData, SystemRec.DataSize, 1, HelpFile);
      CurrentLocation=CurrentLocation+4+SystemRec.DataSize;

      switch(SystemRec.RecordType) 
      {
        case 0x0001:  printf("Help File Title: %s\n", SystemRec.RData);
          break;

        case 0x0002:  printf("Copyright Notice: %s\n", SystemRec.RData);
          break;

        case 0x0003:  printf("Contents ID: 0x%04X\n", (long) *SystemRec.RData);
          break;

        case 0x0004:  printf("Macro Data: %s\n",SystemRec.RData);
          break;

        case 0x0005:  printf("Icon in System record\n");
          break;

        case 0x0006:  printf("\nSecondary window:\n");
                      SWin = (SECWINDOW *)SystemRec.RData;
                      printf("Flag: %d\n", SWin->Flags);
                      if (SWin->Flags & WSYSFLAG_TYPE) 
                        printf("Type: %s\n", SWin->Type);
                      if (SWin->Flags & WSYSFLAG_NAME) 
                        printf("Name: %s\n", SWin->Name);
                      if (SWin->Flags & WSYSFLAG_CAPTION) 
                        printf("Caption: %s\n", SWin->Caption);
                      if (SWin->Flags & WSYSFLAG_X) 
                        printf("X: %d\n", SWin->X);
                      if (SWin->Flags & WSYSFLAG_Y) 
                        printf("Y: %d\n", SWin->Y);
                      if (SWin->Flags & WSYSFLAG_WIDTH)
                        printf("Width: %d\n", SWin->Width);
                      if (SWin->Flags & WSYSFLAG_HEIGHT)
                        printf("Height: %d\n", SWin->Height);
                      if (SWin->Flags & WSYSFLAG_MAXIMIZE) 
                        printf("Maximize Flag: %d\n", SWin->Maximize);
                      if (SWin->Flags & WSYSFLAG_RGB) 
                        printf("RGB Foreground Colors Set\n");
                      if (SWin->Flags & WSYSFLAG_RGBNSR)
                        printf("RGB For Non-Scrollable Region Set\n");
                      if (SWin->Flags & WSYSFLAG_TOP) 
                        printf("Secondary Window is always On Top\n");
          break;

        case 0x0008:  printf("Citation: %s\n", SystemRec.RData);
          break;

        case 0x000A:  printf("\nContents File: %s\n\n",SystemRec.RData);
          break;                                                 
          
        default:      printf("\nUnknown record type: 0x%04X\n",SystemRec.RecordType);
                      /* Back-up and hex-dump the data */
                      fseek(HelpFile, 
                            ftell(HelpFile) - SystemRec.DataSize, 
                            SEEK_SET);
                      HexDumpData(HelpFile, SystemRec.DataSize);
      } /* switch */
     
      free(SystemRec.RData);

    } /* while */
  } /* else */
} /* SysDump */


/***************************************************
  Dumps the |FONT file
****************************************************/

void FontDump(FILE *HelpFile, long FileStart) 
{
  FONTHEADER      FontHdr;
  FONTDESCRIPTOR  FontDesc;
  char            AFont[32];
  long            FontStart, CurrLoc;
  long            fileSize;
  long            counter;
  long            NameLen;

  /* Go to the FONT file and get the headers */
  fseek(HelpFile, FileStart, SEEK_SET);
  fileSize = LoadHeader(HelpFile);

  fread(&FontHdr, sizeof(FontHdr), 1, HelpFile);

  printf("|FONTS\n\n Number Fonts: %d\n",FontHdr.NumFonts);
  printf("Font #  -  Font Name\n");


  /* Font names are 20 chars prior to Winhelp 4.0 */
  /* In WinHelp 4.0, they are 32 characters.      */
  if (SysInfo.Revision == 0x15)
  {
    NameLen = 20;
  }
  else
  {
    NameLen = 32;
  }

  /* Keep track of start of fonts */
  FontStart = ftell(HelpFile);
  for (counter = 0; counter < (long) FontHdr.NumFonts; counter++) 
  {
    fread(AFont, NameLen, 1, HelpFile);
    printf(" %3d    -  %s\n", counter, AFont);
  }

  /* Go to Font Descriptors. Don't actually need this, because we're
     there, but wanted to show how to get there using the offset.    */
  fseek(HelpFile, FontStart + (long)(FontHdr.DescriptorsOffset) - sizeof(FontHdr), SEEK_SET); 
  printf("\nNum Font Descriptors: %d\n", FontHdr.NumDescriptors);
  printf("Default Descriptor: %d\n\n", FontHdr.DefDescriptor);
  printf("Attributes: n=none  b=bold  i=ital  u=undr  s=strkout  d=dblundr  C=smallcaps\n\n");
  printf("Font Name                        PointSize  Family   FG RGB      BG RGB   Attr\n");
  printf("------------------------------------------------------------------------------\n");

  for (counter = 0; counter < (long) FontHdr.NumDescriptors; counter++) {
    fread(&FontDesc, sizeof(FontDesc), 1, HelpFile);
    CurrLoc = ftell(HelpFile);
    fseek(HelpFile, FontStart + (NameLen * FontDesc.FontName), SEEK_SET);
    fread(AFont, NameLen, 1, HelpFile);
    fseek(HelpFile, CurrLoc, SEEK_SET);
      
    /* write out info on Font descriptor */
    printf("%-32s    %4.1f    ", AFont, (float)(FontDesc.HalfPoints / 2));
    switch (FontDesc.FontFamily) {
      case FAM_MODERN: printf("Modern");
                          break;

      case FAM_ROMAN:  printf("Roman ");
                       break;

      case FAM_SWISS:  printf("Swiss ");
                       break;

      case FAM_SCRIPT: printf("Script");
                       break;

      case FAM_DECOR:  printf("Decor ");
                       break;

      default:         printf("0X%02X ", FontDesc.FontFamily);
                       break;
    } /* Switch */
    printf(" 0X%08lX  ",RGB(FontDesc.SRRGB.rgbRed,
                            FontDesc.SRRGB.rgbGreen,
                            FontDesc.SRRGB.rgbBlue));
    printf("0X%08lX    ",RGB(FontDesc.NSRRGB.rgbRed,
                             FontDesc.NSRRGB.rgbGreen,
                             FontDesc.NSRRGB.rgbBlue));

    if (FontDesc.Attributes == 0) putchar('n');
    if (FontDesc.Attributes & FONT_BOLD) putchar('b');
    if (FontDesc.Attributes & FONT_ITAL) putchar('i');
    if (FontDesc.Attributes & FONT_UNDR) putchar('u');
    if (FontDesc.Attributes & FONT_STRK) putchar('s');
    if (FontDesc.Attributes & FONT_DBUN) putchar('d');
    if (FontDesc.Attributes & FONT_SMCP) putchar('C');
    printf("\n");

#ifdef HLP_DEBUG
    printf("Unknown = %d\n", FontDesc.Unknown);
#endif
  }
}


/***************************************************
  Dumps the |CONTEXT file
****************************************************/

void ContextDump(FILE *HelpFile, long FileStart) 
{
  long            count;
  long            CurrPage, FirstPageLoc;
  long            TopicOffset, HashValue;
  BTREEHEADER     BTreeHdr;
  BTREELEAFHEADER CurrNode;

  /* Go to the TTLBTREE file and get the headers */
  fseek(HelpFile, FileStart, SEEK_SET);
  LoadHeader(HelpFile);
  fread(&BTreeHdr, sizeof(BTreeHdr), 1, HelpFile);

  /* Save the current location */
  FirstPageLoc = ftell(HelpFile);
   
  fseek(HelpFile, 
        FirstPageLoc+(BTreeHdr.RootPage * BTreeHdr.PageSize), 
        SEEK_SET);

  printf("# Context Hash Values in |CONTEXT %lu\n\n", BTreeHdr.TotalBtreeEntries);
  CurrPage = BTreeHdr.FirstLeaf;

  do 
  {
    fseek(HelpFile, 
          FirstPageLoc+(CurrPage * BTreeHdr.PageSize), 
          SEEK_SET);
    fread(&CurrNode, 8, 1, HelpFile);
    for(count = 1; count <= CurrNode.NEntries; count++) 
    {
      fread(&HashValue, sizeof(HashValue), 1, HelpFile);
      fread(&TopicOffset, sizeof(TopicOffset), 1, HelpFile);
      printf("Topic Offset:0x%08lX  Hash Value: 0x%08lX\n", TopicOffset, HashValue);
    }
    CurrPage = CurrNode.NextPage;
  } while(CurrPage != -1);
}


/***************************************************
  Prints a Phrase from the Phrase table.
****************************************************/
void PrintPhrase(long PhraseNum) 
{
  short *Offsets;
  char  *p;

  p = Phrases+Offsets[PhraseNum];
  while (p < Phrases + Offsets[PhraseNum + 1])
    putchar(*p++);
}


/***************************************************
  Dumps the Keyword B-Tree
****************************************************/

void KWBTreeDump(FILE *HelpFile, long FileStart) 
{
  char            Keyword[80], c;
  long            count, Index;
  long            CurrPage, FirstPageLoc;
  long            KeywordOffset;
  long            KeywordCount;
  BTREEHEADER     BTreeHdr;
  BTREELEAFHEADER CurrNode;

  /* Go to the KWBTREE file and get the headers */
  fseek(HelpFile, FileStart, SEEK_SET);
  LoadHeader(HelpFile);
  fread(&BTreeHdr, sizeof(BTreeHdr), 1, HelpFile);

  /* Save the current location */
  FirstPageLoc = ftell(HelpFile);
   
  fseek(HelpFile, 
        FirstPageLoc+(BTreeHdr.RootPage * BTreeHdr.PageSize), 
        SEEK_SET);

  printf("# Keywords - %lu\n\n", BTreeHdr.TotalBtreeEntries);
  CurrPage = BTreeHdr.FirstLeaf;

  do 
  {
    fseek(HelpFile, 
          FirstPageLoc+(CurrPage * BTreeHdr.PageSize), 
          SEEK_SET);
    fread(&CurrNode, 8, 1, HelpFile);
    for(count = 1; count <= CurrNode.NEntries; count++) 
    {
      Index = 0;
      while(c = (char) fgetc(HelpFile))
        Keyword[Index++] = c;
        
      Keyword[Index] = 0;
      fread(&KeywordCount, sizeof(KeywordCount), 1, HelpFile);
      fread(&KeywordOffset, sizeof(KeywordOffset), 1, HelpFile);

      printf("Offset: 0x%08lX  Count: %d  Keyword: %s\n", 
                          KeywordOffset, KeywordCount, Keyword);
    }
    CurrPage = CurrNode.NextPage;
  } while(CurrPage != -1);
}


/***************************************************
  Dumps the Keyword Data file
****************************************************/

void KWDataDump(FILE *HelpFile, long FileStart) 
{
  long fileSize;
  long nIndex;
  long Offset;
  
  /* Go to the KWDATA file and get the headers */
  fseek(HelpFile, FileStart, SEEK_SET);
  fileSize = LoadHeader(HelpFile);
          
  printf("Dumping Keyword Data File\n\n");
  /* Go through all keyword offsets (fileSize / 4) */
  for (nIndex = 0; nIndex < (fileSize / sizeof(Offset)); nIndex++)
  {
    fread(&Offset, sizeof(Offset), 1, HelpFile);
    printf("Index: %d   Offset: 0x%08lX\n", nIndex, Offset);
  }
}


/***************************************************
  Dumps the Keyword Map file
****************************************************/

void KWMapDump(FILE *HelpFile, long FileStart) 
{
  long      fileSize;
  long      nIndex;
  WORD      nKWMaps;
  KWMAPREC  kwMap;
  
  /* Go to the KWMAP file and get the headers */
  fseek(HelpFile, FileStart, SEEK_SET);
  fileSize = LoadHeader(HelpFile);
          
  printf("Dumping Keyword Map\n\n");
  
  fread(&nKWMaps, sizeof(nKWMaps), 1, HelpFile);
  
  /* Go through all keyword offsets (fileSize / 4) */
  for (nIndex = 0; nIndex < (int) nKWMaps; nIndex++)
  {
    fread(&kwMap, sizeof(KWMAPREC), 1, HelpFile);
    printf("Index: %d   First Keyword 0x%08lX    Leaf Page#: %05u\n", 
                          nIndex, kwMap.FirstRec, kwMap.PageNum);
  }
}


/***************************************************
  Dumps the Topic Titles B-Tree
****************************************************/

void TTLDump(FILE *HelpFile, long FileStart) 
{
  char            Title[80], c;
  long            count, Index;
  long            CurrPage, FirstPageLoc, TopicOffset;
  BTREEHEADER     BTreeHdr;
  BTREELEAFHEADER CurrNode;

  /* Go to the TTLBTREE file and get the headers */
  fseek(HelpFile, FileStart, SEEK_SET);
  LoadHeader(HelpFile);
  fread(&BTreeHdr, sizeof(BTreeHdr), 1, HelpFile);

  /* Save the current location */
  FirstPageLoc = ftell(HelpFile);
   
  fseek(HelpFile, 
        FirstPageLoc+(BTreeHdr.RootPage * BTreeHdr.PageSize), 
        SEEK_SET);

  printf("# Titles in |TTLBTREE %lu\n\n", BTreeHdr.TotalBtreeEntries);
  CurrPage = BTreeHdr.FirstLeaf;

  do 
  {
    fseek(HelpFile, 
          FirstPageLoc+(CurrPage * BTreeHdr.PageSize), 
          SEEK_SET);
    fread(&CurrNode, 8, 1, HelpFile);
    for(count = 1; count <= CurrNode.NEntries; count++) 
    {
      fread(&TopicOffset, sizeof(TopicOffset), 1, HelpFile);
      Index = 0;
      while(c = (char) fgetc(HelpFile))
        Title[Index++] = c;
        
      Title[Index] = 0;

      printf("Topic Offset:0x%08lX  Title: %s\n", TopicOffset, Title);
    }
    CurrPage = CurrNode.NextPage;

  } while(CurrPage != -1);
}


/***************************************************
  Dumps the |Phrases file
****************************************************/

void PhrasesDump(FILE *HelpFile, long FileStart) 
{
  long  nOuterIndex, nInnerIndex;
  WORD  start, len;
  
  PhrasesLoad(HelpFile, FileStart);

  printf("Phrase#     Phrase\n");
  for (nOuterIndex = 0; nOuterIndex < NumPhrases; nOuterIndex++)
  {
    start = (WORD) PhrOffsets[nOuterIndex] ;
    len = PhrOffsets[nOuterIndex + 1] - start;
    printf(" %5d     ", nOuterIndex + 1);
    for (nInnerIndex = 0; nInnerIndex < (int) len; nInnerIndex++)
    {
      printf("%c", (Phrases[start + nInnerIndex]));
    }
    printf("\n");
  }
}


/***************************************************
  Loads the |SYSTEM info
****************************************************/

int SysLoad(FILE *HelpFile, long FileStart) 
{
  fseek(HelpFile, FileStart, SEEK_SET);

  LoadHeader(HelpFile);
  
  fread(&SysInfo, sizeof(SYSTEMHEADER), 1, HelpFile);
  
#ifdef HLP_DEBUG
  printf("DEBUG -> SysLoad()\n");
  printf("Magic: 0x%02x\n", SysInfo.Magic);
  printf("Version: 0x%02x\n", SysInfo.Version);
  printf("Revision: 0x%02x\n", SysInfo.Revision);
  printf("Flags: 0x%04x\n\n", SysInfo.Flags);
#endif

  if (SysInfo.Magic != SYS_MAGIC)
  {
    return 0;
  }
  
  return 1;
}


/***************************************************
  Loads the compression phrases
****************************************************/

void PhrasesLoad(FILE *HelpFile, long FileStart) 
{
  PHRASEHEADER  phraseHeader;
  long          FileSize;
  long          DeCompSize;
  
  fseek(HelpFile, FileStart, SEEK_SET);
  FileSize = LoadHeader(HelpFile);

  fread(&phraseHeader, sizeof(phraseHeader), 1, HelpFile);
  
  if (SysInfo.Flags != NO_COMPRESSION)
  {
    if ((PhrOffsets = malloc(phraseHeader.PhrasesSize + 
                            (phraseHeader.NumPhrases + 1) * 2 + 10)) == NULL)
    {
      printf("Unable to allocate space for Phrases.\n");
      return;
    }

    /* Assign Phrases to  where the comrpessed phrases are */   
    Phrases = (char*) PhrOffsets + fread(PhrOffsets, 
                                         2, 
                                         phraseHeader.NumPhrases + 1, 
                                         HelpFile);

    DeCompSize = Decompress(HelpFile,
                            FileSize - (sizeof(phraseHeader) +
                            2 * (phraseHeader.NumPhrases + 1)),
                            Phrases);
    if (DeCompSize != (long) phraseHeader.PhrasesSize)
    {
      printf("Warning, Phrases did not decompress to the proper size.\n");
    }
  }
  else
  {
    if ((PhrOffsets = malloc(phraseHeader.PhrasesSize + 
                            (phraseHeader.NumPhrases + 1) * 2 + 10)) == NULL)
    {
      printf("Unable to allocate space for Phrases.\n");
      return;
    }
    /* Back up four bytes if phrases aren't compressed */
    /* because PhrasesSize field doesn't exist.        */
    fseek(HelpFile, -4, SEEK_CUR);
    fread(PhrOffsets, FileSize - 4, 1, HelpFile);
  }
  /* Reset Phrases to be equal to PhrOffsets */
  Phrases = (char *) PhrOffsets;  
  NumPhrases = phraseHeader.NumPhrases;
}


/***************************************************
  Finds an HFS File by traversing the HFS b-tree
  Note: This is the only place I actually traverse
  the b-tree instead of cycling through the leaf
  pages. I put this in specifically to show how to
  traverse the b-tree, since speed isn't a real
  concern for HelpDump.
****************************************************/

char FindFile(FILE *HelpFile, char* filename, long* offset)
{
  BTREEHEADER       HFSHeader;
  BTREEINDEXHEADER* HFSIndexHeader;
  BTREELEAFHEADER*  HFSLeafHeader;
  long              HFSStart;
  short*            pNextPage;
  char*             buffer;
  char*             currPtr;
  long              nKeys, nFiles;
  char              found = 0;
  long              currLevel = 1;

  /* Go to the HFS and read the header. */
  fseek(HelpFile, HelpHeader.HFSLoc, SEEK_SET);
  LoadHeader(HelpFile);
  fread(&HFSHeader, sizeof(HFSHeader), 1, HelpFile);

  /* Allocate space for read buffer */
  buffer = malloc(HFSHeader.PageSize);
  if (buffer == NULL)
  {
    printf("Unable to allocate space for buffer.\n");
    return found;
  }
  HFSIndexHeader = (BTREEINDEXHEADER*) buffer;

  HFSStart = ftell(HelpFile);
  
  /* Advance to root page */
  fseek(HelpFile, 
        (HFSHeader.RootPage * HFSHeader.PageSize) + HFSStart, 
        SEEK_SET);
      
  /* If there's only one page, then it must be a leaf */
  if (HFSHeader.TotalPages > 1)
  {
    /* Traverse b-tree looking for the key for the leaf page */  
    while (!found)
    {
      /* Read in the page */
      fread(buffer, HFSHeader.PageSize, 1, HelpFile);
      currPtr = buffer + sizeof(BTREEINDEXHEADER);
      pNextPage = (int *) currPtr;

      currPtr += sizeof(int);
      
      /* Go through all keys in the page */
      for (nKeys = 0; nKeys < HFSIndexHeader->NEntries; nKeys++)
      {
        /* If filename is less than key, this is our page. */
        if (strcmp(filename, currPtr) < 0)
        {
          break;
        }
        else
        {
          /* Advance to the next page# */
          while (*currPtr)
            currPtr++;
          currPtr++;
          pNextPage = (int *) currPtr;
          currPtr += sizeof(int);
        }
      }
      
      /* Advance to next page */
      fseek(HelpFile, 
            (*pNextPage * HFSHeader.PageSize) + HFSStart, 
            SEEK_SET);
      
      /* If this is the last index page */
      /* then pNextPage points to a     */
      /* leaf page                      */
      if (currLevel == HFSHeader.nLevels - 1)
      {
        found = 1;
      }
      currLevel++;
    }
  }
  
  fread(buffer, HFSHeader.PageSize, 1, HelpFile);
  HFSLeafHeader = (BTREELEAFHEADER*) buffer;
  currPtr = buffer + sizeof(BTREELEAFHEADER);


  found = 0;
  
  /* Loop through all files in this page */
  for (nFiles = 0; nFiles < HFSLeafHeader->NEntries; nFiles++)
  {
    if (strcmp(filename, currPtr))
    {  
      /* Advance to the file offset */
      while (*currPtr)
        currPtr++;
      
      /* Move past the null and file offset */
      /* to next file                       */
      currPtr += 5;    
    }
    else
    {
      /* Save the offset of the file */
      while (*currPtr)
        currPtr++;
      currPtr++;
      
      *offset = (long) *((long*) currPtr);
      found = 1;
      break;
    }
  }

  /* TRUE if file was found, FALSE if it wasn't */
  return found;
}


/***************************************************
  DumpFile
****************************************************/

void DumpFile(FILE *HelpFile)
{
  long fileOffset;
  char fileName[255];
  
  /* For many files we need to know information about    */
  /* The system, so we'll load the system data info here */
  /* if it's available                                   */
  SysLoaded = 0;
  strcpy(fileName, "|SYSTEM");
  if (FindFile(HelpFile, fileName, &fileOffset))
  {
     SysLoaded = (char) SysLoad(HelpFile, fileOffset);
  }

  /* If it's not a version 3 help file */
  /* then we can't handle it.          */
  if (SysInfo.Version != 0x03)
  {
    printf("Warning: Not a version 3 help file. Version is %d.\n",
           SysInfo.Version);
  }
  
  /* If it's version 3, but not revision   */
  /* 0x15 or 0x21, we also can't handle it */
  if (SysInfo.Revision != 0x15 && SysInfo.Revision != 0x21)
  {
    printf("Revision not 0x15 or 0x21. Revision is 0x%02x.\n",
           SysInfo.Revision);
  }
  
  /* If we're reading the |TOPIC file, then we need to */
  /* pre-load the phrases from the |Phrases file       */  
  if (!strcmp(HFSFileToRead, "|TOPIC") ||
      !strcmp(HFSFileToRead, "TOPIC"))
  {
    strcpy(fileName, "|Phrases");
    if (FindFile(HelpFile, fileName, &fileOffset))
    {
      PhrasesLoad(HelpFile, fileOffset);
    }
  }

  strcpy(fileName, HFSFileToRead);
  if (!FindFile(HelpFile, fileName, &fileOffset))
  {
    /* Append a "|" character to the beginning  */
    /* of the filename and try to find the file */
    strcpy(fileName, "|");
    strcat(fileName, HFSFileToRead);
    if (!FindFile(HelpFile, fileName, &fileOffset))
    {
      /* File not found */
      printf("Error: Unable to find HFS file %s or %s\n",
             HFSFileToRead, fileName);
      return;
    }
  }

  if (!ForceHex)
  {  
    if (!strcmp(fileName, "|SYSTEM"))
      SystemDump(HelpFile, fileOffset);
    else if (!strcmp(fileName, "|TTLBTREE"))
      TTLDump(HelpFile, fileOffset);
    else if (!strcmp(fileName, "|CONTEXT"))
      ContextDump(HelpFile, fileOffset);
    else if (!strcmp(fileName, "|FONT"))
      FontDump(HelpFile, fileOffset);
    else if (!strcmp(fileName, "|KWBTREE"))
      KWBTreeDump(HelpFile, fileOffset);
    else if (!strcmp(fileName, "|AWBTREE"))
      KWBTreeDump(HelpFile, fileOffset);
    else if (!strcmp(fileName, "|KWDATA"))
      KWDataDump(HelpFile, fileOffset);
    else if (!strcmp(fileName, "|AWDATA"))
      KWDataDump(HelpFile, fileOffset);
    else if (!strcmp(fileName, "|KWMAP"))
      KWMapDump(HelpFile, fileOffset);
    else if (!strcmp(fileName, "|Phrases"))
      PhrasesDump(HelpFile, fileOffset);
    else 
      HexDumpFile(HelpFile, fileOffset);
  }
  else
  {
    HexDumpFile(HelpFile, fileOffset);
  }  
}

/***************************************************
  List out all the HFS files in a .HLP file
****************************************************/

void ListFiles(FILE *HelpFile)
{
  BTREEHEADER       HFSHeader;
  BTREELEAFHEADER*  HFSLeafHeader;
  long              HFSStart;
  char*             buffer;
  char*             currPtr;
  long              nIndex, nFiles;
  long*             offset;

  /* Go to the HFS and read the header. */
  fseek(HelpFile, HelpHeader.HFSLoc, SEEK_SET);
  LoadHeader(HelpFile);
  fread(&HFSHeader, sizeof(HFSHeader), 1, HelpFile);

  CHECK_SIGNATURE(HFSHeader.Signature, 0x293B);

#ifdef HLP_DEBUG  
  printf("DEBUG -> ListFiles()\n");
  printf("B-Tree Page Size %d\n", HFSHeader.PageSize);
  printf("B-Tree First Leaf %d\n", HFSHeader.FirstLeaf);
  printf("B-Tree Num. Levels %d\n", HFSHeader.nLevels);
  printf("B-Tree Total Pages %d\n", HFSHeader.TotalPages);
  printf("B-Tree Total # Entries %ld\n", HFSHeader.TotalBtreeEntries);
#endif 
  /* Start of the HFS b-tree pages */
  HFSStart = ftell(HelpFile);

  /* Go to the first leaf page of the HFS */  
  fseek(HelpFile, HFSHeader.FirstLeaf * HFSHeader.PageSize, SEEK_CUR);

  /* Allocate space for read buffer and read first page */
  buffer = malloc(HFSHeader.PageSize);
  if (buffer == NULL)
  {
    printf("Unable to allocate space for buffer.\n");
    return;
  }
  HFSLeafHeader = (BTREELEAFHEADER*) buffer;
  
  printf("\nHFS Filename                 Offset\n");
  printf("-----------------------------------\n");
  
  /* Loop through all HFS leaf pages */
  for (nIndex = 0; nIndex < HFSHeader.TotalPages; nIndex++)
  {
    fread(buffer, HFSHeader.PageSize, 1, HelpFile);

    currPtr = buffer + sizeof(BTREELEAFHEADER);

    /* Loop through all files in this page */
    for (nFiles = 0; nFiles < HFSLeafHeader->NEntries; nFiles++)
    {

      /* Print filename */
      printf("%-30s", currPtr);
    
      /* Advance to next filename */
      while (*currPtr)
        currPtr++;
      currPtr++;
      
      offset = (long*) currPtr;
      /* print offset to file */
      printf("0x%08lX\n", *offset);

      /* Move past the file offset to next file */
      currPtr += 4;    
    }
  }
}

/***************************************************
  Check to make sure it's a help file. Then
  either dump the HFS directory or dump an HFS
  file.
****************************************************/

void HelpDump(FILE *HelpFile) 
{
  fread(&HelpHeader, sizeof(HelpHeader), 1, HelpFile);
  if (HelpHeader.MagicNumber != HF_MAGIC) 
  {
    printf("Fatal Error:\n");
    printf("  Not a valid WinHelp file!\n");
    return;
  }

  if (ReadHFSFile) 
     DumpFile(HelpFile);
  else
     ListFiles(HelpFile);

}


/***************************************************
  Show usage
****************************************************/

void Usage()
{
  printf("HLPDUMP2  (version 2.0 of Help Dump)\n");
  printf("By Pete Davis and Mike Wallace               Copyright 1997\n\n");
  printf("Usage: HLPDUMP2 helpfile[.hlp] [hfsfilename] [/H]\n\n");
  printf("where:\n");
  printf("  helpfile    - name of .HLP/.GID/.ANN/.BMK file to open\n");
  printf("  hfsfilename - name of HFS file to read\n");
  printf("  /H          - force a hex dump\n\n");
  printf("note: Do not include the pipe '|' character in the hfsfilename.\n");
}


/***************************************************
 Entry point to HLPDUMP2
****************************************************/

int main(int argc, char *argv[])
{
  char filename[_MAX_PATH];
  FILE *HelpFile;

  if (argc < 2) 
  {
    Usage();
    return EXIT_FAILURE;
  }
  ReadHFSFile = 0;
  if (argc >= 3) 
  {
    strcpy(HFSFileToRead, argv[2]);
    ReadHFSFile = 1;
  }

  /* Are we forcing a hex dump? */
  ForceHex = 0;
  if (argc == 4)
  {

  if (stricmp(argv[3], "/H"))
    {
      printf("Error: Argument 3 unrecognized\n");
      return EXIT_FAILURE;
    }
    ForceHex = 1;

  }
  strcpy(filename, argv[1]);
  strupr(filename);
  if (!strchr(filename, '.')) 
    strcat(filename, ".HLP");

  if ((HelpFile = fopen(filename, "rb")) == NULL) 
  {
    printf("%s does not exist!", filename);
    return EXIT_FAILURE;
  }

  printf("Dumping %s\n\n", filename);
  HelpDump(HelpFile);
  fclose(HelpFile);

  return EXIT_SUCCESS;
  
}
