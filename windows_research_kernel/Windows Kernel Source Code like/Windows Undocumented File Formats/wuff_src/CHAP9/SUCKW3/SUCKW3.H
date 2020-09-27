/**********************************************************************
 *
 * PROGRAM: SUCKW3.H
 *
 * PURPOSE: Extracts VxDs from a W3 file.
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 9, W3 and W4 File Formats, from Undocumented Windows File Formats,
 * published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef unsigned char BOOL;

#define TRUE	1
#define FALSE	0

typedef struct tagMZHEADER
{
	char	MZMagic[2];
	char	Stuff[58];
	long	OtherOff;
} MZHEADER;

typedef struct tagW3HEADER
{
	char	W3Magic[2];
	WORD	WinVer;
	WORD	NumVxDs;
	BYTE	Reserved[10];
} W3HEADER;

typedef struct tagVxDRECORD
{
	char	VxDName[8];
	long	VxDStart;
	long	VxDHdrSize;
} VxDRECORD;



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

