/**********************************************************************
 *
 * PROGRAM: SHGDUMP.C
 *
 * PURPOSE: Dump the hotspot information for a SHG file
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 3, Segmented Hypergraphic (.SHG) File Format, 
 * from Undocumented Windows File Formats, published by R&D Books,
 * an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shededit.h"

/* Flags an incorrect value for known fields */
#define CheckUVal(a,b) \
       {if (a != b) {  \
       printf("UINT: Not a 0x%04X?\n\n 0x%04x\n",b, a); exit(1); } }

#define CheckBVal(a,b) \
       {if (a != b) {  \
       printf("BYTE: Not a 0x%02X?\n\n 0x%02x\n",b, a); exit(1); } }

#define ReadString(f, s) { char *p = (char *)(s); \
                  while ((*p++ = fgetc(f)) != 0) ; *p = 0; }


/* Reads in a BYTE. If it's odd, it reads in a WORD. Returns 1/2 value */
WORD ReadWordVal(FILE *SHGFile)
{
  BYTE a, b;
  
  b = 0;
  fread(&a, sizeof(a), 1, SHGFile);
  if (a % 2) fread(&b, sizeof(b), 1, SHGFile);
  return (WORD) ((WORD)b*256 + a) /2;
}


/* Reads in a WORD. If it's odd, it reads in a DWORD. Returns 1/2 value */
DWORD ReadDWordVal(FILE *SHGFile)
{
  WORD a, b;
  
  b = 0;
  fread(&a, sizeof(a), 1, SHGFile);
  if (a % 2) fread(&b, sizeof(b), 1, SHGFile);
  return (DWORD) ((DWORD)b*65536 + a) /2;
}


/* Dumps HotSpot data from .SHG file */
void DumpHotSpots(FILE *SHGFile)
{
  HOTSPOTHEADER HSHead;
  HOTSPOTRECORD HSRec;
  WORD i, NumMacs = 0;
  char AString[128];
  long fileLoc;
  
  fread(&HSHead, sizeof(HSHead), 1, SHGFile);
  
  for(i = 1; i <= HSHead.hhNumHS; i++)
  {
    fileLoc = ftell(SHGFile);
    fread(&HSRec, sizeof(HSRec), 1, SHGFile);
    printf("Hot Spot %u (offset %ld) - ", i, fileLoc);
    switch(HSRec.hrType)
    {
      case HS_INVISJUMP:  printf("Invisible Jump\n");  break;
      case HS_VISJUMP:    printf("Visible Jump\n");    break;
      case HS_INVISPOPUP: printf("Invisible Popup\n"); break;
      case HS_VISPOPUP:   printf("Visible Popup\n");   break;

      case HS_INVISMACRO:
        printf("Invisible Macro\n");
        NumMacs++;
        break;

      case HS_VISMACRO:
        printf("Visible Macro\n"); 
        NumMacs++; 
        break;

      default:
        printf("Invalid Record Type\n"); 
        return;
    }
  }
  
  /* Print out the list of macros */
  for (i = 1; i <= NumMacs; i++)
  {
    ReadString(SHGFile, AString); 
    printf("Macro %u> %s\n", i, AString);
  }
  
  printf("\n");
  
  /* Print out the list of hotspot IDs */
  for(i = 1; i <= HSHead.hhNumHS; i++)
  {
    ReadString(SHGFile, AString);
    printf("Hotspot ID #%u> %s\n", i, AString);
    ReadString(SHGFile, AString);
    printf("Context String-> %s\n\n", AString);
  }
}


/* Reads in a bitmap header. Doubles the 
   size of certain fields when necessary. */
void ReadBMHeader(FILE *SHGFile, SHGBITMAPHEADER *SHGBM)
{
  /* Read first three fields unmodified */
  fread(&(SHGBM->sbIsZero), 1, 1, SHGFile);
  SHGBM->sbDPI = ReadWordVal(SHGFile);
  fread(&(SHGBM->sbTwoHund), 2, 1, SHGFile);
  SHGBM->sbNumBits = ReadWordVal(SHGFile);
  SHGBM->sbWidth = ReadDWordVal(SHGFile); 
  SHGBM->sbHeight = ReadDWordVal(SHGFile); 
  SHGBM->sbNumQuads = ReadDWordVal(SHGFile); 
  fread(&(SHGBM->sbNumImp), 2, 1, SHGFile);
  SHGBM->sbCmpSize = ReadDWordVal(SHGFile); 
  SHGBM->sbSizeHS = ReadDWordVal(SHGFile); 
  fread(&(SHGBM->sbunk1), 4, 1, SHGFile);
  fread(&(SHGBM->sbSizeImage), 4, 1, SHGFile);
}


/* Dump bitmap data in .SHG file */
void BitMapDump(FILE *SHGFile)
{
  RGBQUAD         AnRGBQuad;
  DWORD           i=0L;
  SHGBITMAPHEADER SHGBM;
  
  ReadBMHeader(SHGFile, &SHGBM);
  
  /* Read past bitmap quad info */
  for(i = 0L; i < SHGBM.sbNumQuads; i++)
  {
    fread(&AnRGBQuad, sizeof(AnRGBQuad), 1, SHGFile);
  }
  
  /* Jump past the image to the hotspot data */
  fseek(SHGFile, SHGBM.sbCmpSize, SEEK_CUR);
  
  if (SHGBM.sbSizeHS)
    DumpHotSpots(SHGFile);
  else
    printf("No Hot Spot data for this Bitmap.\n");
}


