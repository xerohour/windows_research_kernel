/**********************************************************************
 *
 * PROGRAM: SETVAL.C
 *
 * PURPOSE: Modify a byte in a file
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 1, Introduction and Overview, from Undocumented Windows
 * File Formats, published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

#include <stdio.h>
#include <string.h>

void usage()
{
  printf("Usage: SETVAL fn addr val\n");
  printf("  Where addr is the hex location of the byte\n");
  printf("  in the file to modify and val is the byte (in hex)\n");
  printf("  to set at that location.\n\n");
  printf(" Example:  SETVAL LIST.TXT 4B3 FF\n\n");
}

int main(int argc, char *argv[])
{
  char filename[256];
  char address[10], val[10];
  long nAddr = 0, nVal = 0;
  FILE *fp;

  if (argc != 4)
  {
    usage();
    return 0;
  }
  strcpy(filename, argv[1]);

  if ((fp = fopen(filename, "r+b")) == NULL)
  {
    printf("Bad filename supplied\n");
    return 0;
  }

  strcpy(address, argv[2]);
  strcpy(val, argv[3]);

  sscanf(address, "%lx", &nAddr);
  sscanf(val, "%lx", &nVal);

  if (nVal < 0 || nVal > 255)
  {
    usage();
    printf("Error: Supply a val between 0 and 255\n");
  }

  fseek(fp, 0, SEEK_END);
  if (ftell(fp) < nAddr)
  {
    printf("The address supplied is beyond the end of this file.\n");
    printf("Please supply a value within the scope of this file.\n");
    fclose(fp);
    return 0;
  }

  fseek(fp, nAddr + 1, SEEK_SET);
  fwrite(&nVal, 1, 1, fp);
  fclose(fp);

  printf("File modified successfully.\n");
  return 0;
}
