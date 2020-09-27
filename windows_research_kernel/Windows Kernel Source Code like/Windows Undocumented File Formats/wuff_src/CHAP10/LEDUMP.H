/**********************************************************************
 *
 * PROGRAM: LEDUMP.H
 *
 * PURPOSE: Header file for LEDUMP.C
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 10, LE File Format, from Undocumented Windows File Formats,
 * published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

/* Mini-MZ Header. Need only 2 fields */
typedef struct tagMZHEADER
{
	char	MZMagic[2];
	char	Stuff[58];		/* Stuff we don't care about */
	long	LEOff;
} MZHEADER;

/* LE Header structure */
typedef struct tagLEHEADER
{
	char	LEMagic[2];
	BYTE	ByteOrder;
	BYTE	WordOrder;
	DWORD	FormatLevel;
	WORD	CPUType;
	WORD	OSType;
	DWORD	ModuleVer;
	DWORD	ModuleFlags;
	DWORD	NumPages;
	DWORD	EIPObjNum;
	DWORD	EIP;
	DWORD	ESPObjNum;
	DWORD	ESP;
	DWORD	PageSize;
	DWORD	LastPageSize;
	DWORD	FixupSize;
	DWORD	FixupChecksum;
	DWORD	LoaderSize;
	DWORD	LoaderChecksum;
	DWORD	ObjTblOffset;
	DWORD	NumObjects;
	DWORD	ObjPageTbl;
	DWORD	ObjIterPage;
	DWORD	ResourceTbl;
	DWORD	NumResources;
	DWORD	ResNameTable;
	DWORD	EntryTable;
	DWORD	ModDirectTable;
	DWORD	NumModDirect;
	DWORD	FixUpPageTable;
	DWORD	FixUpRecTable;
	DWORD	ImportModTable;
	DWORD	NumImports;
	DWORD	ImportProcTable;
	DWORD	PerPageChecksum;
	DWORD	DataPages;
	DWORD	NumPreloadPages;
	DWORD	NonResTable;
	DWORD	NonResSize;
	DWORD	NonResChecksum;
	DWORD	AutoDSObj;
	DWORD	DebugInfoOff;
	DWORD	DebugInfoLen;
	DWORD	NumInstPreload;
	DWORD	NumInstDemand;
	DWORD	HeapSize;
} LEHEADER;
	
/* Defines for Byte and Word Order */
#define ORD_LITTLEENDIAN	0x00
#define	ORD_BIGENDIAN		0x01

/* Defines for CPU Type */
#define CPU_286				0x01
#define CPU_386				0x02
#define CPU_486				0x03

/* Defines for OS Type */
#define OS_UNKNOWN			0x00
#define OS_OS2				0x01
#define OS_WINDOWS			0x02
#define OS_DOS4X			0x03
#define OS_WINDOWS386		0x04

/* Defines for Module Flags */
#define MOD_PERPROCESSLIBINIT	0x00000004
#define MOD_INTERNALFIXUPS		0x00000010
#define MOD_EXTERNALFIXUPS		0x00000020
#define MOD_INCOMPAT_PM			0x00000100
#define MOD_COMPAT_PM			0x00000200
#define MOD_USES_PM				0x00000300
#define MOD_NOTLOADABLE			0x00002000
#define MOD_MODTYPEMASK			0x00038000
#define MOD_PROGRAMMOD			0x00000000
#define MOD_LIBMOD				0x00008000
#define MOD_PROTLIBMOD			0x00018000
#define MOD_PHYSDEVICEDRVR		0x00020000
#define MOD_VIRTDEVICEDRVR		0x00028000
#define MOD_PERPROCLIBTERM		0x40000000

	
/* Object Table Entry Records */
typedef struct tagOBJTBLENTRY
{
	DWORD	VirtualSize;
	DWORD	RelocBaseAddr;
	DWORD	ObjectFlags;
	DWORD	PageTableIndex;
	DWORD	NumPgTblEntries;
	DWORD	Reserved;
} OBJTBLENTRY;

/* Defines for Object Flags */
#define OBJ_READABLE		0x0001
#define OBJ_WRITEABLE		0x0002
#define OBJ_EXECUTABLE		0x0004
#define OBJ_RESOURCE		0x0008
#define OBJ_DISCARDABLE		0x0010
#define OBJ_SHARED			0x0020
#define OBJ_PRELOAD			0x0040
#define OBJ_INVALID			0x0080
#define OBJ_ZEROFILLED		0x0100
#define OBJ_RESIDENT		0x0200
#define OBJ_RESIDENTCONTIG	0x0300
#define OBJ_RESIDENTLOCK	0x0400
#define OBJ_RESERVED		0x0800
#define OBJ_1616ALIAS		0x1000
#define OBJ_BIGDEFAULT		0x2000
#define OBJ_CONFORM			0x4000
#define OBJ_PRIVILEGE		0x8000


/* Object Page Table Entries */
typedef struct tagOBJPAGETBLENTRY
{
	DWORD	PageDataOffset;
	WORD	DataSize;
	WORD	ObjPageFlags;
} OBJPAGETBLENTRY;

/* Defines for Object Page Flags */
#define OPG_LEGAL			0x00
#define OPG_ITERATEDDATA	0x01
#define OPG_INVALID			0x02
#define OPG_ZEROFILL		0x03
#define OPG_RANGE			0x04


typedef struct tagFIXUPREC
{
	BYTE	Src;
	BYTE	Flags;
	WORD	SrcOff_Cnt;
} FIXUPREC;

typedef struct tagPROCBYNAME
{
	WORD	ModuleOrd;
	DWORD	ProcNameOff;
} PROCBYNAME;

typedef struct tagPROCBYORD
{
	WORD	ModuleOrd;
	WORD	ImportOrd;
};

