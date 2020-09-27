/**********************************************************************
 *
 * PROGRAM: WHSTRUCT.H
 *
 * PURPOSE: Windows Help File Internal Records
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 4, Windows Help File Format, from Undocumented Windows
 * File Formats, published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

typedef unsigned long   DWORD;
typedef unsigned int    WORD;
typedef unsigned char   BYTE;

/* Define WHIFS Page Size and Page size for other B-Trees */
#define WHIFSPageSize 1024L
#define BTreePageSize 2048L

/* Define the size of a Topic Block 4k - sizeof(TOPICLINK) */
#define TopicBlockSize 4084L


#define GotoWHIFSPage(DestPage)     \
    fseek(HelpFile, FirstPageLoc+(DestPage * WHIFSPageSize), SEEK_SET)

#define GotoPage(DestPage)          \
    fseek(HelpFile, FirstPageLoc+(DestPage * BTreePageSize), SEEK_SET)

#define RGB(r,g,b) ((DWORD)(b*65536) + (DWORD)(g*256) + (DWORD) (r))

/* Help file Header record */
typedef struct HELPHEADER {
    DWORD   MagicNumber;      /* 0x00035F3F                */
    long    WHIFS;            /* Pointer to WHIFS header   */
    long    Negative1;
    long    FileSize;         /* Size of entire .HLP File  */
} HELPHEADER;


/* File Header for WHIFS files */
typedef struct FILEHEADER {
    long    FilePlusHeader;  /* File size including this header */
    long    FileSize;        /* File size not including header  */
    char    TermNull;
} FILEHEADER;


/* Help Directory BTREE */
typedef struct WHIFSBTREEHEADER {
    char    Magic[18];      /* Not exactly magic for some .MVB files   */
    char    Garbage[13];
    int     MustBeZero;     /* Probably shows up when Help > ~40 megs  */
    int     NSplits;        /* Number of page split Btree has suffered */
    int     RootPage;       /* Page # of root page                     */
    int     MustBeNegOne;   /* Probably shows up when B-Tree is HUGE!! */
    int     TotalPages;     /* total # to 2Kb pages in Btree           */
    int     NLevels;        /* Number of levels in this Btree          */
    DWORD   TotalWHIFSEntries;
} WHIFSBTREEHEADER;

/* Keyword & TTL BTREE Headers - Slightly different than WHIFS B-tree Header.
   Both WHIFS and Keyword B-Trees use same leaf and index node headers.    */

typedef struct BTREEHEADER {
    char    Unknown1[22];
    int     MustBeZero1;
    int     NSplits;        /* # of page splits Btree has suffered */
    int     RootPage;       /* page #of root page */
    int     MustBeNegOne1;
    int     TotalPages;     /* total # of 2Kb pages in Btree */
    int     NLevels;        /* # of levels in this Btree */
    DWORD   TotalBtreeEntries;
} BTREEHEADER;

/* Modified B-Tree Node header to handle a pointer to the page */
typedef struct BTREENODEHEADER {
    WORD    Signature;      /* Signature word            */
    int     NEntries;       /* Number of entries         */
    int     PreviousPage;   /* Index of Previous Page    */
    int     NextPage;       /* Index of Next Page        */
} BTREENODEHEADER;

/* Modified B-Tree Index header to handle a pointer to the page */
typedef struct BTREEINDEXHEADER {
    WORD    Signature;      /* Signature byte            */
    int     NEntries;       /* Number of entries in node */
} BTREEINDEXHEADER;


/* Phrase header In uncompressed, last field is empty */
typedef struct PHRASEHDR    {
    int     NumPhrases;     /* Number of phrases in table                     */
    WORD    OneHundred;     /* 0x0100                                         */
    long    PhrasesSize;    /* Amount of space uncompressed phrases requires. */
} PHRASEHDR;



/* Header for |FONT file */
typedef struct FONTHEADER {
    WORD    NumFonts;           /* Number of fonts in Font List */
    WORD    NumDescriptors;     /* Number of font descriptors   */
    WORD    DefDescriptor;      /* Default font descriptor      */
    WORD    DescriptorsOffset;  /* Offset to descriptor list    */
} FONTHEADER;

