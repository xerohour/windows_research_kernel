/**********************************************************************
 *
 * PROGRAM: PIFDUMP.C
 *
 * PURPOSE: This program extracts information from an MS Windows
 *          PIF file (either 386 or 286 mode)
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 8, PIF File Format, from Undocumented Windows File Formats,
 * published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "windows.h"
#include "pifstruc.h"

#define SUCCESS       0
#define ERROR_FOUND   1


/******************************************************************\
 *                                                                *
 * Check how the program was called.  Required parameters are     *
 * the mode of information to extract (-3 for 386, -2 for 286)    *
 * and the input filename.                                        *
 *                                                                *
\******************************************************************/
int check_usage(int argc, char *argv[]) {

   int dump_type=0;

   if (argc == 3) {

      if (!strcmp(argv[1], "-2"))
         dump_type = 2;
      else if (!strcmp(argv[1], "-3"))
         dump_type = 3;

   } /* if (argc == 3) - end */

   if (dump_type == 0) {
      printf("Usage:  pifdump < -3 | -2 > <infile>\n");
      exit(0);
   }

   return(dump_type);

} /* check_usage - end */


/******************************************************************\
 *                                                                *
 * Strip out trailing spaces from text_string.                    *
 *                                                                *
\******************************************************************/
void trim(char text_string[], int size) {

   /* check for a non-positive size */
   if (size < 1) return;

   /* find last non-blank character, then set next char. to null */
   for (--size; (--size >= 0) && (text_string[size] != ' '); ) {}
   text_string[++size] = '\0';

} /* trim - end */


/******************************************************************\
 *                                                                *
 * If test_flag is non-zero, print str1, else print str2.         *
 *                                                                *
\******************************************************************/
void print_flag(WORD test_flag, char *str1, char *str2) {

   if (test_flag)
      puts(str1);
   else
      puts(str2);

} /* print_flag - end */


/******************************************************************\
 *                                                                *
 * Convert the hotkey from the PIF file into a readable character.*
 *                                                                *
\******************************************************************/
void convert_hotkey( WORD hotkey, WORD num_flag) {

   switch(hotkey) {

      case 30: putchar('a');    break;
      case 48: putchar('b');    break;
      case 46: putchar('c');    break;
      case 32: putchar('d');    break;
      case 18: putchar('e');    break;
      case 33: putchar('f');    break;
      case 34: putchar('g');    break;
      case 35: putchar('h');    break;
      case 23: putchar('i');    break;
      case 36: putchar('j');    break;
      case 37: putchar('k');    break;
      case 38: putchar('l');    break;
      case 50: putchar('m');    break;
      case 49: putchar('n');    break;
      case 24: putchar('o');    break;
      case 25: putchar('p');    break;
      case 16: putchar('q');    break;
      case 19: putchar('r');    break;
      case 31: putchar('s');    break;
      case 20: putchar('t');    break;
      case 22: putchar('u');    break;
      case 47: putchar('v');    break;
      case 17: putchar('w');    break;
      case 45: putchar('x');    break;
      case 21: putchar('y');    break;
      case 44: putchar('z');    break;
      case 2: putchar('1');     break;
      case 3: putchar('2');     break;
      case 4: putchar('3');     break;
      case 5: putchar('4');     break;
      case 6: putchar('5');     break;
      case 7: putchar('6');     break;
      case 8: putchar('7');     break;
      case 9: putchar('8');     break;
      case 10: putchar('9');    break;
      case 11: putchar('0');    break;
      case 27: putchar(']');    break;
      case 26: putchar('[');    break;
      case 39: putchar(';');    break;
      case 40: putchar('\'');   break;
      case 41: putchar('`');    break;
      case 51: putchar(',');    break;
      case 52: putchar('.');    break;
      case 53: print_flag(num_flag, "Num /", "/"); break;
      case 12: putchar('-');    break;
      case 13: putchar('=');    break;
      case 43: putchar('\\');   break;
      case 59: puts("F1");      break;
      case 60: puts("F2");      break;
      case 61: puts("F3");      break;
      case 62: puts("F4");      break;
      case 63: puts("F5");      break;
      case 64: puts("F6");      break;
      case 65: puts("F7");      break;
      case 66: puts("F8");      break;
      case 67: puts("F9");      break;
      case 68: puts("F10");     break;
      case 87: puts("F11");     break;
      case 88: puts("F12");     break;
      case 78: puts("Num +");   break;
      case 69: puts("NumLock"); break;
      case 76: puts("Num 5");   break;
      case 74: puts("Num -");   break;

      case 82: print_flag(num_flag, "Insert", "Num 0");  break;
      case 70: print_flag(num_flag, "Break", "Scroll Lock");  break;
      case 71: print_flag(num_flag, "Home", "Num 7");  break;
      case 72: print_flag(num_flag, "Up", "Num 8");  break;
      case 73: print_flag(num_flag, "Page Up", "Num 9");  break;
      case 75: print_flag(num_flag, "Left", "Num 4");  break;
      case 77: print_flag(num_flag, "Right", "Num 6");  break;
      case 79: print_flag(num_flag, "End", "Num 1");  break;
      case 80: print_flag(num_flag, "Down", "Num 2");  break;
      case 81: print_flag(num_flag, "Page Down", "Num 3");  break;
      case 83: print_flag(num_flag, "Delete", "Num Del");  break;

      default: printf("<Unknown: %d>", hotkey); break;
   }
   putchar('\n');

} /* convert_hotkey - end */


