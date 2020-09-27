/**********************************************************************
 *
 * PROGRAM: W4DECOMP.H
 *
 * PURPOSE: Decompresses a W4 file into a W3 file.
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 9, W3 and W4 File Formats, from Undocumented Windows File Formats,
 * published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

typedef unsigned char  BOOL;
typedef unsigned short WORD;
typedef unsigned char  BYTE;
typedef unsigned long  DWORD;

#define FALSE 0;
#define TRUE 1;

typedef struct tagMZHEADER
{
  int   Magic;
  char  Stuff[58];
  long  OtherOff;
} MZHEADER;

typedef struct tagW4HEADER {
  WORD  Magic;
  WORD  Unknown1;
  WORD  ChunkSize;
  WORD  ChunkCount;
  WORD  DS;
  WORD  Unknown2;
  WORD  Unknown3;
  WORD  Unknown4;
} W4HEADER;

#define MZMAGIC 0x5A4D
#define W4MAGIC 0x3457

