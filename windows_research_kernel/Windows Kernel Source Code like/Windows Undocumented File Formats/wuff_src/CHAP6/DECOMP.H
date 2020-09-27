/**********************************************************************
 *
 * PROGRAM: DECOMP.H
 *
 * PURPOSE: Header information for compressing/decompressing files using
 * Microsoft's LZ77 derivative.
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 6, Compression Algorithm and File Formats, from Undocumented Windows
 * File Formats, published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

typedef struct tagCOMPHEADER {
	long    Magic1;
	long    Magic2;
	char    Is41;        /* 0x41                          */
	char    FileFix;     /* Character saved for -r option */
	long    DecompSize;
} COMPHEADER;

/* Both of these numbers are constants */
/* in the LZEXPAND Library             */

/* First magic# "SZDD" */
#define MAGIC1 0x44445A53

/* Second magic# */
#define MAGIC2 0x3327F088

/* Define some global variables */
unsigned char FlagByte;       /* Byte at the start of each block of compressed data */
unsigned char DataBytes[17];  /* Block of compressed data */
int  DataCount=0,    /* Number of chars in current DataBytes[] */
     FlagCount=0;    /* Number of items in FlagByte */
long InfileSize=0L;  /* Size of input file */