/******************************************************************\
 *                                                                *
 * Use the 8 bits in the 286-Flags BYTE to fill in the 8 bytes of *
 * the FLAGS286 structure.                                        *
 *                                                                *
\******************************************************************/
void fill_flags286( BYTE flags286, FLAGS286 *f286_data) {

   /* The comment at the end of each line converts decimal to binary */
   f286_data->AltTab286    = (BYTE)(flags286 & 1);    /* 1   = 00000001 */
   f286_data->AltEsc286    = (BYTE)(flags286 & 2);    /* 2   = 00000010 */
   f286_data->AltPrtScr286 = (BYTE)(flags286 & 4);    /* 4   = 00000100 */
   f286_data->PrtScr286    = (BYTE)(flags286 & 8);    /* 8   = 00001000 */
   f286_data->CtrlEsc286   = (BYTE)(flags286 & 16);   /* 16  = 00010000 */
   f286_data->NoSaveScreen = (BYTE)(flags286 & 32);   /* 32  = 00100000 */
   f286_data->Unused10[0]  = (BYTE)(flags286 & 64);   /* 64  = 01000000 */
   f286_data->Unused10[1]  = (BYTE)(flags286 & 128);  /* 128 = 10000000 */

} /* fill_flags286 - end */


/******************************************************************\
 *                                                                *
 * Use the 8 bits in the COM Ports BYTE to fill in the 8 BYTES of *
 * the COMPORT structure.                                         *
 *                                                                *
\******************************************************************/
void fill_com_ports( BYTE comports, COMPORT *com_ports) {

   /* The comment at the end of each line converts decimal to binary */
   com_ports->Unused11[0]  = (BYTE)(comports & 1);    /* 1   = 00000001 */
   com_ports->Unused11[1]  = (BYTE)(comports & 2);    /* 2   = 00000010 */
   com_ports->Unused11[2]  = (BYTE)(comports & 4);    /* 4   = 00000100 */
   com_ports->Unused11[3]  = (BYTE)(comports & 8);    /* 8   = 00001000 */
   com_ports->Unused11[4]  = (BYTE)(comports & 16);   /* 16  = 00010000 */
   com_ports->Unused11[5]  = (BYTE)(comports & 32);   /* 32  = 00100000 */
   com_ports->Com3         = (BYTE)(comports & 64);   /* 64  = 01000000 */
   com_ports->Com4         = (BYTE)(comports & 128);  /* 128 = 10000000 */

} /* fill_com_ports - end */


