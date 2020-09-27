/**********************************************************************
 *
 * PROGRAM: HLPDUMP2.H
 *
 * PURPOSE: Header file for HLPDUMP2.C
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 4, Windows Help File Format, from Undocumented Windows
 * File Formats, published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

/***********************************************************
  Global Variables
************************************************************/

int          ReadHFSFile;
int          ForceHex;

/* While it appears HFS filenames don't exceed 8           */
/* it's safer to assume that the limit is higher.          */
char         HFSFileToRead[255];

char         SysLoaded;
HELPHEADER   HelpHeader;
SYSTEMHEADER SysInfo;
int*         PhrOffsets;
char*        Phrases;
int          NumPhrases;

/***********************************************************
  Function Prototypes
************************************************************/

long LoadHeader(FILE *);
long Decompress(FILE *, long, char *);
void HexDumpData(FILE *, long);
void HexDumpFile(FILE *, long);
void SystemDump(FILE *, long);
void FontDump(FILE *, long); 
void ContextDump(FILE *, long);
void PrintPhrase(long);
void KWBTreeDump(FILE *, long);
void KWDataDump(FILE *, long);
void KWMapDump(FILE *, long);
void TTLDump(FILE *, long);
void PhrasesDump(FILE *, long);
int SysLoad(FILE *, long);
void PhrasesLoad(FILE *, long);
char FindFile(FILE *, char*, long*);
void DumpFile(FILE *);
void ListFiles(FILE *);
void HelpDump(FILE *);
void Usage();
int  main(int, char**);

