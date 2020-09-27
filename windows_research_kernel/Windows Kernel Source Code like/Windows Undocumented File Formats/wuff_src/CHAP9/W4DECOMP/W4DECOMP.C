/**********************************************************************
 *
 * PROGRAM: W4DECOMP.C
 *
 * PURPOSE: Decompresses a W4 file into a W3 file.
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 9, W3 and W4 File Formats, from Undocumented Windows File Formats,
 * published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "w4decomp.h"

void LoadMiniBuffer(DWORD *pMiniBuffer, 
                    BYTE **pSrcBuffer, 
                    WORD *pBitsUsed,
                    WORD *pBitCount)
{
  while ((*pBitsUsed)--)
  {
    *pMiniBuffer >>= 1;
    if (--(*pBitCount) == 0)
    {
      *pMiniBuffer += (DWORD) **pSrcBuffer << 24l;
      *pSrcBuffer += 1;
      *pBitCount = 8;
    }
  }
}


WORD W4Decompress(BYTE *pSrcBuffer, BYTE *pDestBuffer, WORD nSize)
{ 
  DWORD dwMiniBuffer = 0;
  WORD  nCount, nDepth;     // Count and Depth of a compressed "string"
  WORD  nBitCount;          // How many bits are left before we read
                            // another BYTE into dwMiniBuffer
  WORD  nBitsUsed;          // Number of bits used by the last "code",
                            // a code being a count or depth value.                            
  WORD  nDestIndex;         // Index into pDestBuffer
  WORD  nIndex;

  WORD  tmpSize = nSize;
  
  BYTE  *pTmpBuffer;
  
  pTmpBuffer = pSrcBuffer;
  
  // Load up dwMiniBuffer with first 4 bytes. We want it
  // to look like this:   
  //
  // msb              dwMiniBuffer              lsb
  //  +----------+----------+----------+---------+
  //  |  byte 3  |  byte 2  |  byte 1  |  byte 0 |
  //  +----------+----------+----------+---------+
  //
    
  nDestIndex = 0;           // start at byte 0 of dest buffer
  
  for (nIndex = 0; nIndex <= 3; nIndex++)
    dwMiniBuffer = (dwMiniBuffer >> 8) + ((DWORD)*pSrcBuffer++ << 24);
  
  nBitCount = 8;  // We start with 8 bits left before reading another BYTE
  
  nDepth = 1;      // Just allows us into the following loop.
  
  // While nDepth != 0. In other words, if there's nothing left
  // to decompress, then we're done.
  while (nDepth)
  {                     

    // Is the next piece of data an uncompressed byte?
    if (((dwMiniBuffer & 0x0003l) == 0x0001l) ||
        ((dwMiniBuffer & 0x0003l) == 0x0002l))
    {
 
      if (--nSize == 0xFFFF)
      {
        printf("Error: Over-run of data\n");
        return 0;
      }
 
      pDestBuffer[nDestIndex++] = (BYTE)(((dwMiniBuffer & 0x01FCl) >> 2l) |
                                         ((dwMiniBuffer & 0x0001l) << 7l));
      nBitsUsed = 9;
    }
    else // Depth data is compressed
    { 
      // (0-63)
      if ((dwMiniBuffer & 0x0003l) == 0x0000l)
      {
        nDepth = (WORD)((dwMiniBuffer & 0x00FCl) >> 2);
        nBitsUsed = 8;
      }
      // (64-319)
      else if ((dwMiniBuffer & 0x0007l) == 0x0003l)
      {
        nDepth = (WORD)((dwMiniBuffer & 0x07F8l) >> 3) + 0x0040;
        nBitsUsed = 11;
      }
      // (320-4414)
      else if ((dwMiniBuffer & 0x0007l) == 0x0007l)
      {
        nDepth = (WORD)((dwMiniBuffer & 0x07FF8l) >> 3) + 0x0140;
        nBitsUsed = 15;
      }
      else
      {
        printf("Error, invalid depth data. \n");
        return 0;
      }
          
      // If depth isn't 0 and not a CheckBuffer,
      // load buffer, as needed.
      if ((nDepth) && 
          (nDepth != 0x113F)) // 0x113F == (4415 - 320)
      {
        LoadMiniBuffer(&dwMiniBuffer, &pSrcBuffer, &nBitsUsed, &nBitCount);
        
        // Get count
        if ((dwMiniBuffer & 0x00001l) == 0x00001l) // 2
        {
          nCount = 2;
          nBitsUsed = 1;
        }
        else if ((dwMiniBuffer & 0x00003l) == 0x0002l) // 3-4
        {
          nCount = (WORD)((dwMiniBuffer & 0x0004l) >> 2l) + 3;
          nBitsUsed = 3;
        }
        else if ((dwMiniBuffer & 0x00007l) == 0x00004l) // 5-8
        {
          nCount = (WORD)((dwMiniBuffer & 0x00018l) >> 3l) + 5;
          nBitsUsed = 5;
        }
        else if ((dwMiniBuffer & 0x0000Fl) == 0x0008l) // 9-16
        {
          nCount = (WORD)((dwMiniBuffer & 0x00070l) >> 4l) + 9;
          nBitsUsed = 7;
        }
        else if ((dwMiniBuffer & 0x0001Fl) == 0x0010l) // 17-32
        {
          nCount = (WORD)((dwMiniBuffer & 0x001E0l) >> 5l) + 17;
          nBitsUsed = 9;
        }
        else if ((dwMiniBuffer & 0x0003Fl) == 0x00020l) // 33-64
        {
          nCount = (WORD)((dwMiniBuffer & 0x007C0l) >> 6l) + 33;
          nBitsUsed = 11;
        }
        else if ((dwMiniBuffer & 0x0007Fl) == 0x00040l) // 65-128
        {
          nCount = (WORD)((dwMiniBuffer & 0x01F80l) >> 7l) + 65;
          nBitsUsed = 13;
        }
        else if ((dwMiniBuffer & 0x000FFl) == 0x00080l) // 129-256
        {
          nCount = (WORD)((dwMiniBuffer & 0x07F00l) >> 8l) + 129;
          nBitsUsed = 15;
        }
        else if ((dwMiniBuffer & 0x001FFl) == 0x00100l) // 257-512
        {
          nCount = (WORD)((dwMiniBuffer & 0x01FE00l) >> 9l) + 257;
          nBitsUsed = 17;
        }
        else
        {
          // Bad data, but handle as if it were a quit condition
          printf("Bad count data, quiting at current point.\n");
          nDepth = 0;
          nCount = 0;
          nBitsUsed = 9;
        }
  
        // Copy "nCount" bytes of data from "nDepth" bytes back,      
        while (nCount--)
        {
          if (--nSize == 0xFFFF)
          {
            printf("Error: Over-run of data\n");
            return 0;
          }
          
          pDestBuffer[nDestIndex] = pDestBuffer[nDestIndex - nDepth];
          nDestIndex++;
        }
      }
      else
      {
        // If we get a Check Buffer and
        // size of the remaining data is 0,
        // then we're done.
        if ((nDepth == 0x113F) &&
            (nSize == 0x0000))
        { 
          nDepth = 0;
        }
      }
        
    } // else
    
    LoadMiniBuffer(&dwMiniBuffer, &pSrcBuffer, &nBitsUsed, &nBitCount);
    
  } // while(nDepth)

  return nDestIndex;
}

int ExtractW3(FILE *W4File, char *filename)
{
  FILE     *W3File;
  MZHEADER mzHeader;
  W4HEADER w4Header;
  DWORD    *pChunkTable;
  DWORD    start, end;
  BYTE     *pSrcBuffer, *pDestBuffer;
  WORD     nChunkIndex;
  WORD     nDestSize;
  DWORD    i;

  if ((W3File = fopen("LIBRARY.W3", "wb")) == NULL) 
  {
    printf("Unable to open file LIBRARY.W3 for output\n");
    return 1;
  }

  fread(&mzHeader, sizeof(mzHeader), 1, W4File);
  if (mzHeader.Magic != MZMAGIC)
  {
    printf("Not an executable file.\n");
    return 1;
  }
  
  fseek(W4File, mzHeader.OtherOff, SEEK_SET);
  
  fread(&w4Header, sizeof(w4Header), 1, W4File);
  
  if (w4Header.Magic != W4MAGIC)
  {
    printf("Not a W4 file.\n");
    fclose(W3File);
    return 1;
  }

  // Allocate space for chunk table
  pChunkTable = malloc(w4Header.ChunkCount * 4);
  
  if (pChunkTable == NULL)
  {
    printf("Not enough memory to allocate chunk table.\n");
    return 1;
  }

  // Allocate space for source buffer
  pSrcBuffer = malloc(w4Header.ChunkSize);
  
  if (pSrcBuffer == NULL)
  {
    printf("Not enough memory for source buffer.\n");
    return 1;
  }
  
  // Allocate space for destination buffer
  pDestBuffer = malloc(w4Header.ChunkSize * 2);
  
  if (pDestBuffer == NULL)
  {
    printf("Not enough memory for destination buffer.\n");
    return 1;
  }
  
  // Read chunk table
  fread(pChunkTable, w4Header.ChunkCount, 4, W4File);

  // Pad W3File so that offsets in the list of VxDs
  // will match up.
  printf("Padding W3 File.\n");
  fwrite(&mzHeader, sizeof(mzHeader), 1, W3File);
  
  end = mzHeader.OtherOff - sizeof(mzHeader);
  for (i = 0; i < end; i++)
  {
    fputc(0, W3File);
  }

  for (nChunkIndex = 0; nChunkIndex < w4Header.ChunkCount; nChunkIndex++)
  {
    start = pChunkTable[nChunkIndex];
    
    if (nChunkIndex == w4Header.ChunkCount)
    {
      end = fseek(W4File, 0l, SEEK_END);
    }
    else
    {
      end = pChunkTable[nChunkIndex + 1];
    }

    printf("Decompressing chunk %d   -   Compressed Size %d\n", nChunkIndex, (end - start));    
    // Go to and read the current chunk
    // Note: This code assumes that the chunk size
    //       is not beyond the ability of fread.
    fseek(W4File, start, SEEK_SET);
    fread(pSrcBuffer, (WORD)(end - start), 1, W4File);

    // Fill destination buffer with clear marker
    memset(pDestBuffer, 0xE5, w4Header.ChunkSize);
    
    if ((WORD)(end - start) != w4Header.ChunkSize)
    {
      nDestSize = W4Decompress(pSrcBuffer, pDestBuffer, w4Header.ChunkSize);
      
      // This is an error condition.
      if (!nDestSize)
      {
        fclose(W3File);
        return 1;
      }
      
      fwrite(pDestBuffer, (WORD)nDestSize, 1, W3File);
    }
    else
    {
      fwrite(pSrcBuffer, (WORD)(end - start), 1, W3File);
    }
  }
  
  fclose(W3File);
  free(pChunkTable);
  free(pSrcBuffer);
  free(pDestBuffer);
}


void Usage(void)
{
  printf("Usage: W4DECOMP W4Name\n\n");
  printf("W4Name   is the name of the W4 executable, probably\n");
  printf("         VMM32.VXD\n");
}

int main(int argc, char *argv[]) 
{

  char  filename[256];
  FILE  *W4File;

  if (argc < 2) {
    Usage();
    return 1;
  }
  
  strcpy(filename, argv[1]);
  
  if (!strchr(filename, '.'))
    strcat(filename, ".EXE");
    
  if ((W4File = fopen(filename, "rb")) == NULL) 
  {
    printf("%s does not exist!\n", filename);
    return 1;
  }
  
  ExtractW3(W4File, filename);

  fclose(W4File);
  return 0;
}