/******************************************************************\
 *                                                                *
 * Use the 16 bits in the video[2] bytes to fill in the 16 bytes  *
 * of the VIDEO structure.                                        *
 *                                                                *
\******************************************************************/
void fill_video( BYTE video[2], VIDEO *video_data) {

   /* The comment at the end of each line converts decimal to binary */
   video_data->EmulateText  = (BYTE)(video[0] & 1);    /* 1   = 00000001 */
   video_data->MonitorText  = (BYTE)(video[0] & 2);    /* 2   = 00000010 */
   video_data->MonitorLoGr  = (BYTE)(video[0] & 4);    /* 4   = 00000100 */
   video_data->MonitorHiGr  = (BYTE)(video[0] & 8);    /* 8   = 00001000 */
   video_data->InitModeText = (BYTE)(video[0] & 16);   /* 16  = 00010000 */
   video_data->InitModeLoGr = (BYTE)(video[0] & 32);   /* 32  = 00100000 */
   video_data->InitModeHiGr = (BYTE)(video[0] & 64);   /* 64  = 01000000 */
   video_data->RetainVideo  = (BYTE)(video[0] & 128);  /* 128 = 10000000 */

   video_data->VideoUnused[0] = (BYTE)(video[1] & 1);    /* 1   = 00000001 */
   video_data->VideoUnused[1] = (BYTE)(video[1] & 2);    /* 2   = 00000010 */
   video_data->VideoUnused[2] = (BYTE)(video[1] & 4);    /* 4   = 00000100 */
   video_data->VideoUnused[3] = (BYTE)(video[1] & 8);    /* 8   = 00001000 */
   video_data->VideoUnused[4] = (BYTE)(video[1] & 16);   /* 16  = 00010000 */
   video_data->VideoUnused[5] = (BYTE)(video[1] & 32);   /* 32  = 00100000 */
   video_data->VideoUnused[6] = (BYTE)(video[1] & 64);   /* 64  = 01000000 */
   video_data->VideoUnused[7] = (BYTE)(video[1] & 128);  /* 128 = 10000000 */

} /* fill_video - end */


/******************************************************************\
 *                                                                *
 * Use the 16 bits in the hotkey[2] bytes to fill in the 16 bytes *
 * of the HOTKEY structure.                                       *
 *                                                                *
\******************************************************************/
void fill_hotkey( BYTE hotkey[2], HOTKEY *hotkey_data) {

   /* The comment at the end of each line converts decimal to binary */
   hotkey_data->HOT_KEYSHIFT = (BYTE)(hotkey[0] & 1);    /* 1   = 00000001 */
   hotkey_data->Unused4      = (BYTE)(hotkey[0] & 2);    /* 2   = 00000010 */
   hotkey_data->HOT_KEYCTRL  = (BYTE)(hotkey[0] & 4);    /* 4   = 00000100 */
   hotkey_data->HOT_KEYALT   = (BYTE)(hotkey[0] & 8);    /* 8   = 00001000 */
   hotkey_data->Unused5[0]   = (BYTE)(hotkey[0] & 16);   /* 16  = 00010000 */
   hotkey_data->Unused5[1]   = (BYTE)(hotkey[0] & 32);   /* 32  = 00100000 */
   hotkey_data->Unused5[2]   = (BYTE)(hotkey[0] & 64);   /* 64  = 01000000 */
   hotkey_data->Unused5[3]   = (BYTE)(hotkey[0] & 128);  /* 128 = 10000000 */

   hotkey_data->Unused5[4]   = (BYTE)(hotkey[0] & 1);    /* 1   = 00000001 */
   hotkey_data->Unused5[5]   = (BYTE)(hotkey[0] & 2);    /* 2   = 00000010 */
   hotkey_data->Unused5[6]   = (BYTE)(hotkey[0] & 4);    /* 4   = 00000100 */
   hotkey_data->Unused5[7]   = (BYTE)(hotkey[0] & 8);    /* 8   = 00001000 */
   hotkey_data->Unused5[8]   = (BYTE)(hotkey[0] & 16);   /* 16  = 00010000 */
   hotkey_data->Unused5[9]   = (BYTE)(hotkey[0] & 32);   /* 32  = 00100000 */
   hotkey_data->Unused5[10]  = (BYTE)(hotkey[0] & 64);   /* 64  = 01000000 */
   hotkey_data->Unused5[11]  = (BYTE)(hotkey[0] & 128);  /* 128 = 10000000 */

} /* fill_hotkey - end */


