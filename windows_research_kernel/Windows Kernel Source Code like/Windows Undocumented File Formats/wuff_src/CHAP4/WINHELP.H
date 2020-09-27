/**********************************************************************
 *
 * PROGRAM: WINHELP.H
 *
 * PURPOSE: Sample header file for an .HLP file
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 4, Windows Help File Format, from Undocumented Windows
 * File Formats, published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

/* Force byte aligned packing of data structures */
#pragma pack(1)


/*
   The following are defined as based on the Windows 3.1
   Programmer's Reference.
*/
#ifndef _INC_WINDOWS
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;

typedef struct tagRGBTRIPLE
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
} RGBTRIPLE;

typedef DWORD COLORREF;

#define RGB(r,g,b) ((COLORREF) \
            (((BYTE)(r)|((WORD)(g)<<8))|(((DWORD)(BYTE)(b))<<16)))

#endif

/* Define the size of a Topic Block 4k - sizeof(TOPICLINK) */
#define TopicBlockSize   4096L


/***********************************************************
   WinHelp Common Structures
************************************************************/

/* Help file Header record */
typedef struct tagHELPHEADER {
    DWORD   MagicNumber;      /* 0x00035F3F                */
    long    HFSLoc;           /* Pointer to WHIFS header   */
    long    Negative1;
    long    FileSize;         /* Size of entire .HLP File  */
} HELPHEADER;

#define HF_MAGIC  0x00035F3F

/* File Header for WHIFS files */
typedef struct tagHFSFILEHEADER {
    long    FilePlusHeader;  /* File size including this header */
    long    FileSize;        /* File size not including header  */
    char    FileType;      /*                                 */
} HFSFILEHEADER;

/* File types used by HFS               */
/* FT_NORMAL is a regular file                */
/* FT_HFS is the HFS directory file         */
/* FT_UNK Found in MSDNCD4.MVB |TTLBTREE file   */

#define FT_NORMAL 0x00
#define FT_HFS    0x04


/***********************************************************
   B-Tree related structures
************************************************************/

/* Keyword & TTL BTREE Headers - Slightly different than HFS B-tree Header.
   Both HFS and Keyword B-Trees use same leaf and index node headers.    */

typedef struct tagBTREEHEADER {
  WORD  Signature;        /* 0x293B                               */
  char  Unknown1;         /* 0x02 always                          */
  char  FileTypeFlag;     /* Same as FILEHEADER FileTypeFlag field*/
  short PageSize;         /* Size of tree pages                   */
  char  SortOrder[16];    /* Used for internationalization        */
  short FirstLeaf;      /* Probably First Leaf page!!!          */
  short NSplits;        /* # of page splits Btree has suffered  */
  short RootPage;       /* page #of root page                   */
  short Reserved2;
  short TotalPages;     /* total # of 2Kb pages in Btree        */
  short nLevels;        /* # of levels in this Btree            */
  DWORD TotalBtreeEntries;
} BTREEHEADER;

/* Modified B-Tree Leaf Header */
typedef struct tagBTREELEAFHEADER {
    WORD    Signature;      /* Signature word            */
    short   NEntries;       /* Number of entries         */
    short   PreviousPage;   /* Index of Previous Page    */
    short   NextPage;       /* Index of Next Page        */
} BTREELEAFHEADER;

/* Modified B-Tree Index node header */
typedef struct tagBTREEINDEXHEADER {
    WORD    Signature;      /* Signature byte            */
    short   NEntries;       /* Number of entries in node */
} BTREEINDEXHEADER;

/***********************************************************
   |Phrases header
************************************************************/

/* Phrases header. In uncompressed, last field doesn't exist */
typedef struct tagPHRASEHEADER    {
    short   NumPhrases;     /* Number of phrases in table                     */
    WORD    OneHundred;     /* 0x0100                                         */
    long    PhrasesSize;    /* Amount of space uncompressed phrases requires. */
} PHRASEHEADER;


/***********************************************************
   |FONT File structures
************************************************************/

/* Header for |FONT file */
typedef struct tagFONTHEADER {
    WORD    NumFonts;           /* Number of fonts in Font List */
    WORD    NumDescriptors;     /* Number of font descriptors   */
    WORD    DefDescriptor;      /* Default font descriptor      */
    WORD    DescriptorsOffset;  /* Offset to descriptor list    */
} FONTHEADER;

