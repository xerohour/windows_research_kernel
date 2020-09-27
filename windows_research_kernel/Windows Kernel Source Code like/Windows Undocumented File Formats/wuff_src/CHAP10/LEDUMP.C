/**********************************************************************
 *
 * PROGRAM: LEDUMP.C
 *
 * PURPOSE: Dump 'LE' file info
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 10, LE File Format, from Undocumented Windows File Formats,
 * published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <windows.h>
#include "ledump.h"

/* Make this big thing global since we'll need it everywhere. */
LEHEADER	LEHeader;
long		LEStart;	/* Keep track of start of LE Header */


BOOL SkipMZ(FILE *LEFile)
{
	MZHEADER MZHeader;

	fread(&MZHeader, sizeof(MZHeader), 1, LEFile);
	if (MZHeader.MZMagic[0] != 'M' || MZHeader.MZMagic[1] != 'Z')
	{
		printf("This is not an executable file!\n");
		return FALSE;
	}

	if (!MZHeader.LEOff)
	{
		printf("This is a DOS executable.\n");
		return FALSE;
	}

	fseek(LEFile, MZHeader.LEOff, SEEK_SET);
	return TRUE;
}

void DumpHeaderInfo()
{
	DWORD Flags;
	
	Flags = LEHeader.ModuleFlags; /* Get it into an easier to type variable */
	
	printf("CPU Required: ");
	switch(LEHeader.CPUType)
	{
		case CPU_286:
			printf("80286\n");
			break;
		case CPU_386:
			printf("80386\n");
			break;
		case CPU_486:
			printf("80486\n");
			break;
	}
	
	printf("OS Required: ");
	switch(LEHeader.OSType)
	{
		case OS_UNKNOWN:
			printf("Unknown\n");
			break;
		case OS_OS2:
			printf("OS/2\n");
			break;
		case OS_WINDOWS:
			printf("Windows\n");
			break;
		case OS_DOS4X:
			printf("DOS 4.x\n");
			break;
		case OS_WINDOWS386:
			printf("Windows 386 Enhanced Mode\n");
			break;
	}
	
	printf("Initial EIP: %lX:%08lX\n", LEHeader.EIPObjNum, LEHeader.EIP);
	printf("Initial ESP: %lX:%08lX\n", LEHeader.ESPObjNum, LEHeader.ESP);
	
	printf("Flags: 0x%08lX\n", Flags);
	if (Flags & MOD_PERPROCESSLIBINIT)
		printf("- Per Process Library Initialization\n");
	if (Flags & MOD_INTERNALFIXUPS)
		printf("- Internal Fixups\n");
	if (Flags & MOD_EXTERNALFIXUPS)
		printf("- External Fixups\n");
	if ( (Flags & MOD_USES_PM) == MOD_USES_PM)
		printf("- Uses Presentation Manager\n");
	else
	{
		if (Flags & MOD_INCOMPAT_PM)
			printf("- Incompatible with Presentation Manager\n");
		if (Flags & MOD_COMPAT_PM)
			printf("- Compatible with Presentation Manager\n");
	}
	if (Flags & MOD_NOTLOADABLE)
		printf("- Module is not loadable.\n");
	if ( (Flags & MOD_PROTLIBMOD) == MOD_PROTLIBMOD)
		printf("- Protected Memory Library Module\n");
	else if ( (Flags & MOD_VIRTDEVICEDRVR) == MOD_VIRTDEVICEDRVR)
		printf("- Virtual Device Driver Module\n");
	else
	{
		if (Flags & MOD_LIBMOD)
			printf("- Library Module\n");
		if (Flags & MOD_PHYSDEVICEDRVR)
			printf("- Physical Device Driver\n");
	}
	if (Flags & MOD_PERPROCLIBTERM)
		printf("- Per-Process Library Termination\n");
		
	printf("\n");
}

void DumpObjectTable(FILE *LEFile)
{
	DWORD 		i;
	OBJTBLENTRY	ote;
	
	printf("There are %ld objects in the Object Table.\n", LEHeader.NumObjects);
	
	fseek(LEFile, LEStart + LEHeader.ObjTblOffset, SEEK_SET);
	for (i=1; i<=LEHeader.NumObjects; i++)
	{
		fread(&ote, sizeof(ote), 1, LEFile);
		printf("Segment #: %ld  Size: 0x%08lX  Reloc Addr: 0x%08lX   # Pages: %ld\n",
				i, ote.VirtualSize, ote.RelocBaseAddr, ote.NumPgTblEntries);
		if (i == LEHeader.AutoDSObj)
			printf("Auto Data Segment\n");
		printf("Flags:\n");
		if (ote.ObjectFlags & OBJ_READABLE)
			printf("Readable\n");
		if (ote.ObjectFlags & OBJ_WRITEABLE)
			printf("Writeable\n");
		if (ote.ObjectFlags & OBJ_EXECUTABLE)
			printf("Executable\n");
		if (ote.ObjectFlags & OBJ_RESOURCE)
			printf("Resource\n");
		if (ote.ObjectFlags & OBJ_DISCARDABLE)
			printf("Discardable\n");
		if (ote.ObjectFlags & OBJ_SHARED)
			printf("Shared\n");
		if (ote.ObjectFlags & OBJ_PRELOAD)
			printf("Preload\n");
		if (ote.ObjectFlags & OBJ_INVALID)
			printf("Invalid\n");
		if ( (ote.ObjectFlags & OBJ_RESIDENTCONTIG) == OBJ_RESIDENTCONTIG)
			printf("Resident & Contiguous\n");
		else
		{
			if (ote.ObjectFlags & OBJ_ZEROFILLED)
				printf("Zero-Filled\n");
			if (ote.ObjectFlags & OBJ_RESIDENT)
				printf("Resident\n");
		}
		if (ote.ObjectFlags & OBJ_RESERVED)
			printf("Reserved\n");
		if (ote.ObjectFlags & OBJ_1616ALIAS)
			printf("16:16 Alias Required\n");
		if (ote.ObjectFlags & OBJ_BIGDEFAULT)
			printf("'Big' bit for segment descriptor\n");
		if (ote.ObjectFlags & OBJ_CONFORM)
			printf("Object is conforming to code\n");
		if (ote.ObjectFlags & OBJ_PRIVILEGE)
			printf("Object I/O privilege level\n");
		printf("\n");
	}
}