/******************************************************************\
 *                                                                *
 * Use the 16 bits in the flags_XMS[2] bytes to fill in the 16    *
 * bytes of the FLAGSXMS structure.                               *
 *                                                                *
\******************************************************************/
void fill_flagsxms( BYTE fxms[2], FLAGSXMS *xms_data) {

   /* The comment at the end of each line converts decimal to binary */
   xms_data->XMS_Locked    = (BYTE)(fxms[0] & 1);    /* 1   = 00000001 */
   xms_data->Allow_FastPst = (BYTE)(fxms[0] & 2);    /* 2   = 00000010 */
   xms_data->Lock_App      = (BYTE)(fxms[0] & 4);    /* 4   = 00000100 */
   xms_data->Unused3[0]    = (BYTE)(fxms[0] & 8);    /* 8   = 00001000 */
   xms_data->Unused3[1]    = (BYTE)(fxms[0] & 16);   /* 16  = 00010000 */
   xms_data->Unused3[2]    = (BYTE)(fxms[0] & 32);   /* 32  = 00100000 */
   xms_data->Unused3[3]    = (BYTE)(fxms[0] & 64);   /* 64  = 01000000 */
   xms_data->Unused3[4]    = (BYTE)(fxms[0] & 128);  /* 128 = 10000000 */

   xms_data->Unused3[5]    = (BYTE)(fxms[0] & 1);    /* 1   = 00000001 */
   xms_data->Unused3[6]    = (BYTE)(fxms[0] & 2);    /* 2   = 00000010 */
   xms_data->Unused3[7]    = (BYTE)(fxms[0] & 4);    /* 4   = 00000100 */
   xms_data->Unused3[8]    = (BYTE)(fxms[0] & 8);    /* 8   = 00001000 */
   xms_data->Unused3[9]    = (BYTE)(fxms[0] & 16);   /* 16  = 00010000 */
   xms_data->Unused3[10]   = (BYTE)(fxms[0] & 32);   /* 32  = 00100000 */
   xms_data->Unused3[11]   = (BYTE)(fxms[0] & 64);   /* 64  = 01000000 */
   xms_data->Unused3[12]   = (BYTE)(fxms[0] & 128);  /* 128 = 10000000 */

} /* fill_flagsxms - end */


