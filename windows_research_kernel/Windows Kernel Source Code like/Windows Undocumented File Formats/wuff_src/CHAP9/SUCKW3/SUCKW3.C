/**********************************************************************
 *
 * PROGRAM: SUCKW3.C
 *
 * PURPOSE: Extracts VxDs from a W3 file.
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 9, W3 and W4 File Formats, from Undocumented Windows File Formats,
 * published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "suckw3.h"

// Pass by the MZ Header
BOOL SkipMZ(FILE *inFile)
{
  MZHEADER  MZHeader;
  
  fread(&MZHeader, sizeof(MZHeader), 1, inFile);
  
  if (MZHeader.MZMagic[0] == 'W' && MZHeader.MZMagic[1] == '3')
  {
    fseek(inFile, 0, SEEK_SET);
    printf("Note: This is an extracted W3 file.\n");
    return TRUE;
  }
  
  if (MZHeader.MZMagic[0] != 'M' || MZHeader.MZMagic[1] != 'Z')
  {
    printf("This is not an executable\n");
    return FALSE;
  }
  
  if (!MZHeader.OtherOff)
  {
    printf("This is a DOS Executable\n");
    return FALSE;
  }

  fseek(inFile, MZHeader.OtherOff, SEEK_SET);
  return TRUE;
}

// List VxDs in the W3 file
void ListW3File(FILE *W3File)
{
  long    W3Start;
  W3HEADER  W3Hdr;
  WORD    i;
  VxDRECORD VxDRec;
  
  W3Start = ftell(W3File);
  
  fread(&W3Hdr, sizeof(W3Hdr), 1, W3File);
  if (W3Hdr.WinVer == 0x30A)
    printf("W3 File for Windows Version 3.1\n\n");
  else if (W3Hdr.WinVer == 0x400)
    printf("W3 File for Windows 95.\n\n");
  
  printf("%u VxDs in this W3 File.\n\n", W3Hdr.NumVxDs);
  
  printf("VxDName    VxDStart       VxDHdrLen\n");
  printf("-----------------------------------\n");
  for (i=0; i<W3Hdr.NumVxDs; i++)
  {
    fread(&VxDRec, sizeof(VxDRec), 1, W3File);
    printf("%-10s 0x%08lX     0x%08lX\n", 
      VxDRec.VxDName, VxDRec.VxDStart, VxDRec.VxDHdrSize);
  }
}

// Extract the VxD
void PullVxD(FILE *W3File, char *VxDName, long VxDStart)
{
  char    OutFile[12];
  FILE    *VxDFile;
  LEHEADER  LEHdr;
  long    Remaining;
  int     ToCopy;
  static char buffer[8192];
  
  fseek(W3File, VxDStart, SEEK_SET);

  strcpy(OutFile, VxDName);
  strcat(OutFile, ".386");
  if ((VxDFile = fopen(OutFile, "wb")) == NULL) 
    printf("Unable to create file %s!\n", OutFile);
  
  fread(&LEHdr, sizeof(LEHdr), 1, W3File);
  Remaining = LEHdr.NonResTable;
  
  // Patch values for Non-Resident Name Table
  LEHdr.NonResSize = strlen(VxDName) * 2 + 2;

  // Patch Data Pages offset
  LEHdr.DataPages -= VxDStart;
  
  // Write the new LE Header
  fwrite(&LEHdr, sizeof(LEHdr), 1, VxDFile);
  Remaining -= sizeof(LEHdr);
  
  // Copy remaining information
  while (Remaining)
  {
    ToCopy = Remaining > 4096 ? 4096 : (int) Remaining;
    fread(buffer, ToCopy, 1, W3File);
    fwrite(buffer, ToCopy, 1, VxDFile);
    Remaining -= ToCopy;
  }
  
  // Patch Non-Resident Name Table itself
  buffer[0]=strlen(VxDName);
  memcpy(&buffer[1], VxDName, strlen(VxDName));
  buffer[strlen(VxDName) + 1] = 0;
  buffer[strlen(VxDName) + 2] = 0;
  buffer[strlen(VxDName) + 3] = buffer[0];
  ToCopy = strlen(VxDName) + 4;
  memcpy(&buffer[ToCopy], VxDName, strlen(VxDName));
  ToCopy += strlen(VxDName);
  buffer[ToCopy] = 0x01;
  buffer[ToCopy + 1] = 0;
  ToCopy+=2;
  
  // Write the Non-Resident Name Table and close file.
  fwrite(buffer, ToCopy, 1, VxDFile);
  fclose(VxDFile);

}

// Find the VxD to "suck" out
void SuckVxD(FILE *W3File, char *VxDName)
{
  long    W3Start;
  W3HEADER  W3Hdr;
  WORD    i;
  VxDRECORD VxDRec;
  
  W3Start = ftell(W3File);
  
  fread(&W3Hdr, sizeof(W3Hdr), 1, W3File);
  
  // Try to find the VxD
  for(i=0; i<W3Hdr.NumVxDs; i++)
  {
    fread(&VxDRec, sizeof(VxDRec), 1, W3File);
    if (!memcmp(VxDRec.VxDName, VxDName, strlen(VxDName)))
    {
      printf("Extracting %s..\n", VxDName);
      PullVxD(W3File, VxDName, VxDRec.VxDStart);
      return;
    }
  }
  
  // Didn't find the VxD;
  printf("VxD %s not found in this W3 File.\n");
}


void Usage(void)
{
  printf("Usage: SUCKW3 W3Name [VxDName]\n\n");
  printf("W3Name   is the name of the W3 executable, probably\n");
  printf("         WIN386.EXE, VMM32.VXD, or VMM32.EXE\n");
  printf("VxDName  is, optionally, the name of the VxD to extract.\n\n");
  printf("Just providing the W3Name will give a directory\n");
  printf("of the contents of the W3 executable.\n");
}

int main(int argc, char *argv[]) 
{

  char  filename[256];
  char  VxDName[9];
  FILE  *W3File;

  if (argc < 2) {
    Usage();
    return EXIT_FAILURE;
  }
  
  strcpy(filename, argv[1]);
  
  if (argc == 3) 
  {
    if (strlen(argv[2]) > 8)
    {
      printf("Invalid VxDName. Must be 8 characters or less.\n");
      return EXIT_FAILURE;
    }
    strcpy(VxDName, argv[2]);
  }
  if (!strchr(filename, '.'))
    strcat(filename, ".EXE");
    
  if ((W3File = fopen(filename, "rb")) == NULL) 
  {
    printf("%s does not exist!\n", filename);
    return EXIT_FAILURE;
  }
  
  if (SkipMZ(W3File)) 
  { 
    if (argc == 2)
    {
      ListW3File(W3File);
    }
    if (argc == 3)
    {
      SuckVxD(W3File, VxDName);
    }
  }
  fclose(W3File);
  return EXIT_SUCCESS;
}

