/**********************************************************************
 *
 * PROGRAM: SHEDEDIT.H
 *
 * PURPOSE: Header file for SHGDUMP.C
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 3, Segmented Hypergraphic (.SHG) File Format, 
 * from Undocumented Windows File Formats, published by R&D Books,
 * an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

/* Structures for .SHG files */

/* SHG File Header */
typedef struct tagSHGFILEHEADER
{
  char  sfType[2];    /* Must be 'lp' or 0x706C     */
  WORD  sfNumObjects; /* Number of objects in file  */
  DWORD *sfObjectOff; /* Offsets to objects in file */
}
SHGFILEHEADER;

/* SHG Image Header */
typedef struct tagSHGIMAGEHEADER
{
  BYTE  siImageType;      /* 0x06=.BMP  0x08=.WMF                */
  BYTE  siCompression;    /* 0x00 = None, 0x01 = RLE, 0x02= LZ77 */
  BYTE  siDPI;            /* Dots Per Inch x 2 (0x10 for .wmf)   */
}  
SHGIMAGEHEADER;

/* Defines for image type and compression type */
#define IT_BMP   0x06
#define IT_WMF   0x08
#define IC_NONE  0x00
#define IC_RLE   0x01
#define IC_LZ77  0x02
#define IC_BOTH  0x03

/* SHG Bitmap Header */
typedef struct tagSHGBITMAPHEADER
{
  BYTE  sbIsZero;      /* Always 0x00                   */
  BYTE  sbDPI;         /* Dots Per Inch x 2             */
  WORD  sbTwoHund;     /* 0x0200                        */
  WORD  sbNumBits;     /* Number bits per pixel x 2     */
  DWORD sbWidth;       /* Width x 2                     */
  DWORD sbHeight;      /* Height x 2                    */
  DWORD sbNumQuads;    /* Number RGB Quads x 2          */
  WORD  sbNumImp;      /* Number of 'important' RGB Qds */
  DWORD sbCmpSize;     /* Size of Compressed BMP x 2    */
  DWORD sbSizeHS;      /* Size of Hotspot Data area x 2 */
  DWORD sbunk1; 
  DWORD sbSizeImage;   /* size ImageHdr+BmpHdr+ImageDat */
}
SHGBITMAPHEADER;

/* SHG Metafile Header */
typedef struct tagSHGMETAFILEHEADER
{
  WORD  smXWidth;    /* Width of image in metafile units  */
  WORD  smYHeight;   /* Height of image in metafile units */
  DWORD smUncSize;   /* Size of uncompressed metafile     */
  DWORD smCmpSize;   /* Size of compressed metafile       */
  DWORD smSizeHS;    /* Size of hot spot data area x 2    */
  DWORD smUnk1; 
  DWORD smSizeImage; /* Size ImageHdr+wmfHdr+ImageDat     */
}
SHGMETAFILEHEADER;

/* Documented in "Microsoft Windows 3.1 Programmer's 
   Referenece Volume 4, Resources." Not in WINDOWS.H.
   This is the header for a Placeable Metafile        */
typedef struct tagMETAFILEHEADER
{
  DWORD  key;
  HANDLE hmf;      /* Must be 0 */
  RECT   bbox;
  WORD   inch; 
  DWORD  reserved; /* Must be 0 */
  WORD   checksum;
}
METAFILEHEADER;

/* Hot Spot Header */
typedef struct tagHOTSPOTHEADER
{
  BYTE  hhOne;           /* Always 0x01                   */
  WORD  hhNumHS;         /* Number of Hot Spots           */
  DWORD hhContextOffset; /* Offset to Cntxt Strings & IDs */
}
HOTSPOTHEADER;

/* Hot Spot Record */
typedef struct tagHOTSPOTRECORD
{
  WORD  hrType; /* Hot Spot Type. See below */
  BYTE  hrZero; /* Always 0                 */
  WORD  hrLeft; /* Bounding box of Hot Spot */
  WORD  hrTop;
  WORD  hrRight;
  WORD  hrBottom;
  DWORD hrMacOffset; /* Offset to macro for Hot Spot */
}
HOTSPOTRECORD;

#define HS_INVISJUMP  0x04E7
#define HS_INVISPOPUP 0x04E6
#define HS_INVISMACRO 0x04CC
#define HS_VISJUMP    0x00E3
#define HS_VISPOPUP   0x00E2
#define HS_VISMACRO   0x00C8