/******************************************************************\
 *                                                                *
 * Use the 16 bits in the 386-Flags[2] bytes to fill in the 16    *
 * bytes of the FLAGS386 structure.                               *
 *                                                                *
\******************************************************************/
void fill_flags386( BYTE flags386[2], FLAGS386 *f386_data) {

   /* The comment at the end of each line converts decimal to binary */
   f386_data->AllowCloseAct = (BYTE)(flags386[0] & 1);   /* 1   = 00000001 */
   f386_data->BackgroundOn  = (BYTE)(flags386[0] & 2);   /* 2   = 00000010 */
   f386_data->ExclusiveOn   = (BYTE)(flags386[0] & 4);   /* 4   = 00000100 */
   f386_data->FullScreenYes = (BYTE)(flags386[0] & 8);   /* 8   = 00001000 */
   f386_data->Unused0       = (BYTE)(flags386[0] & 16);  /* 16  = 00010000 */
   f386_data->SK_AltTab     = (BYTE)(flags386[0] & 32);  /* 32  = 00100000 */
   f386_data->SK_AltEsc     = (BYTE)(flags386[0] & 64);  /* 64  = 01000000 */
   f386_data->SK_AltSpace   = (BYTE)(flags386[0] & 128); /* 128 = 10000000 */

   f386_data->SK_AltEnter   = (BYTE)(flags386[1] & 1);   /* 1   = 00000001 */
   f386_data->SK_AltPrtSc   = (BYTE)(flags386[1] & 2);   /* 2   = 00000010 */
   f386_data->SK_PrtSc      = (BYTE)(flags386[1] & 4);   /* 4   = 00000100 */
   f386_data->SK_CtrlEsc    = (BYTE)(flags386[1] & 8);   /* 8   = 00001000 */
   f386_data->Detect_Idle   = (BYTE)(flags386[1] & 16);  /* 16  = 00010000 */
   f386_data->UseHMA        = (BYTE)(flags386[1] & 32);  /* 32  = 00100000 */
   f386_data->Unused1       = (BYTE)(flags386[1] & 64);  /* 64  = 01000000 */
   f386_data->EMS_Locked    = (BYTE)(flags386[1] & 128); /* 128 = 10000000 */

} /* fill_flags386 - end */


/******************************************************************\
 *                                                                *
 * Use the 8 bits in the PIF close_on_exit BYTE to fill in the    *
 * 8 BYTES of the CLOSEONEXIT structure.                          *
 *                                                                *
\******************************************************************/
void fill_close_on_exit( BYTE close_on_exit, CLOSEONEXIT *coe_data) {

   /* The comment at the end of each line converts decimal to binary */
   coe_data->Unused0       = (BYTE)(close_on_exit & 1);  /* 1   = 00000001 */
   coe_data->Graph286      = (BYTE)(close_on_exit & 2);  /* 2   = 00000010 */
   coe_data->PreventSwitch = (BYTE)(close_on_exit & 4);  /* 4   = 00000100 */
   coe_data->NoScreenExch  = (BYTE)(close_on_exit & 8);  /* 8   = 00001000 */
   coe_data->Close_OnExit  = (BYTE)(close_on_exit & 16); /* 16  = 00010000 */
   coe_data->Unused1       = (BYTE)(close_on_exit & 32); /* 32  = 00100000 */
   coe_data->Com2          = (BYTE)(close_on_exit & 64); /* 64  = 01000000 */
   coe_data->Com1          = (BYTE)(close_on_exit & 128);/* 128 = 10000000 */

} /* fill_close_on_exit - end */