typedef struct tagFONTDESCRIPTOR {
    BYTE    Attributes;         /* Font Attributes See values below */
    BYTE    HalfPoints;         /* PointSize * 2                    */
    BYTE    FontFamily;         /* Font Family. See values below    */
    BYTE    FontName;           /* Number of font in Font List      */
    BYTE    Unknown;            /* Unknown                          */
    RGBTRIPLE SRRGB;            /* RGB values of foreground         */
    RGBTRIPLE NSRRGB;           /* background RGB Values (?? Not sure */
} FONTDESCRIPTOR;

/* Font Attributes */
#define FONT_NORM    0x00       /* Normal         */
#define FONT_BOLD    0x01       /* Bold           */
#define FONT_ITAL    0x02       /* Italics        */
#define FONT_UNDR    0x04       /* Underline      */
#define FONT_STRK    0x08       /* Strike Through */
#define FONT_DBUN    0x10       /* Dbl Underline  */
#define FONT_SMCP    0x20       /* Small Caps     */

/* Font Families */
#define FAM_MODERN   0x01
#define FAM_ROMAN    0x02
#define FAM_SWISS    0x03
#define FAM_TECH     0x03
#define FAM_NIL      0x03
#define FAM_SCRIPT   0x04
#define FAM_DECOR    0x05


/***********************************************************
   |SYSTEM file structures
************************************************************/

/* Header for |SYSTEM file */
typedef struct tagSYSTEMHEADER {
    BYTE    Magic;     /* 0x6C                  */
    BYTE    Version;   /* Version #             */
    BYTE    Revision;  /* Revision code         */
    BYTE    Always0;   /* Unknown               */
    WORD    Always1;   /* Always 0x0001         */
    DWORD   GenDate;   /* Date/Time that the help file was generated    */
    WORD    Flags;     /* Values seen: 0x0000 0x0004, 0x0008, 0x000A    */
} SYSTEMHEADER;

/* Magic number of SYSTEM record */
#define SYS_MAGIC     0x6C

/* Flags for |SYSTEM header Flags field below */
#define NO_COMPRESSION          0x0000
#define COMPRESSION_HIGH        0x0004

/* Help Compiler 3.1 System record. Multiple records possible */
typedef struct tagSYSTEMREC {
    WORD    RecordType;   /* Type of Data in record      */
    WORD    DataSize;     /* Size of RData               */
    char    *RData;       /* Raw data (Icon, title, etc) */
} SYSTEMREC;

/* Types for SYSTEMREC RecordType below */  
#define HPJ_TITLE       0x0001      /* Title from .HPJ file            */
#define HPJ_COPYRIGHT   0x0002      /* Copyright notice from .HPJ file */
#define HPJ_CONTENTS    0x0003      /* Contents=??? from .HPJ          */
#define MACRO_DATA      0x0004      /* SData = 4 nulls if no macros    */
#define ICON_DATA       0x0005
#define HPJ_SECWINDOWS  0x0006      /* Secondary window info in .HPJ   */
#define HPJ_CITATION    0x0008      /* CITATION= under [OPTIONS]       */


/* Secondary Window Record following type 0x0006 System Record */

typedef struct tagSECWINDOW {
    WORD    Flags;          /* Flags (See Below)        */
    BYTE    Type[10];       /* Type of window           */
    BYTE    Name[9];        /* Window name              */
    BYTE    Caption[51];    /* Caption for window       */
    WORD    X;              /* X coordinate to start at */
    WORD    Y;              /* Y coordinate to start at */
    WORD    Width;          /* Width to create for      */
    WORD    Height;         /* Height to create for     */
    WORD    Maximize;       /* Maximize flag            */
    BYTE    Rgb[3];
    BYTE    Unknown1;
    BYTE    RgbNsr[3];      /* RGB for non scrollable region */
    BYTE    Unknown2;
} SECWINDOW;

/** Values for Flags **/