/* Reads in a metafile header. Doubles the
   size of certain fields when necessary. */
void ReadWMHeader(FILE *SHGFile, SHGMETAFILEHEADER *SHGWM)
{
  /* Read first two fields unmodified */
  fread(&(SHGWM->smXWidth), 4, 1, SHGFile);
  SHGWM->smUncSize = ReadDWordVal(SHGFile);
  SHGWM->smCmpSize = ReadDWordVal(SHGFile);
  SHGWM->smSizeHS  = ReadDWordVal(SHGFile); 
  fread(&(SHGWM->smUnk1), 4, 1, SHGFile);
  SHGWM->smSizeImage = ReadDWordVal(SHGFile);
}


/* Dump the .WMF data from the .SHG file */
void WMFDump(FILE *SHGFile, BOOL bCompUsed)
{
  SHGMETAFILEHEADER SHGWM;
  
  ReadWMHeader(SHGFile, &SHGWM);
  
  /* Jump to the hot spot information (how far we jump */
  /* is based on whether the image is compressed) */
  if (bCompUsed)
    fseek(SHGFile, SHGWM.smCmpSize, SEEK_CUR);
  else
    fseek(SHGFile, SHGWM.smUncSize, SEEK_CUR);
  
  if (SHGWM.smSizeHS)
    DumpHotSpots(SHGFile);
  else
    printf("No Hot Spot Data for this metafile.\n");
}


/* Dumps the .SHG file */
void SHGDump(FILE *SHGFile)
{
  SHGFILEHEADER SHGHead;
  SHGIMAGEHEADER SHGImage;
  WORD i;
  BOOL bCompUsed;
  
  /* Read in first 4 bytes */
  fread(&SHGHead, 4, 1, SHGFile);
  printf("Number of images = %d\n", SHGHead.sfNumObjects);
  SHGHead.sfObjectOff=malloc(4*SHGHead.sfNumObjects);
  fread(SHGHead.sfObjectOff, 4, SHGHead.sfNumObjects, SHGFile);
  
  /* Make sure it is an .SHG file */
  if (strncmp(SHGHead.sfType, "lp", 2))
  {
    printf("Invalid .SHG or .MRB file! \n\nType: %c%c\n", SHGHead.sfType[0],
           SHGHead.sfType[1]);
    exit(1);
  }
  
  for (i = 0; i < SHGHead.sfNumObjects; i++)
  {
    /* Jump to Image Header and read it in */
    fseek(SHGFile, SHGHead.sfObjectOff[i], SEEK_SET);
    fread(&SHGImage, sizeof(SHGImage), 1, SHGFile);
    
    if (SHGImage.siImageType == IT_BMP)
      printf("\nFile is a BITMAP using ");
    else
      printf("\nFile is a METAFILE using ");
    
    if (SHGImage.siCompression == IC_NONE)
      printf("no compression.\n");
    else if (SHGImage.siCompression == IC_RLE)
      printf("RLE compression.\n");
    else if (SHGImage.siCompression == IC_LZ77)
      printf("LZ77 compression.\n");
    else if (SHGImage.siCompression == IC_BOTH)
      printf("RLE and LZ77 compression.\n");
    else
      printf("unknown compression.\n");
    
    bCompUsed = (SHGImage.siCompression == IC_NONE) ? FALSE : TRUE;
    
    if (SHGImage.siImageType == IT_BMP)
      BitMapDump(SHGFile);
    else if (SHGImage.siImageType == IT_WMF)
      WMFDump(SHGFile, bCompUsed);
    else {
      printf("Unknown sfImageType value.\n");
      exit(1);
    }
  }
  
  free(SHGHead.sfObjectOff);
}


/* Show usage for SHGDUMP */
void Usage(void)
{
  printf("Usage:\n");
  printf(" SHGDUMP shgfile[.shg] \n");
  printf("   shgfile  - Name of .SHG file\n");
}


/* main routine */
int main(int argc, char *argv[])
{
  char inputFile[20];
  FILE *SHGFile;
  
  /* Check if the program was invoked correctly */
  if (argc < 2) {
    Usage();
    return EXIT_FAILURE;
  }
  
  /* Save the input filename */
  strcpy(inputFile, argv[1]);
  
  /* If no extension in the input filename, assume .shg */
  if (0 == strchr(inputFile, '.'))
    strcat(inputFile, ".SHG");
  
  /* Check that the input file exists */
  if ((SHGFile = fopen(inputFile, "rb")) == NULL)
  {
    printf("%s does not exist!", inputFile);
    return EXIT_FAILURE;
  }
  
  /* Dump the hotspot information */
  SHGDump(SHGFile);
  
  /* Close the file and exit */
  fclose(SHGFile);
  return EXIT_SUCCESS;
}