/******************************************************************\
 *                                                                *
 * Search the linked list of records at the end of the PIF file   *
 * for the section with a title of "title".                       *
 *                                                                *
\******************************************************************/
long search_pif_file( char *title, FILE *infile) {

   SECTIONHDR sect_hdr;
   SECTIONNAME sect_name;

   long offset=0L;
   short match_found=0;
   short at_eof=0;

   /* Skip over "MICROSOFT PIFEX" header - it points to the first part */
   /* of the file (in its sect_hdr structure, next_section = 0x187,    */
   /* current_section = 0x0, and size_section = 0x171; next_section =  */
   /* 0x171 (size of section) + 0x10 (size of "MICROSOFT PIFEX\0") +   */
   /* 0x06 (size of section header).                                   */
   fread( &sect_name, sizeof(sect_name), 1, infile);
   fread( &sect_hdr,  sizeof(sect_hdr),  1, infile);

   if (strcmp(sect_name.name_string, "MICROSOFT PIFEX")) {
      printf("Invalid PIF file.  Stopping.\n");
      exit(1);
   }

   /* Now scan through remaining sections in the PIF file */
   while ((!match_found) && (!at_eof)) {

      if (feof(infile))
         at_eof = 1;
      else {

         /* Read in the name and header of the current section */
         fread( &sect_name, sizeof(sect_name), 1, infile);
         fread( &sect_hdr,  sizeof(sect_hdr),  1, infile);

         /* Check if section found, or at eof, or if can't fseek to the */
         /* next section (these 3 actions are mutually exclusive)       */
         if (!strcmp(title, sect_name.name_string))
            match_found = 1;
         else if (sect_hdr.next_section == 0xFFFF)
            at_eof = 1;
         else {

            /* Convert a WORD to signed long */
            offset = 0x0000FFFF & sect_hdr.next_section;
            if (fseek(infile, offset, 0)) {
               printf("Unable to fseek to %ld.  Stopping.\n", offset);
               exit(1);
            }

         } /* if (section-not-found && not-at-end) - end */

      } /* if (not at end of file) - end */

   } /* while (match-not-found && not-at-end-of-file) - end */

   if (match_found)
      offset = sect_hdr.current_section;
   else
      offset = -1;

   return(offset);

} /* search_pif_file - end */


/******************************************************************\
 *                                                                *
 * Read the first 286 block from the PIF file.                    *
 *                                                                *
\******************************************************************/
void read_286_block( DATA286 *data, FILE *infile ) {

   long offset=0L;

   if ((offset = search_pif_file("WINDOWS 286 3.0", infile)) == -1) {
      printf("Error: 286 Section not found.  Stopping.\n");
      exit(1);
   }
   else {

      /* Read 286 section into memory */
      fseek(infile, offset, 0);
      fread( data, sizeof(DATA286), 1, infile);

   }

} /* read_286_block - end */


/******************************************************************\
 *                                                                *
 * Read the first 386 block from the PIF file.                    *
 *                                                                *
\******************************************************************/
void read_386_block( DATA386 *data, FILE *infile ) {

   long offset=0L;

   if ((offset = search_pif_file("WINDOWS 386 3.0", infile)) == -1) {
      printf("Error: 386 Section not found.  Stopping.\n");
      exit(1);
   }
   else {

      /* Read 386 section into memory */
      fseek(infile, offset, 0);
      fread( data, sizeof(DATA386), 1, infile);

   }

} /* read_386_block - end */