typedef struct FONTDESCRIPTOR {
    BYTE    Attributes;         /* Font Attributes See values below */
    BYTE    HalfPoints;         /* PointSize * 2                    */
    BYTE    FontFamily;         /* Font Family. See values below    */
    BYTE    FontName;           /* Number of font in Font List      */
    BYTE    Unknown;
    BYTE    FGRGB[3];           /* RGB values of foreground         */
    BYTE    BGRGB[3];           /* background RGB Values (?? Not sure */
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


/* Header for |SYSTEM file */
typedef struct SYSTEMHEADER {
    BYTE    Magic;     /* 0x6C                  */
    BYTE    Version;   /* Version #             */
    BYTE    Revision;  /* Revision code         */
    BYTE    Always0;   /* Unknown               */
    WORD    Always1;   /* Always 0x0001         */
    DWORD   GenDate;   /* Date/Time that the help file was generated    */
    WORD    Flags;     /* Values seen: 0x0000 0x0004, 0x0008, 0x000A    */
} SYSTEMHEADER;

/* Flags for |SYSTEM header Flags field below */
#define NO_COMPRESSION_310      0x0000
#define COMPRESSION_310         0x0004
#define COMPRESSION_UNKN        0x0008
#define SYSFLAG_300             0x000A

/* Help Compiler 3.1 System record. Multiple records possible */
typedef struct SYSTEMREC {
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

typedef struct SECWINDOW {
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

/* Header for |TOMAP file */
typedef struct TOMAPHEADER {
    long    IndexTopic;   /* Index topic for help file */
    long    Reserved[15];
    int     ToMapLen;     /* Number of topic pointers  */
    long    *TopicPtr;    /* Pointer to all the topics */
} TOMAPHEADER;



/* Keyword Map Record */
typedef struct KWMAPREC {
    long    FirstRec;       /* Index number of first keyword on leaf page    */
    WORD    PageNum;        /* Page number that keywords are associated with */
} KWMAPREC;


/* |TOPIC Block header - Header for a block of topic data. If
uncompressed, there's only one of these at the beginning of the
file. If the help file is compressed, then these occur in 4k
increments. (e.g. 0x0000, 0x1000, 0x2000, 0x3000, 0x4000, etc. ) */
typedef struct TOPICBLOCKHEADER {
    long    LastTopicLink;   /* Offset of last topic link in previous block   */
    long    TopicData;       /* Offset of topic data start                    */
    long    LastTopicHeader; /* Offset of last topic header in previous block */
} TOPICBLOCKHEADER;


/* Linked list record for |TOPIC file */
typedef struct TOPICLINK {
    long     BlockSize;   /* Size of this link + Data         */
    long     DataLen2;    /* Length of LinkData2              */
    long     PrevBlock;   /* Relative to first byte of |TOPIC */
    long     NextBlock;   /* Relative to first byte of |TOPIC */
    long     DataLen1;    /* Len(LinkData1 + 11(hdr size))    */
    BYTE     RecordType;  /* See below                        */
    BYTE     *LinkData1;  /* Data associated with this link   */
    BYTE     *LinkData2;  /* Second set of data               */
} TOPICLINK;

/* Known record types for topic link */
#define TL_TOPICHDR    0x02  /* Topic header information */
#define TL_DISPLAY     0x20  /* Displayable information  */
#define TL_TABLE       0x23  /* WinHelp Table            */

/* Topic header. Starts inside LinkData of a type 0x02 record */
typedef struct TOPICHEADER {
    long     BlockSize; /* Size of topic, including internal topic links  */
    long     BrowseBck; /* Topic offset for prev topic in Browse sequence */
    long     BrowseFor; /* Topic offset for next topic in Browse sequence */
    DWORD    TopicNum;  /* Topic Number(?)                                */
    long     NonScroll; /* Start of Non-Scroll Region                     */
    long     Scroll;    /* Start of Scrolling Region of text.             */
    long     NextTopic; /* Start of next Type 0x02 record                 */
} TOPICHEADER;

/* Record from |CTXOMAP file. Created from the [MAP] section of .HPJ file */
typedef struct CTXOMAPREC {
    long     MapID;
    long     TopicOffset;
} CTXOMAPREC;


/* Record from |CONTEXT file */
typedef struct CONTEXTREC {
    long     HashValue;       /* Hash value of a phrase     */
    long     TopicOffset;     /* Topic offset of the phrase */
} CONTEXTREC;

/* Record for the |KWBTREE file */
typedef struct KWBTREEREC {
    char     Keyword[80];     /* Variable Length Keyword      */
    int      Count;           /* Count of Keywords occurances */
    long     KWDataOffset;    /* Offset into |KWDATA file     */
} KWBTREEREC;