#define WSYSFLAG_TYPE       0x0001  /* Type is valid        */
#define WSYSFLAG_NAME       0x0002  /* Name is valid        */
#define WSYSFLAG_CAPTION    0x0004  /* Ccaption is valid    */
#define WSYSFLAG_X          0x0008  /* X    is valid        */
#define WSYSFLAG_Y          0x0010  /* Y    is valid        */
#define WSYSFLAG_WIDTH      0x0020  /* Width    is valid    */
#define WSYSFLAG_HEIGHT     0x0040  /* Height   is valid    */
#define WSYSFLAG_MAXIMIZE   0x0080  /* Maximize is valid    */
#define WSYSFLAG_RGB        0x0100  /* Rgb  is valid        */
#define WSYSFLAG_RGBNSR     0x0200  /* RgbNsr   is valid    */
#define WSYSFLAG_TOP        0x0400  /* On top was set in HPJ file */


/***********************************************************
   Keyword file structures
************************************************************/

/* Keyword Map Record */
typedef struct tagKWMAPREC {
    long    FirstRec;       /* Index number of first keyword on leaf page    */
    WORD    PageNum;        /* Page number that keywords are associated with */
} KWMAPREC;

/* Record for the |KWBTREE file */
typedef struct tagKWBTREEREC {
    char     Keyword[80];     /* Variable Length Keyword      */
    short    Count;           /* Count of Keywords occurances */
    long     KWDataOffset;    /* Offset into |KWDATA file     */
} KWBTREEREC;


/***********************************************************
   |TOPIC file structures
************************************************************/

/* |TOPIC Block header - Header for a block of topic data. If
   uncompressed, there's only one of these at the beginning of the
   file. If the help file is compressed, then these occur in 4k
   increments. (e.g. 0x0000, 0x1000, 0x2000, 0x3000, 0x4000, etc. ) */

typedef long  TOPICOFFSET;

typedef struct tagTOPICBLOCKHEADER {
    long    LastTopicLink;   /* Offset of last topic link in previous block   */
    long    TopicData;       /* Offset of topic data start                    */
    long    LastTopicHeader; /* Offset of last topic header in previous block */
} TOPICBLOCKHEADER;

/* Linked list record for |TOPIC file */
typedef struct tagPARAGRAPH {
    long     BlockSize;   /* Size of this link + Data         */
    long     DataLen2;    /* Length of LinkData2              */
    long     PrevBlock;   /* Relative to first byte of |TOPIC */
    long     NextBlock;   /* Relative to first byte of |TOPIC */
    long     DataLen1;    /* Len(LinkData1 + 11(hdr size))    */
    BYTE     RecordType;  /* See below                        */
    BYTE     *LinkData1;  /* Data associated with this link   */
    BYTE     *LinkData2;  /* Second set of data               */
} PARAGRAPH;

/* Known record types for topic link */
#define TL_TOPICHDR    0x02  /* Topic header information */
#define TL_DISPLAY     0x20  /* Displayable information  */
#define TL_TABLE       0x23  /* WinHelp Table            */

/* Topic header. Starts inside LinkData of a type 0x02 record */
typedef struct tagTOPICHEADER {
    long          BlockSize; /* Size of topic, including internal topic links  */
    TOPICOFFSET   BrowseBck; /* Topic offset for prev topic in Browse sequence */
    TOPICOFFSET   BrowseFor; /* Topic offset for next topic in Browse sequence */
    DWORD         TopicNum;  /* Topic Number(?)                                */
    long          NonScroll; /* Start of Non-Scroll Region                     */
    long          Scroll;    /* Start of Scrolling Region of text.             */
    TOPICOFFSET   NextTopic; /* Start of next Type 0x02 record                 */
} TOPICHEADER;

/***********************************************************
   Structures for other system files
************************************************************/

/* Header for |TOMAP file */
typedef struct tagTOMAPHEADER {
    long    IndexTopic;   /* Index topic for help file */
    long    Reserved[15];
    short   ToMapLen;     /* Number of topic pointers  */
    long    *TopicPtr;    /* Pointer to all the topics */
} TOMAPHEADER;

/* Record from |CTXOMAP file. Created from the [MAP] section of .HPJ file */
typedef struct tagCTXOMAPREC {
    long     MapID;
    long     TopicOffset;
} CTXOMAPREC;


/* Record from |CONTEXT file */
typedef struct tagCONTEXTREC {
    long     HashValue;       /* Hash value of a phrase     */
    long     TopicOffset;     /* Topic offset of the phrase */
} CONTEXTREC;