/******************************************************************\
 *                                                                *
 * Process the PIF input file.  If the user passes in "-2",       *
 * print out "Standard Mode" dump; for "-3", print "Enhanced"     *
 * mode dump.                                                     *
 *                                                                *
\******************************************************************/
void main(int argc, char *argv[]) {

   int  dump_type=0;    /* 2=286 dump, 3=386 dump */

   FILE *infile;

   PIF      pif_header;
   DATA386  data386;
   DATA286  data286;

   CLOSEONEXIT close_onexit_data;
   FLAGS286    f286_data;
   FLAGS386    f386_data;
   COMPORT     com_ports;
   VIDEO       video_data;
   HOTKEY      hotkey_data;
   FLAGSXMS    xms_data;

   /* Determine (from command line) if user wants 286 or */
   /* 386 info; set dump_type to 2 or 3, respectively.   */
   dump_type = check_usage(argc, argv);

   /* Try to open the file.  If it fails, exit. */
   if ((infile = fopen(argv[2], "rb")) == NULL) {
      printf("Input file not found.  Stopping.\n");
      exit(0);
   }

   /* fopen() was successful, so read in the first header */
   printf("Extracting %d86 information from %s.\n\n", dump_type, argv[2]);
   fread( &pif_header, sizeof(pif_header), 1, infile);

   /* Retrieve 286 or 386 (Standard or Enhanced) info from PIF file */
   if (dump_type == 2) {
      read_286_block( &data286, infile);

      fill_flags286(  data286.flags_286, &f286_data);
      fill_com_ports( data286.com_ports, &com_ports);
   }
   else {
      read_386_block( &data386, infile);

      fill_flags386(  data386.flags_386, &f386_data);
      fill_video( data386.video, &video_data);
      fill_hotkey( data386.hot_key_state, &hotkey_data);
      fill_flagsxms( data386.flags_XMS, &xms_data);
   }

   /* We're done with the input file, so close it */
   fclose(infile);

   /* Initialize bit-replacement structures */
   fill_close_on_exit( pif_header.close_on_exit, &close_onexit_data);

   /* Remove trailing spaces from text fields */
   trim(pif_header.prog_path,  sizeof(pif_header.prog_path));
   trim(pif_header.title,      sizeof(pif_header.title));
   trim(pif_header.def_dir,    sizeof(pif_header.def_dir));
   trim(pif_header.prog_param, sizeof(pif_header.prog_param));

   trim(pif_header.shared_prog_name, sizeof(pif_header.shared_prog_name));
   trim(pif_header.shared_data_file, sizeof(pif_header.shared_data_file));

   /* Print out the data common to both groups */
   printf("Program Filename   :  %s\n", pif_header.prog_path);
   printf("Window Title       :  %s\n", pif_header.title);

   /* print optional parameters based on dump_type */
   printf("Opt. Parameters    :  ");
   if (dump_type == 2)
      printf( "%s\n", pif_header.prog_param);
   else 
      printf( "%s\n", data386.opt_params);

   printf("Startup Directory  :  %s\n", pif_header.def_dir);

   /* print out video data */
   if (dump_type == 2) {
      printf("Video Mode         :  ");
      if (close_onexit_data.Graph286)
         printf("Graphics/Mult. Text\n");
      else
         printf("Text\n");
   }
   else if (dump_type == 3) {
      printf("Video Memory       :");
      if (video_data.InitModeText)
         printf("  Text");
      if (video_data.InitModeLoGr)
         printf("  Low Graphics");
      if (video_data.InitModeHiGr)
         printf("  High Graphics");
      printf("\n");
   }

   if (dump_type == 3) {

      printf("Memory Requirements:  %dK Required\t %dK Desired\n",
             data386.mem_req, data386.mem_limit);
      printf("EMS Memory         :  %dK Required\t %dK Limit\n",
             data386.ems_min, data386.ems_max);
      printf("XMS Memory         :  %dK Required\t %dK Limit\n",
             data386.xms_min, data386.xms_max);

      if (f386_data.FullScreenYes)
         printf("Display Usage      :  Full Screen\n");
      else
         printf("Display Usage      :  Windowed\n");

      printf("Execution          :  ");
      if (f386_data.BackgroundOn)
         printf("Background  ");
      else
         printf("Foreground  ");

      if (f386_data.ExclusiveOn)
         printf("Exclusive\n");
      else
         printf("Non-exclusive\n");

      if (close_onexit_data.Close_OnExit)
         printf("Close Window On Exit\n");
      else
         printf("Don't Close Window On Exit\n");

   }
   else if (dump_type == 2) {

      printf("Memory Requirements:  %dK Required\n",
             pif_header.min_mem);
      printf("XMS Memory         :  %dK Required\t %dK Limit\n\n",
             data286.xmsReq286, data286.xmsLimit286);

      printf("Directly Modifies:");
      if (close_onexit_data.Com1)
         printf("   Com1");
      if (close_onexit_data.Com2)
         printf("   Com2");
      if (com_ports.Com3)
         printf("   Com3");
      if (com_ports.Com4)
         printf("   Com4");
      if (pif_header.flags1 & 16)    /* Check the 5th bit for keyboard */
         printf("   Keyboard");
      printf("\n");

      if (close_onexit_data.NoScreenExch)
         printf("No Screen Exchange\n");
      else
         printf("Screen Exchange Allowed\n");

      if (close_onexit_data.PreventSwitch)
         printf("Program Switch Prevented\n");
      else
         printf("Program Switch Allowed\n");

      if (close_onexit_data.Close_OnExit)
         printf("Close Window On Exit\n");
      else
         printf("Don't Close Window On Exit\n");

      if (f286_data.NoSaveScreen)
         printf("Save Screen is not Enabled\n");
      else
         printf("Save Screen is Enabled\n");

      printf("Reserve Shortcut Keys:");
      if (f286_data.AltTab286)
         printf("  Alt+Tab");
      if (f286_data.AltEsc286)
         printf("  Alt+Esc");
      if (f286_data.CtrlEsc286)
         printf("  Ctrl+Esc");
      if (f286_data.PrtScr286)
         printf("  PrtScr");
      if (f286_data.AltPrtScr286)
         printf("  Alt+PrtScr");
      printf("\n");
   }

   /* Print out the 386 Advanced screen data */
   if (dump_type == 3) {

      printf("\nAdvanced Options:\n\n");

      printf("Multitasking Options:\n");
      printf("Background Priority: %d\n", data386.back_pri);
      printf("Foreground Priority: %d\n", data386.for_pri);
      if (f386_data.Detect_Idle)
         printf("Detect Idle Time\n\n");
      else
         printf("Do not Detect Idle Time\n\n");

      printf("Memory Options:\n");
      if (f386_data.EMS_Locked)
         printf("EMS Memory Locked\n");
      else
         printf("EMS Memory Not Locked\n");

      if (xms_data.XMS_Locked)
         printf("XMS Memory Locked\n");
      else
         printf("XMS Memory Not Locked\n");

      if (!f386_data.UseHMA)
         printf("Use High Memory\n");
      else
         printf("Do Not Use High Memory\n");

      if (xms_data.Lock_App)
         printf("Lock Application Memory\n");
      else
         printf("Do Not Lock Application Memory\n");

      printf("\nMonitor Ports:");
      if (!video_data.MonitorText)
         printf("   Text");
      if (!video_data.MonitorLoGr)
         printf("   Low Gr");
      if (!video_data.MonitorHiGr)
         printf("   High Gr");
      if (video_data.EmulateText)
         printf("   Emul. Text");
      if (video_data.RetainVideo)
         printf("   Retain Video Mem");
      printf("\n\n");

      if (xms_data.Allow_FastPst)
         printf("Allow Fast Paste\n");
      else
         printf("Don't Allow Fast Paste\n");

      if (f386_data.AllowCloseAct)
         printf("Allow Close When Active\n");
      else
         printf("Don't Allow Close When Active\n");

      printf("\nReserved Shortcut Keys\n");
      if (f386_data.SK_AltTab)
         printf("   Alt+Tab");
      if (f386_data.SK_AltEsc)
         printf("   Alt+Esc");
      if (f386_data.SK_CtrlEsc)
         printf("   Ctrl+Esc");
      if (f386_data.SK_PrtSc)
         printf("   PrtScr");
      if (f386_data.SK_AltPrtSc)
         printf("   Alt+PrtScr");
      if (f386_data.SK_AltSpace)
         printf("   Alt+Space");
      if (f386_data.SK_AltEnter)
         printf("   Alt+Enter");

      printf("\n\nApplication Shortcut Key: ");
      if (data386.hot_key_flag == 0)
         printf("None\n");
      else {
         if (hotkey_data.HOT_KEYALT)
            printf("Alt+");
         if (hotkey_data.HOT_KEYCTRL)
            printf("Ctrl+");
         if (hotkey_data.HOT_KEYSHIFT)
            printf("Shift+");

         convert_hotkey(data386.hot_key_scan, data386.hk_numflag);
      }

   } /* if (dump_type = 3) - end */

} /* main - end */

/* pifdump.c - end */