void DumpNameTable(FILE *LEFile, DWORD size)
{
	DWORD	Curr;
	BYTE	len;
	char	Name[257];
	WORD	Ord;
	
	Curr = 0;
	while (Curr < size)
	{
		fread(&len, sizeof(len), 1, LEFile);
		Curr += (len + sizeof(len) + sizeof(Ord));
		fread(Name, len, 1, LEFile);
		Name[len] = 0x00;
		fread(&Ord, sizeof(Ord), 1, LEFile);
		printf("%u         %s\n", Ord, Name);
	}
	
}

/* Dumps the Resident and Non-Resident Name Tables */
void DumpNameTables(FILE *LEFile)
{
	printf("\nResident Name Table\n");
	printf("Ordinal   Name\n");
	fseek(LEFile, LEStart + LEHeader.ResNameTable, SEEK_SET);
	DumpNameTable(LEFile, (LEHeader.EntryTable - LEHeader.ResNameTable) - 1);
	printf("\n");
	printf("Non-Resident Name Table\n");
	printf("Ordinal   Name\n");
	fseek(LEFile, LEHeader.NonResTable, SEEK_SET);
	DumpNameTable(LEFile, LEHeader.NonResSize - 1);
	
}
/*
void DumpImports(FILE *LEFile)
{
	FIXUPREC	FixupRec;
	PROCBYNAME	ProcByName;
	PROCBYORD	ProcByOrd;
	long		CurrLoc;
	
	fseek(LEFile, LEStart+LEHeader.FixupRecTable, SEEK_SET);
	

}
*/
void Check4HeaderSurprises()
{
	if (LEHeader.ByteOrder != ORD_LITTLEENDIAN)
		printf("Byte Order is not Little Endian!\n");
	if (LEHeader.WordOrder != ORD_LITTLEENDIAN)
		printf("Word Order is not Little Endian!\n");
	
	if (LEHeader.CPUType < CPU_386)
		printf("Doesn't require a 386 or better!\n");
	if (LEHeader.OSType != OS_WINDOWS386) 
		printf("Not a Windows386 VxD!\n"); 

	if (LEHeader.ResourceTbl)
		printf("Resource Table Found!\n");
	if (LEHeader.NumResources)
		printf("Num Resources Found!\n");
	if (LEHeader.ModDirectTable)
		printf("Module Directive Table Found!\n");
	if (LEHeader.NumModDirect)
		printf("NumModDirect Found!\n");
	if (LEHeader.NumInstPreload)
		printf("Instance Preloads Found!\n");
	if (LEHeader.NumInstDemand)
		printf("Instance Demands Found!\n");
	if (LEHeader.FixupChecksum)
		printf("Fixup Checksum Found!\n");
	if (LEHeader.LoaderChecksum)
		printf("Loader Checksum Found!\n");
	if (LEHeader.PerPageChecksum)
		printf("Per-Page Checksum Found!\n");
	if (LEHeader.NonResChecksum)
		printf("Non-Resident Name Table Checksum Found!\n");
}

void DumpLEFile(FILE *LEFile)
{
	LEStart = ftell(LEFile);

	fread(&LEHeader, sizeof(LEHeader), 1, LEFile);
	if (LEHeader.LEMagic[0] != 'L' || LEHeader.LEMagic[1] != 'E') 
	{
		printf("This is not an 'LE' executable.\n\n");
		return;
	}
	
	DumpHeaderInfo();
	
	Check4HeaderSurprises();
	
	DumpObjectTable(LEFile);
	
	DumpNameTables(LEFile);

//	if (LEHeader.NumImports)	
//		DumpImports(LEFile);

}

void Usage(void)
{
	printf("Usage: LEDump filename[.386]\n\n");
}

int main(int argc, char *argv[]) 
{

	char 	filename[256];
	FILE 	*LEFile;

	if (argc < 2) {
		Usage();
		return EXIT_FAILURE;
	}
	
	strcpy(filename, argv[1]);
	
	if (!strchr(filename, '.'))
		strcat(filename, ".386");
		
	if ((LEFile = fopen(filename, "rb")) == NULL) {
		printf("%s does not exist!\n", filename);
		return EXIT_FAILURE;
	}
	
	if (SkipMZ(LEFile)) 
	{
		printf("Dumping %s\n", filename);
		DumpLEFile(LEFile);
	}
	fclose(LEFile);
	return EXIT_SUCCESS;
}

