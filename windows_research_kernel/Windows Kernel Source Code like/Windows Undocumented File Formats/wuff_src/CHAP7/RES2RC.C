/**********************************************************************
 *
 * PROGRAM: RES2RC.C
 *
 * PURPOSE: Converts a .RES file to an .RC file
 *
 * Copyright 1997, Mike Wallace and Pete Davis
 *
 * Chapter 7, Resource (.RES) File Format, from Undocumented Windows
 * File Formats, published by R&D Books, an imprint of Miller Freeman, Inc.
 *
 **********************************************************************/

#include "restypes.h"

/***************************************************
   Read a byte from infile
****************************************************/
BYTE get_byte(FILE *infile) {

    BYTE ch;

    fread(&ch, sizeof(BYTE), 1, infile);
    return(ch);

} /* get_byte - end */


/***************************************************
   Read a word from infile
****************************************************/
WORD get_word(FILE *infile) {

    WORD num;

    fread(&num, sizeof(WORD), 1, infile);
    return(num);

} /* get_word - end */


/***************************************************
   Write out an integer to outfile
****************************************************/
void write_number(int num, FILE *outfile) {

    fprintf( outfile, "%d ", num);

} /* write_number - end */


/***************************************************
   Write out a word (unsigned short integer) to outfile
****************************************************/
void write_word(WORD num, FILE *outfile) {

    fprintf( outfile, "%hu", num);

} /* write_word - end */


/***************************************************
   Write out a dword (unsigned long) to outfile
****************************************************/
void write_dword(DWORD num, FILE *outfile) {

    fprintf( outfile, "%lu", num);

} /* write_dword - end */


/***************************************************
   Write out a character to outfile
****************************************************/
void write_char(char ch, FILE *outfile) {

   /* if the character is null, write out the string "\0" */
   if (ch == '\0')
      fprintf( outfile, "\\0");
   else
      fprintf( outfile, "%c", ch);

} /* write_char - end */


/***************************************************
   Write out an unsigned character to outfile
****************************************************/
void write_byte(BYTE ch, FILE *outfile) {

    fputc( ch, outfile);

} /* write_byte - end */


/***************************************************
   Write out a string to outfile, then add a newline
****************************************************/
void writeline(char *string, FILE *outfile) {

   if(strlen(string))
       fputs( string, outfile);

   fputc( CR, outfile);
   fputc( NL, outfile);

} /* writeline - end */


/***************************************************
   Write out a string to outfile
****************************************************/
void writestring(char *string, FILE *outfile) {

   if(strlen(string))
       fputs( string, outfile);

} /* writestring - end */


/***************************************************
   Increase the INDENT variable value by INDENT_SPACES
****************************************************/
void increase_indent( void ) {

    INDENT += INDENT_SPACES;

} /* increase_indent - end */


/***************************************************
   Decrease the INDENT variable value by INDENT_SPACES
****************************************************/
void decrease_indent( void ) {

    if((INDENT -= INDENT_SPACES) < 0)
       INDENT = 0;

} /* decrease_indent - end */


/***************************************************
   Write out INDENT spaces to outfile
****************************************************/
void write_indent(FILE *outfile) {

    int n;

    for(n = INDENT; (n); --n)
       write_char(' ', outfile);

} /* write_indent - end */


/***************************************************
   If the last stringtable wasn't closed, do so now.   
****************************************************/
void finish_off_stringtable(FILE *outfile) {

   if (StringCount)
      writeline("}", outfile);

} /* finish_off_stringtable - end */


/***************************************************
   Get name of custom resource
****************************************************/
void get_custom_type( BYTE ch, FILE *infile, FILE *outfile) {

   while(ch != 0x00) {
      write_char(ch, outfile);
      ch = get_byte(infile);
   }

} /* get_custom_type - end */


/***************************************************
   Get the resource name from the .res file (infile)
****************************************************/
DWORD write_item_text(FILE *infile, FILE *outfile) {

   BYTE  ch;
   DWORD length;

   ch = get_byte( infile );
   if(ch == 0x00)
      return(1);
   writestring("\"", outfile);
   length = 1;
   while(ch != 0x00) {
      ++length;
      write_char(ch, outfile);
      ch = get_byte( infile );
   }
   writestring("\"", outfile);
   return(length);

} /* write_item_text - end */


/***************************************************
   Returns the first new filename of the form
   <prefix>#.<suffix>
****************************************************/
char *get_data_filename(char *prefix, char *suffix) {

   FILE *fp;
   WORD number;
   char fname[13];
   char fname_start[3];
   char fname_end[5];
   int  name_length;

   /* form the prefix to the new filename */
   name_length = (strlen(prefix) > 2) ? 2 : strlen(prefix);
   strncpy(fname_start, prefix, name_length);
   fname_start[name_length] = '\0';

   /* form the suffix to the new filename */
   fname_end[0] = '.';
   name_length = (strlen(suffix) > 3) ? 3 : strlen(suffix);
   strncat( fname_end, suffix, name_length);
   fname_end[name_length + 1] = '\0';

   /* Keep forming new filenames until we get to one we can't open */
   number = 0;
   sprintf( fname, "%s%u%s", fname_start, number, fname_end);
   while((number < 1000) && ((fp = fopen( fname, "r")) != NULL)) {
      fclose(fp);
      ++number;
      sprintf( fname, "%s%u%s", fname_start, number, fname_end);
   }

   if(number == 1000)
      fname[0] = '\0';

   return(fname);

} /* get_data_filename - end */


/***************************************************
   Write out reslen bytes of data to datafile
****************************************************/
void write_data( FILE *infile, FILE *datafile, DWORD reslen ) {

   BYTE ch;

   ch = get_byte(infile);
   while(reslen) {
      fputc( ch, datafile);
      if(--reslen)
         ch = get_byte(infile);
   }
   
} /* write_data - end */


/***************************************************
   Retrieves the length of the resource from the .res file
****************************************************/
DWORD get_resource_length(FILE *infile) {

   DWORD reslen;

   fread(&reslen, sizeof(DWORD), 1, infile);

   return(reslen);

} /* get_resource_length - end */


/***************************************************
   Get the resource name from the .res file (infile)
****************************************************/
void get_resource_name(FILE *infile) {

   int resname;
   BYTE ch;

   ch = get_byte(infile);
   if(ch == 0xFF) {
       fread(&resname, sizeof(int), 1, infile);
   }
   else {
      ch = get_byte(infile);
      while((!feof(infile)) && (ch != 0x00)) {
         ch = get_byte(infile);
      }
   }

} /* get_resource_name - end */


/***************************************************
   Get the version name from the .res file (infile)
****************************************************/
void get_version_name(FILE *infile) {

   int  count=1; /* record number of chars read */
   BYTE ch;

   ch = get_byte(infile);
   while((!feof(infile)) && (ch != 0x00)) {
      ++count;
      ch = get_byte(infile);
   }

   /* Ensure number of characters read is a multiple of 4.    */
   /* According to the MS documentation, this is the format.  */
   /* See "MS Windows 3.1 Programmer's Reference" Vol.4, p.99 */
   count = count % 4;
   count = (count > 0) ? (4 - count) : 0;
   while ((!feof(infile)) && (count > 0)) {
      ch = get_byte(infile);
      --count;
   }

} /* get_version_name - end */


/***************************************************
   Get the resource number from the .res file (infile)
****************************************************/
WORD get_resource_number(FILE *infile) {

   WORD resname = 0;
   BYTE ch;

   ch = get_byte(infile);
   if(ch == 0xFF) {
       resname = get_word( infile );
   }
   else {
      ch = get_byte(infile);
      while((!feof(infile)) && (ch != 0x00)) {
         ch = get_byte(infile);
      }
   }

   return(resname);

} /* get_resource_number - end */


/***************************************************
   Get the resource name from the .res file (infile)
****************************************************/
void write_resource_name(FILE *infile, FILE *outfile) {

   int resname;
   BYTE ch;

   writeline( "", outfile);

   ch = get_byte(infile);
   if(ch == 0xFF) {
       fread(&resname, sizeof(int), 1, infile);
       write_number(resname, outfile);
   }
   else {
      write_char(ch, outfile);
      ch = get_byte(infile);
      while((!feof(infile)) && (ch != 0x00)) {
         write_char(ch, outfile);
         ch = get_byte(infile);
      }
   }

} /* write_resource_name - end */


/***************************************************
   Get the version number from the .res file (infile)
****************************************************/
void write_version_number(FILE *infile, FILE *outfile) {

   int resname;
   BYTE ch;

   writeline( "", outfile);

   ch = get_byte(infile);
   if(ch == 0xFF) {
       fread(&resname, sizeof(int), 1, infile);
       if (resname == (int) VS_VERSION_INFO)
          writestring("VS_VERSION_INFO ", outfile);
       else
          write_number(resname, outfile);
   }
   else {
      write_char(ch, outfile);
      ch = get_byte(infile);
      while((!feof(infile)) && (ch != 0x00)) {
         write_char(ch, outfile);
         ch = get_byte(infile);
      }
   }

} /* write_version_number - end */


/***************************************************
   Get the memory flags for the current resource.
****************************************************/
void get_mem_flags(FILE *infile) {

   /* Save the flag in a global variable.  See the header file! */
   prev_mem_flag = get_word( infile );
   return;

} /* get_mem_flags - end */


/***************************************************
   Write the memory flag values for the "special"
   resources - cursors and icons. They're different
   than the rest in that 0x20 means "discardable",
   not "pure".
****************************************************/
void write_special_mem_flag_values( WORD memtype, FILE *outfile) {

   if(memtype & 0x40)
      writestring(" PRELOAD", outfile);
   else
      writestring(" LOADONCALL", outfile);

   if(memtype & 0x10)
      writestring(" MOVEABLE", outfile);
   else
      writestring(" FIXED", outfile);

   if(memtype & 0x20)
      writestring(" DISCARDABLE", outfile);

} /* write_special_mem_flag_values - end */


/***************************************************
   Write the memory flag values for the current resource.
****************************************************/
void write_mem_flag_values( WORD memtype, FILE *outfile) {

   if(memtype & 0x40)
      writestring(" PRELOAD", outfile);
   else
      writestring(" LOADONCALL", outfile);

   if(memtype & 0x10)
      writestring(" MOVEABLE", outfile);
   else
      writestring(" FIXED", outfile);

/* This one isn't really used - "PURE" doesn't seem to make a difference.

   if(memtype & 0x20)
      writestring(" PURE", outfile);
*/

   if(memtype & 0x1000)
      writestring(" DISCARDABLE", outfile);

} /* write_mem_flag_values - end */


/***************************************************
   Get the memory flags for the current resource.
****************************************************/
void write_mem_flags(FILE *infile, FILE *outfile) {

   WORD memtype;

   memtype = get_word( infile );
   write_mem_flag_values( memtype, outfile );

} /* write_mem_flags - end */


/***************************************************
  Process the cursor resource
****************************************************/
void process_cursor( FILE *infile ) {

   DWORD reslen;

   /* Skip it for now.  We'll come back and get the information   */
   /* when we hit the group cursor resource - that has the header */
   /* that has to go at the start of the .cur file.               */

   get_resource_name(infile);
   get_mem_flags(infile);
   reslen = get_resource_length(infile);

   fseek(infile, reslen, 1);

} /* process_cursor - end */


/***************************************************
  Process the bitmap resource
****************************************************/
void process_bitmap(int restype, FILE *infile, FILE *outfile) {

   DWORD reslen;
   BITMAPFILEHEADER bmfh;
   char  datafilename[13];
   FILE  *datafile;

   write_resource_name(infile, outfile);
   write_char(' ', outfile);
   writestring(res_array[restype], outfile);
   write_mem_flags(infile, outfile);
   write_char(' ', outfile);

   reslen = get_resource_length(infile);

   strcpy(datafilename, get_data_filename("BM", "BMP"));
   if(strlen(datafilename) == 0) {
      writeline("<Unable to open output file>", outfile);
      fseek( infile, reslen, 1);
      return;
   }

   writeline( datafilename, outfile );

   if((datafile = fopen(datafilename, "wb")) == NULL) {
      printf("Unable to open file %s\n", datafilename);
      fseek( infile, reslen, 1);
      return;
   }

   bmfh.bfType      = 0x4D42;
   bmfh.bfSize      = reslen + sizeof(BITMAPFILEHEADER);
   bmfh.bfReserved1 = 0;
   bmfh.bfReserved2 = 0;
   bmfh.bfOffBits   = 0x76L;

   fwrite(&bmfh, sizeof(BITMAPFILEHEADER), 1, datafile);

   write_data( infile, datafile, reslen );
   fclose( datafile );

} /* process_bitmap - end */


/***************************************************
  Process the flags for a popup menu
****************************************************/
void process_popup_flags(DWORD menuitem, FILE *outfile) {

   if(menuitem & MF_GRAYED)
      writestring(", GRAYED", outfile);
   if(menuitem & MF_DISABLED)
      writestring(", INACTIVE", outfile);
   if(menuitem & MF_CHECKED)
      writestring(", CHECKED", outfile);
   if(menuitem & MF_MENUBARBREAK)
      writestring(", MENUBARBREAK", outfile);
   if(menuitem & MF_MENUBREAK)
      writestring(", MENUBREAK", outfile);
   if(menuitem & MF_END) {
   }

} /* process_popup_flags - end */


/***************************************************
  Process the flags for a normal menuitem
****************************************************/
void process_menuitem_flags(DWORD menuitem, FILE *outfile) {

   if(menuitem & MF_GRAYED)
      writestring(", GRAYED", outfile);
   if(menuitem & MF_DISABLED)
      writestring(", INACTIVE", outfile);
   if(menuitem & MF_CHECKED)
      writestring(", CHECKED", outfile);
   if(menuitem & MF_MENUBARBREAK)
      writestring(", MENUBARBREAK", outfile);
   if(menuitem & MF_MENUBREAK)
      writestring(", MENUBREAK", outfile);
   if(menuitem & MF_HELP)
      writestring(", HELP", outfile);
   if(menuitem & MF_END) {
      writeline("", outfile);
      decrease_indent();
      write_indent(outfile);
      writestring("}", outfile);
   }

} /* process_menuitem_flags - end */


/***************************************************
  Process a normal menu resource
****************************************************/
DWORD process_normal_menu(DWORD menuitem, FILE *infile, FILE *outfile) {

   WORD  menuid;
   DWORD bytesread;

   fread(&menuid, sizeof(menuid), 1, infile);
   bytesread = sizeof(menuid);

   increase_indent();
   write_indent(outfile);

   writestring("MENUITEM ", outfile);
   bytesread += write_item_text( infile, outfile);
   if((bytesread - 1) > sizeof(menuid)) {
      writestring(", ", outfile);
      write_word( menuid, outfile);
      process_menuitem_flags(menuitem, outfile);
   }
   else if((!menuid) && (!menuitem)) {
      /* *** remove the leading quote from above */
      writestring("SEPARATOR", outfile);
   }
   writeline("", outfile);

   decrease_indent();

   return(bytesread);

} /* process_normal_menu - end */


/***************************************************
  Process a popup menu resource
****************************************************/
DWORD process_popup_menu(DWORD menuitem, FILE *infile, FILE *outfile) {

   DWORD bytesread;

   increase_indent();
   write_indent(outfile);

   writestring("POPUP ", outfile);
   bytesread = write_item_text( infile, outfile);
   process_popup_flags(menuitem, outfile);
   writeline(" {", outfile);

   return(bytesread);

} /* process_popup_menu - end */


/***************************************************
  Process the menu resource
****************************************************/
void process_menu(int restype, FILE *infile, FILE *outfile) {

   DWORD reslen;
   DWORD bytesread;
   WORD  menuitem;
   WORD  popupmenuitem;
   struct MenuHeader menuhdr;

   write_resource_name(infile, outfile);
   write_char(' ', outfile);
   writestring(res_array[restype], outfile);
   write_mem_flags(infile, outfile);
   writeline(" {", outfile);
   reslen = get_resource_length(infile);

   fread(&menuhdr, sizeof(struct MenuHeader), 1, infile);
   reslen -= sizeof(struct MenuHeader);

   fread(&menuitem, sizeof(menuitem), 1, infile);
   reslen -= sizeof(menuitem);
   while(reslen) {

      if(menuitem & MF_POPUP) {
          popupmenuitem = menuitem;
          bytesread = process_popup_menu(  menuitem, infile, outfile);
      }
      else
          bytesread = process_normal_menu( menuitem, infile, outfile);

      reslen -= bytesread;

      if(reslen) {
         fread(&menuitem, sizeof(menuitem), 1, infile);
         reslen -= sizeof(menuitem);
      }
      else if(popupmenuitem & MF_END) /* check last popup was the end */
          writeline("}", outfile);

   }

} /* process_menu - end */


/***************************************************
  Process the icon resource
****************************************************/
void process_icon( FILE *infile ) {

   DWORD reslen;

   /* Skip it for now.  We'll come back and get the information */
   /* when we hit the group icon resource - that has the header */
   /* that has to go at the start of the .ico file.             */

   get_resource_name(infile);
   get_mem_flags(infile);
   reslen = get_resource_length(infile);

   fseek(infile, reslen, 1);

} /* process_icon - end */


/***************************************************
  Write the dialog box size and shape information
****************************************************/
void write_dialog_sizes( DIALOGHEADER dlg_hdr, FILE *outfile) {

   write_char(' ', outfile);
   write_word( dlg_hdr.x, outfile );
   writestring(", ", outfile);
   write_word( dlg_hdr.y, outfile );
   writestring(", ", outfile);
   write_word( dlg_hdr.width, outfile );
   writestring(", ", outfile);
   write_word( dlg_hdr.height, outfile );
   writeline("", outfile);

} /* write_dialog_sizes - end */


/***************************************************
  Check if the style is true; if so, write it out
****************************************************/
void check_for_style( DWORD style, char *name, FILE *outfile ) {

   static WORD first_style = 1;
   static WORD line_size = 6;          /* length of 'STYLE ' */

   if (style) {

      if (first_style) {
         writestring("STYLE ", outfile);
         first_style = 0;
      }
      else {
         writestring(" | ", outfile);
         line_size += 3;               /* add lenght of ' | ' */
      }

      /* this is an attempt to keep the line length reasonable */
      if ((line_size + strlen(name)) > 75) {
         writeline("", outfile);
         line_size = 6;
         writestring("      ", outfile);
      }

      writestring( name, outfile );
      line_size += strlen(name);

   } /* if (style) - end */

} /* check_for_style - end */


/***************************************************
  Write the dialog box style information
****************************************************/
void write_dialog_style( DIALOGHEADER dlg_hdr, FILE *outfile) {

   DWORD style;

   style = dlg_hdr.lStyle;

   check_for_style( style & WS_OVERLAPPED,   "WS_OVERLAPPED",   outfile );
   check_for_style( style & WS_POPUP,        "WS_POPUP",        outfile );
   check_for_style( style & WS_CHILD,        "WS_CHILD",        outfile );
   check_for_style( style & WS_CLIPSIBLINGS, "WS_CLIPSIBLINGS", outfile );
   check_for_style( style & WS_CLIPCHILDREN, "WS_CLIPCHILDREN", outfile );
   check_for_style( style & WS_VISIBLE,      "WS_VISIBLE",      outfile );
   check_for_style( style & WS_DISABLED,     "WS_DISABLED",     outfile );
   check_for_style( style & WS_MINIMIZE,     "WS_MINIMIZE",     outfile );
   check_for_style( style & WS_MAXIMIZE,     "WS_MAXIMIZE",     outfile );

   check_for_style( style & WS_BORDER,      "WS_BORDER",      outfile );
   check_for_style( style & WS_DLGFRAME,    "WS_DLGFRAME",    outfile );
   check_for_style( style & WS_VSCROLL,     "WS_VSCROLL",     outfile );
   check_for_style( style & WS_HSCROLL,     "WS_HSCROLL",     outfile );
   check_for_style( style & WS_SYSMENU,     "WS_SYSMENU",     outfile );
   check_for_style( style & WS_THICKFRAME,  "WS_THICKFRAME",  outfile );
   check_for_style( style & WS_MINIMIZEBOX, "WS_MINIMIZEBOX", outfile );
   check_for_style( style & WS_MAXIMIZEBOX, "WS_MAXIMIZEBOX", outfile );

   check_for_style( style & WS_GROUP,          "WS_GROUP",          outfile );
   check_for_style( style & WS_TABSTOP,        "WS_TABSTOP",        outfile );
   check_for_style( style & WS_EX_TOPMOST,     "WS_EX_TOPMOST",     outfile );
   check_for_style( style & WS_EX_ACCEPTFILES, "WS_EX_ACCEPTFILES", outfile );

   check_for_style( style & WS_EX_NOPARENTNOTIFY, "WS_EX_NOPARENTNOTIFY",
                    outfile );

   check_for_style( style & DS_ABSALIGN,   "DS_ABSALIGN",   outfile );
   check_for_style( style & DS_SYSMODAL,   "DS_SYSMODAL",   outfile );
   check_for_style( style & DS_LOCALEDIT,  "DS_LOCALEDIT",  outfile );
   check_for_style( style & DS_SETFONT,    "DS_SETFONT",    outfile );
   check_for_style( style & DS_MODALFRAME, "DS_MODALFRAME", outfile );
   check_for_style( style & DS_NOIDLEMSG,  "DS_NOIDLEMSG",  outfile );

   writeline("", outfile);

} /* write_dialog_style - end */


/***************************************************
  Write the dialog box menu name
****************************************************/
void write_dialog_menu( FILE *infile, FILE *outfile) {

   BYTE ch;
   WORD menuid;

   /* Read the first character and check for non-zero start byte */
   ch = get_byte(infile);

   /* if first byte is 0x00, no menu name */
   if(ch != 0x00) {

      writestring("MENU ", outfile);
      if(ch == 0xFF) {
         /* menu id is a number */
         menuid = get_word( infile );
         write_word(menuid, outfile);
      }
      else
         get_custom_type( ch, infile, outfile);

      writeline("", outfile);
   }

} /* write_dialog_menu - end */


/***************************************************
  Write the dialog box class name
****************************************************/
void write_dialog_class( FILE *infile, FILE *outfile) {

   BYTE ch;

   /* Read the first character and check for non-zero start byte */
   ch = get_byte(infile);

   /* get the resource type */
   if(ch != 0x00) {
      writestring("CLASS \"", outfile);
      get_custom_type( ch, infile, outfile);
      writeline("\"", outfile);
   }

} /* write_dialog_class - end */


/***************************************************
  Write the dialog box font information
****************************************************/
void write_dialog_font( FILE *infile, FILE *outfile) {

   WORD pointsize;

   writestring("FONT ", outfile);

   /* read and write the font point size */
   pointsize = get_word( infile );
   write_word( pointsize, outfile);
   writestring(", ", outfile);

   /* write the font name */
   (void) write_item_text( infile, outfile);

   writeline("", outfile);

} /* write_dialog_font - end */


/***************************************************
  Write the dialog box caption
****************************************************/
void write_dialog_caption( FILE *infile, FILE *outfile) {

   BYTE ch;

   /* Read the first character and check for non-zero start byte */
   ch = get_byte(infile);

   /* get the resource type */
   if(ch != 0x00) {
      writestring("CAPTION \"", outfile);
      get_custom_type( ch, infile, outfile);
      writeline("\"", outfile);
   }

} /* write_dialog_caption - end */


/***************************************************
  Write the start of the "CONTROL" string 
****************************************************/
void write_control_header( char *text, WORD id, FILE *outfile) {

   write_indent(outfile);

   writestring("CONTROL \"", outfile);
   writestring( text, outfile);
   writestring( "\", ", outfile);

   /* check if id is -1 */
   if (id != 0xFFFF)
      write_word( id, outfile);
   else
      writestring( "-1", outfile);

   writestring( ", ", outfile);

} /* write_control_header - end */


/***************************************************
  Write the end of the "CONTROL" statement.
****************************************************/
void write_control_end( CONTROLDATA ctrl, FILE *outfile) {

   char string[50];

   /* write the size/dimensions of the object */
   write_indent(outfile);
   sprintf( string, "%hu, %hu, %hu, %hu", ctrl.x, ctrl.y, 
            ctrl.width, ctrl.height);
   writeline( string, outfile);

} /* write_control_end - end */


/***************************************************
  Write the style for the dialog box
****************************************************/
void check_for_dlg_style( DWORD style, WORD *first_style, WORD *line_len,
                       char *style_name, FILE *outfile ) {

   if (style) {

      /* if first_style = 2, write out a leading comma */
      if (*first_style == 2) {
         writestring(", ", outfile);
         *first_style = 1;
      }

      if (*first_style == 1) {

         /* this is the first style, so just write out the style name */
         writestring( style_name, outfile);
         *first_style = 0;
         *line_len += strlen(style_name);

      }
      else {

         /* this is after first style, so write '|' for concat */
         writestring(" | ", outfile);
         *line_len += (3 + strlen(style_name));

         /* try to keep each line to a reasonable length */
         if (*line_len >= 75) {
            writeline("", outfile);
            write_indent(outfile);
            *line_len = strlen(style_name);
         }

         /* write out the name of the style */
         writestring(style_name, outfile);

      } /* if (*first_style) / else - end */

   } /* if (style) - end */

} /* check_for_dlg_style - end */


/***************************************************
  Process the button control in the dialog box
****************************************************/
void process_control_button( DWORD style, FILE *outfile ) {

   WORD first_style = 1;
   WORD line_len = 0;

   writestring( "\"button\", ", outfile);

   check_for_dlg_style( BS_LEFTTEXT & style, &first_style, &line_len,
                          "BS_LEFTTEXT", outfile );
   check_for_dlg_style( WS_TABSTOP & style, &first_style, &line_len,
                          "WS_TABSTOP", outfile );
   check_for_dlg_style( WS_GROUP & style, &first_style, &line_len,
                          "WS_GROUP", outfile );
   check_for_dlg_style( WS_DISABLED & style, &first_style, &line_len,
                          "WS_DISABLED", outfile );

   if(first_style == 0)
      writestring(" | ", outfile);

   /* checked for non-exclusive properties, now clear out high bits */
   style &= 0xF;

   if (style == BS_DEFPUSHBUTTON)
      writestring("BS_DEFPUSHBUTTON", outfile);
   else if (style == BS_CHECKBOX)
      writestring("BS_CHECKBOX", outfile);
   else if (style == BS_AUTOCHECKBOX)
      writestring("BS_AUTOCHECKBOX", outfile);
   else if (style == BS_RADIOBUTTON)
      writestring("BS_RADIOBUTTON", outfile);
   else if (style == BS_3STATE)
      writestring("BS_3STATE", outfile);
   else if (style == BS_AUTO3STATE)
      writestring("BS_AUTO3STATE", outfile);
   else if (style == BS_GROUPBOX)
      writestring("BS_GROUPBOX", outfile);
   else if (style == BS_USERBUTTON)
      writestring("BS_USERBUTTON", outfile);
   else if (style == BS_AUTORADIOBUTTON)
      writestring("BS_AUTORADIOBUTTON", outfile);
   else if (style == BS_OWNERDRAW)
      writestring("BS_OWNERDRAW", outfile);
   else
      writestring("BS_PUSHBUTTON", outfile);

   writeline(",", outfile);

} /* process_control_button - end */


/***************************************************
  Process the edit control in the dialog box
****************************************************/
void process_control_edit( CONTROLDATA ctrl, FILE *outfile ) {

   WORD first_style = 2;
   char string[100];
   WORD line_len = 0;
   DWORD style;

   style = ctrl.lStyle;

   write_indent(outfile);
   sprintf( string, "EDITTEXT %hu, %hu, %hu, %hu, %hu", ctrl.id, ctrl.x, 
            ctrl.y, ctrl.width, ctrl.height);
   line_len = strlen(string);
   writestring( string, outfile);

   check_for_dlg_style( ES_LEFT & style, &first_style, &line_len,
                          "ES_LEFT", outfile );
   check_for_dlg_style( ES_CENTER & style, &first_style, &line_len,
                          "ES_CENTER", outfile );
   check_for_dlg_style( ES_RIGHT & style, &first_style, &line_len,
                          "ES_RIGHT", outfile );
   check_for_dlg_style( ES_MULTILINE & style, &first_style, &line_len,
                          "ES_MULTILINE", outfile );
   check_for_dlg_style( ES_UPPERCASE & style, &first_style, &line_len,
                          "ES_UPPERCASE", outfile );
   check_for_dlg_style( ES_LOWERCASE & style, &first_style, &line_len,
                          "ES_LOWERCASE", outfile );
   check_for_dlg_style( ES_PASSWORD & style, &first_style, &line_len,
                          "ES_PASSWORD", outfile );
   check_for_dlg_style( ES_AUTOVSCROLL & style, &first_style, &line_len,
                          "ES_AUTOVSCROLL", outfile );
   check_for_dlg_style( ES_AUTOHSCROLL & style, &first_style, &line_len,
                          "ES_AUTOHSCROLL", outfile );
   check_for_dlg_style( ES_NOHIDESEL & style, &first_style, &line_len,
                          "ES_NOHIDESEL", outfile );
   check_for_dlg_style( ES_OEMCONVERT & style, &first_style, &line_len,
                          "ES_OEMCONVERT", outfile );
   check_for_dlg_style( ES_READONLY & style, &first_style, &line_len,
                          "ES_READONLY", outfile );
   check_for_dlg_style( ES_WANTRETURN & style, &first_style, &line_len,
                          "ES_WANTRETURN", outfile );

   check_for_dlg_style( WS_TABSTOP & style, &first_style, &line_len,
                          "WS_TABSTOP", outfile );
   check_for_dlg_style( WS_GROUP & style, &first_style, &line_len,
                          "WS_GROUP", outfile );
   check_for_dlg_style( WS_VSCROLL & style, &first_style, &line_len,
                          "WS_VSCROLL", outfile );
   check_for_dlg_style( WS_HSCROLL & style, &first_style, &line_len,
                          "WS_HSCROLL", outfile );
   check_for_dlg_style( WS_DISABLED & style, &first_style, &line_len,
                          "WS_DISABLED", outfile );

   writeline("", outfile);

} /* process_control_edit - end */


/***************************************************
  Process the static control in the dialog box
****************************************************/
void process_control_static( DWORD style, FILE *outfile ) {

   WORD first_style = 1;
   WORD line_len = 0;

   writestring( "\"static\", ", outfile);

   check_for_dlg_style( SS_NOPREFIX & style, &first_style, &line_len,
                          "SS_NOPREFIX", outfile );
   check_for_dlg_style( WS_GROUP & style, &first_style, &line_len,
                          "WS_GROUP", outfile );
   check_for_dlg_style( WS_TABSTOP & style, &first_style, &line_len,
                          "WS_TABSTOP", outfile );

   if(first_style == 0)
      writestring(" | ", outfile);

   style &= 0xF;

   if ( style == SS_CENTER )
      writestring( "SS_CENTER", outfile);
   else if ( style == SS_RIGHT )
      writestring( "SS_RIGHT", outfile);
   else if ( style == SS_ICON )
      writestring( "SS_ICON", outfile);
   else if ( style == SS_BLACKRECT )
      writestring( "SS_BLACKRECT", outfile);
   else if ( style == SS_GRAYRECT )
      writestring( "SS_GRAYRECT", outfile);
   else if ( style == SS_WHITERECT )
      writestring( "SS_WHITERECT", outfile);
   else if ( style == SS_BLACKFRAME )
      writestring( "SS_BLACKFRAME", outfile);
   else if ( style == SS_GRAYFRAME )
      writestring( "SS_GRAYFRAME", outfile);
   else if ( style == SS_WHITEFRAME )
      writestring( "SS_WHITEFRAME", outfile);
   else if ( style == SS_SIMPLE )
      writestring( "SS_SIMPLE", outfile);
   else if ( style == SS_LEFTNOWORDWRAP )
      writestring( "SS_LEFTNOWORDWRAP", outfile);
   else
      writestring( "SS_LEFT", outfile);

   writeline(",", outfile);

} /* process_control_static - end */


/***************************************************
  Process the list box control in the dialog box
****************************************************/
void process_control_listbox( CONTROLDATA ctrl, FILE *outfile ) {

   WORD first_style = 2;
   char string[100];
   WORD line_len = 0;
   DWORD style;

   style = ctrl.lStyle;

   write_indent(outfile);
   sprintf( string, "LISTBOX %hu, %hu, %hu, %hu, %hu", ctrl.id, ctrl.x, 
            ctrl.y, ctrl.width, ctrl.height);
   line_len = strlen(string);
   writestring( string, outfile);

   check_for_dlg_style( LBS_NOTIFY & style, &first_style, &line_len,
                          "LBS_NOTIFY", outfile );
   check_for_dlg_style( LBS_SORT & style, &first_style, &line_len,
                          "LBS_SORT", outfile );
   check_for_dlg_style( LBS_NOREDRAW & style, &first_style, &line_len,
                          "LBS_NOREDRAW", outfile );
   check_for_dlg_style( LBS_MULTIPLESEL & style, &first_style, &line_len,
                          "LBS_MULTIPLESEL", outfile );
   check_for_dlg_style( LBS_OWNERDRAWFIXED & style, &first_style, &line_len,
                          "LBS_OWNERDRAWFIXED", outfile );
   check_for_dlg_style( LBS_OWNERDRAWVARIABLE & style, &first_style, &line_len,
                          "LBS_OWNERDRAWVARIABLE", outfile );
   check_for_dlg_style( LBS_HASSTRINGS & style, &first_style, &line_len,
                          "LBS_HASSTRINGS", outfile );
   check_for_dlg_style( LBS_USETABSTOPS & style, &first_style, &line_len,
                          "LBS_USETABSTOPS", outfile );
   check_for_dlg_style( LBS_NOINTEGRALHEIGHT & style, &first_style, &line_len,
                          "LBS_NOINTEGRALHEIGHT", outfile );
   check_for_dlg_style( LBS_MULTICOLUMN & style, &first_style, &line_len,
                          "LBS_MULTICOLUMN", outfile );
   check_for_dlg_style( LBS_WANTKEYBOARDINPUT & style, &first_style, &line_len,
                          "LBS_WANTKEYBOARDINPUT", outfile );
   check_for_dlg_style( LBS_EXTENDEDSEL & style, &first_style, &line_len,
                          "LBS_EXTENDEDSEL", outfile );
   check_for_dlg_style( LBS_DISABLENOSCROLL & style, &first_style, &line_len,
                          "LBS_DISABLENOSCROLL", outfile );

   check_for_dlg_style( WS_BORDER & style, &first_style, &line_len,
                          "WS_BORDER", outfile );
   check_for_dlg_style( WS_VSCROLL & style, &first_style, &line_len,
                          "WS_VSCROLL", outfile );

   writeline("", outfile);

} /* process_control_listbox - end */


/***************************************************
  Process the scroll bar control in the dialog box
****************************************************/
void process_control_scrollbar( CONTROLDATA ctrl, FILE *outfile ) {

   WORD first_style = 2;
   char string[100];
   WORD line_len = 0;
   DWORD style;

   style = ctrl.lStyle;

   write_indent(outfile);
   sprintf( string, "SCROLLBAR %hu, %hu, %hu, %hu, %hu", ctrl.id, ctrl.x, 
            ctrl.y, ctrl.width, ctrl.height);
   line_len = strlen(string);
   writestring( string, outfile);

   check_for_dlg_style( SBS_HORZ & style, &first_style, &line_len,
                          "SBS_HORZ", outfile );
   check_for_dlg_style( SBS_VERT & style, &first_style, &line_len,
                          "SBS_VERT", outfile );
   check_for_dlg_style( SBS_TOPALIGN & style, &first_style, &line_len,
                          "SBS_TOPALIGN", outfile );
   check_for_dlg_style( SBS_BOTTOMALIGN & style, &first_style, &line_len,
                          "SBS_BOTTOMALIGN", outfile );
   check_for_dlg_style( SBS_SIZEBOX & style, &first_style, &line_len,
                          "SBS_SIZEBOX", outfile );

   check_for_dlg_style( WS_TABSTOP & style, &first_style, &line_len,
                          "WS_TABSTOP", outfile );
   check_for_dlg_style( WS_GROUP & style, &first_style, &line_len,
                          "WS_GROUP", outfile );
   check_for_dlg_style( WS_DISABLED & style, &first_style, &line_len,
                          "WS_DISABLED", outfile );

   writeline("", outfile);

} /* process_control_scrollbar - end */


/***************************************************
  Process the combo box control in the dialog box
****************************************************/
void process_control_combobox( CONTROLDATA ctrl, FILE *outfile) {

   WORD first_style = 2;
   WORD line_len = 0;
   char string[100];
   DWORD style;

   style = ctrl.lStyle;

   write_indent(outfile);
   sprintf( string, "COMBOBOX %hu, %hu, %hu, %hu, %hu", ctrl.id, ctrl.x, 
            ctrl.y, ctrl.width, ctrl.height);
   line_len = strlen(string);
   writestring( string, outfile);

   check_for_dlg_style( CBS_SIMPLE & style, &first_style, &line_len,
                          "CBS_SIMPLE", outfile );
   check_for_dlg_style( CBS_DROPDOWN & style, &first_style, &line_len,
                          "CBS_DROPDOWN", outfile );
   check_for_dlg_style( CBS_OWNERDRAWFIXED & style, &first_style, &line_len,
                          "CBS_OWNERDRAWFIXED", outfile );
   check_for_dlg_style( CBS_OWNERDRAWVARIABLE & style, &first_style, &line_len,
                          "CBS_OWNERDRAWVARIABLE", outfile );
   check_for_dlg_style( CBS_AUTOHSCROLL & style, &first_style, &line_len,
                          "CBS_AUTOHSCROLL", outfile );
   check_for_dlg_style( CBS_OEMCONVERT & style, &first_style, &line_len,
                          "CBS_OEMCONVERT", outfile );
   check_for_dlg_style( CBS_SORT & style, &first_style, &line_len,
                          "CBS_SORT", outfile );
   check_for_dlg_style( CBS_HASSTRINGS & style, &first_style, &line_len,
                          "CBS_HASSTRINGS", outfile );
   check_for_dlg_style( CBS_NOINTEGRALHEIGHT & style, &first_style, &line_len,
                          "CBS_NOINTEGRALHEIGHT", outfile );
   check_for_dlg_style( CBS_DISABLENOSCROLL & style, &first_style, &line_len,
                          "CBS_DISABLENOSCROLL", outfile );

   check_for_dlg_style( WS_TABSTOP & style, &first_style, &line_len,
                          "WS_TABSTOP", outfile );
   check_for_dlg_style( WS_GROUP & style, &first_style, &line_len,
                          "WS_GROUP", outfile );
   check_for_dlg_style( WS_VSCROLL & style, &first_style, &line_len,
                          "WS_VSCROLL", outfile );
   check_for_dlg_style( WS_DISABLED & style, &first_style, &line_len,
                          "WS_DISABLED", outfile );

   writeline("", outfile);

} /* process_control_combobox - end */


/***************************************************
  Process each control in the dialog box
****************************************************/
void process_control( FILE *infile, FILE *outfile ) {

   CONTROLDATA ctrl;
   BYTE class_id;
   BYTE ch;
   char ctrl_text[260];
   char ctrl_class[260];
   WORD index = 0;

   fread( &ctrl, sizeof(CONTROLDATA), 1, infile);

   /* read the class type (if 0x8?) or the string (otherwise) */
   ch = get_byte(infile);
   if (ch & 0x80) {
      class_id = ch;
   }
   else {
      class_id = 0x00;
      while (ch != 0x00) {
         if (index < 260) {
            ctrl_class[index] = ch;
            ++index;
         }
         ch = get_byte(infile);
      }
   }
   ctrl_class[index] = '\0';

   /* read the text field */
   ch = get_byte(infile);
   index = 0;
   while (ch != 0x00) {
      if (index < 260) {
         ctrl_text[index] = ch;
         ++index;
      }
      ch = get_byte(infile);
   }
   ctrl_text[index] = '\0';

   /* read the extra 0x00 */
   ch = get_byte(infile);
   if (ch != 0x00) {
      writestring("Error ** ch =>", outfile);
      write_char( ch, outfile);
      writeline("< - should be 00", outfile);
   }

   increase_indent();

   if (class_id & 0x80) {

      switch (class_id) {

         /* Control is a button */
         case 0x80: 
            write_control_header( ctrl_text, ctrl.id, outfile);
            process_control_button( ctrl.lStyle, outfile );
            write_control_end( ctrl, outfile);
            break;

         /* Control is an edit widget */
         case 0x81: 
            process_control_edit( ctrl, outfile );
            break;

         /* Control is a static widget */
         case 0x82: 
            write_control_header( ctrl_text, ctrl.id, outfile);
            process_control_static( ctrl.lStyle, outfile );
            write_control_end( ctrl, outfile);
            break;

         /* Control is a listbox */
         case 0x83: 
            process_control_listbox( ctrl, outfile );
            break;

         /* Control is a scrollbar */
         case 0x84: 
            process_control_scrollbar( ctrl, outfile );
            break;

         /* Control is a combobox */
         case 0x85: 
            process_control_combobox( ctrl, outfile );
            break;

         default: 
            break;  /* Unknown type, so skip */
      }

   }
   else {
      ; /* The resource type is unknown, so skip */
   }

   decrease_indent();

} /* process_control - end */


/***************************************************
  Process the dialog resource
****************************************************/
void process_dialog(int restype, FILE *infile, FILE *outfile) {

   DIALOGHEADER dlg_hdr;

   /* write the generic dialog box info */
   write_resource_name(infile, outfile);
   write_char(' ', outfile);
   writestring(res_array[restype], outfile);
   write_mem_flags(infile, outfile);

   /* get the dialog box resource length, and the dialog box header */
   (void) get_resource_length(infile);
   fread( &dlg_hdr, sizeof(DIALOGHEADER), 1, infile);

   write_dialog_sizes( dlg_hdr, outfile);
   write_dialog_style( dlg_hdr, outfile);
   write_dialog_menu(  infile,  outfile);
   write_dialog_class( infile,  outfile);
   write_dialog_caption( infile, outfile);

   if (dlg_hdr.lStyle & DS_SETFONT)
      write_dialog_font( infile, outfile);

   writeline("BEGIN", outfile);

   while(dlg_hdr.bNumberOfItems) {

      process_control( infile, outfile );

      dlg_hdr.bNumberOfItems -= 1;
   }


   writeline("END", outfile);

} /* process_dialog - end */


/***************************************************
  Process the string resource
****************************************************/
void process_string(int restype, FILE *infile, FILE *outfile) {

   WORD  sID;
   int   index;
   BYTE  ch;
   BYTE  strlen;

   sID = get_resource_number(infile);
   if (StringCount == 0) {
      writeline( "", outfile);
      writestring(res_array[restype], outfile);
      write_mem_flags(infile, outfile);
      writeline(" {", outfile);
   }
   else {
      get_mem_flags(infile);
   }

   ++StringCount;

   increase_indent();

   (void) get_resource_length(infile);

   for(index = 0; index < 16; ++index) {
      if((strlen = get_byte(infile)) != 0x00) {
         write_indent(outfile);
         sID = index + ((sID - 1) * 16);
         write_word( sID, outfile);
         writestring(", \"", outfile);
         while(strlen--) {
            ch = get_byte(infile);
            write_char(ch, outfile);
         }
         writeline("\"", outfile);
      }
   }

   decrease_indent();

   if(StringCount >= 16) {
      writeline("}", outfile);
      StringCount = 0;
   }

} /* process_string - end */


/***************************************************
  Process the fontdir resource
****************************************************/
void process_fontdir( FILE *infile ) {

   DWORD reslen;

   get_resource_name(infile);
   get_mem_flags(infile);
   reslen = get_resource_length(infile);

   fseek(infile, reslen, 1);

} /* process_fontdir - end */


/***************************************************
  Process the font resource
****************************************************/
void process_font(int restype, FILE *infile, FILE *outfile) {

   DWORD reslen;
   char  datafilename[13];
   FILE  *datafile;

   write_resource_name(infile, outfile);
   write_char(' ', outfile);
   writestring(res_array[restype], outfile);
   write_mem_flags(infile, outfile);
   write_char(' ', outfile);

   reslen = get_resource_length(infile);

   strcpy(datafilename, get_data_filename("FO", "FON"));
   if(strlen(datafilename) == 0) {
      writeline("<Unable to open output file>", outfile);
      fseek( infile, reslen, 1);
      return;
   }

   writeline( datafilename, outfile );

   if((datafile = fopen(datafilename, "wb")) == NULL) {
      printf("Unable to open file %s\n", datafilename);
      fseek( infile, reslen, 1);
      return;
   }

   write_data( infile, datafile, reslen );
   fclose( datafile );

} /* process_font - end */


/***************************************************
  Write the virtual character for the current accelerator
****************************************************/
void write_virtual_accel_event( WORD wEvent, FILE *outfile) {

   switch(wEvent) {

      case VK_LBUTTON:
         writestring( "VK_LBUTTON", outfile);
         break;

      case VK_RBUTTON:
         writestring( "VK_RBUTTON", outfile);
         break;

      case VK_CANCEL:
         writestring( "VK_CANCEL", outfile);
         break;

      case VK_MBUTTON:
         writestring( "VK_MBUTTON", outfile);
         break;

      case VK_BACK:
         writestring( "VK_BACK", outfile);
         break;

      case VK_TAB:
         writestring( "VK_TAB", outfile);
         break;

      case VK_CLEAR:
         writestring( "VK_CLEAR", outfile);
         break;

      case VK_RETURN:
         writestring( "VK_RETURN", outfile);
         break;

      case VK_SHIFT:
         writestring( "VK_SHIFT", outfile);
         break;

      case VK_CONTROL:
         writestring( "VK_CONTROL", outfile);
         break;

      case VK_MENU:
         writestring( "VK_MENU", outfile);
         break;

      case VK_PAUSE:
         writestring( "VK_PAUSE", outfile);
         break;

      case VK_CAPITAL:
         writestring( "VK_CAPITAL", outfile);
         break;

      case VK_ESCAPE:
         writestring( "VK_ESCAPE", outfile);
         break;

      case VK_SPACE:
         writestring( "VK_SPACE", outfile);
         break;

      case VK_PRIOR:
         writestring( "VK_PRIOR", outfile);
         break;

      case VK_NEXT:
         writestring( "VK_NEXT", outfile);
         break;

      case VK_END:
         writestring( "VK_END", outfile);
         break;

      case VK_HOME:
         writestring( "VK_HOME", outfile);
         break;

      case VK_LEFT:
         writestring( "VK_LEFT", outfile);
         break;

      case VK_UP:
         writestring( "VK_UP", outfile);
         break;

      case VK_RIGHT:
         writestring( "VK_RIGHT", outfile);
         break;

      case VK_DOWN:
         writestring( "VK_DOWN", outfile);
         break;

      case VK_SELECT:
         writestring( "VK_SELECT", outfile);
         break;

      case VK_PRINT:
         writestring( "VK_PRINT", outfile);
         break;

      case VK_EXECUTE:
         writestring( "VK_EXECUTE", outfile);
         break;

      case VK_SNAPSHOT:
         writestring( "VK_SNAPSHOT", outfile);
         break;

      case VK_INSERT:
         writestring( "VK_INSERT", outfile);
         break;

      case VK_DELETE:
         writestring( "VK_DELETE", outfile);
         break;

      case VK_HELP:
         writestring( "VK_HELP", outfile);
         break;

      case VK_NUMPAD0:
         writestring( "VK_NUMPAD0", outfile);
         break;

      case VK_NUMPAD1:
         writestring( "VK_NUMPAD1", outfile);
         break;

      case VK_NUMPAD2:
         writestring( "VK_NUMPAD2", outfile);
         break;

      case VK_NUMPAD3:
         writestring( "VK_NUMPAD3", outfile);
         break;

      case VK_NUMPAD4:
         writestring( "VK_NUMPAD4", outfile);
         break;

      case VK_NUMPAD5:
         writestring( "VK_NUMPAD5", outfile);
         break;

      case VK_NUMPAD6:
         writestring( "VK_NUMPAD6", outfile);
         break;

      case VK_NUMPAD7:
         writestring( "VK_NUMPAD7", outfile);
         break;

      case VK_NUMPAD8:
         writestring( "VK_NUMPAD8", outfile);
         break;

      case VK_NUMPAD9:
         writestring( "VK_NUMPAD9", outfile);
         break;

      case VK_MULTIPLY:
         writestring( "VK_MULTIPLY", outfile);
         break;

      case VK_ADD:
         writestring( "VK_ADD", outfile);
         break;

      case VK_SEPARATOR:
         writestring( "VK_SEPARATOR", outfile);
         break;

      case VK_SUBTRACT:
         writestring( "VK_SUBTRACT", outfile);
         break;

      case VK_DECIMAL:
         writestring( "VK_DECIMAL", outfile);
         break;

      case VK_DIVIDE:
         writestring( "VK_DIVIDE", outfile);
         break;

      case VK_F1:
         writestring( "VK_F1", outfile);
         break;

      case VK_F2:
         writestring( "VK_F2", outfile);
         break;

      case VK_F3:
         writestring( "VK_F3", outfile);
         break;

      case VK_F4:
         writestring( "VK_F4", outfile);
         break;

      case VK_F5:
         writestring( "VK_F5", outfile);
         break;

      case VK_F6:
         writestring( "VK_F6", outfile);
         break;

      case VK_F7:
         writestring( "VK_F7", outfile);
         break;

      case VK_F8:
         writestring( "VK_F8", outfile);
         break;

      case VK_F9:
         writestring( "VK_F9", outfile);
         break;

      case VK_F10:
         writestring( "VK_F10", outfile);
         break;

      case VK_F11:
         writestring( "VK_F11", outfile);
         break;

      case VK_F12:
         writestring( "VK_F12", outfile);
         break;

      case VK_F13:
         writestring( "VK_F13", outfile);
         break;

      case VK_F14:
         writestring( "VK_F14", outfile);
         break;

      case VK_F15:
         writestring( "VK_F15", outfile);
         break;

      case VK_F16:
         writestring( "VK_F16", outfile);
         break;

      case VK_F17:
         writestring( "VK_F17", outfile);
         break;

      case VK_F18:
         writestring( "VK_F18", outfile);
         break;

      case VK_F19:
         writestring( "VK_F19", outfile);
         break;

      case VK_F20:
         writestring( "VK_F20", outfile);
         break;

      case VK_F21:
         writestring( "VK_F21", outfile);
         break;

      case VK_F22:
         writestring( "VK_F22", outfile);
         break;

      case VK_F23:
         writestring( "VK_F23", outfile);
         break;

      case VK_F24:
         writestring( "VK_F24", outfile);
         break;

      case VK_NUMLOCK:
         writestring( "VK_NUMLOCK", outfile);
         break;

      case VK_SCROLL:
         writestring( "VK_SCROLL", outfile);
         break;

      default:
         write_word( wEvent, outfile);
         break;

    } /* switch(wEvent) - end */

} /* write_virtual_accel_event - end */


/***************************************************
  Write the character for the current accelerator
****************************************************/
int write_accel_event( WORD wEvent, FILE *outfile) {

   if(((wEvent >= 65) && (wEvent <= 90)) ||
      ((wEvent >= 97) && (wEvent <= 122))) {
      write_char('"', outfile);
      write_char((BYTE) wEvent, outfile);
      write_char('"', outfile);
      return(0);
   }

   if((wEvent >= 1) && (wEvent <= 26)) {
      write_char('"', outfile);
      write_char('^', outfile);
      write_char((BYTE) (wEvent + 64), outfile);
      write_char('"', outfile);
      return(0);
   }

   write_word( wEvent, outfile);
   return(1);

} /* write_accel_event - end */


/***************************************************
  Write the flags for the current accelerator
****************************************************/
void write_accel_flags( BYTE flags, FILE *outfile, int ascii_value_used) {

   if(flags & 0x02)
      writestring(", NOINVERT", outfile);
   if(flags & 0x04)
      writestring(", SHIFT", outfile);
   if(flags & 0x08)
      writestring(", CONTROL", outfile);
   if(flags & 0x10)
      writestring(", ALT", outfile);
   if(flags & 0x01)
      writestring(", VIRTKEY", outfile);
   else if (ascii_value_used)
      writestring(", ASCII", outfile);
   writeline("", outfile);

} /* write_accel_flags - end */


/***************************************************
  Process the accelerator resource
****************************************************/
void process_accelerator(int restype, FILE *infile, FILE *outfile) {

   DWORD  reslen;
   struct AccelTableEntry accelhdr;
   int    ascii_value_used;

   write_resource_name(infile, outfile);
   write_char(' ', outfile);
   writestring(res_array[restype], outfile);
   get_mem_flags(infile);
   writeline(" {", outfile);

   increase_indent();

   reslen = get_resource_length(infile);

   while(reslen) {

      write_indent(outfile);

      fread(&accelhdr, sizeof(struct AccelTableEntry), 1, infile);
      reslen -= sizeof(struct AccelTableEntry);

      if(accelhdr.fFlags & 0x01)
         write_virtual_accel_event( accelhdr.wEvent, outfile);
      else
         ascii_value_used = write_accel_event( accelhdr.wEvent, outfile);

      writestring(", ", outfile);
      write_word(accelhdr.wId, outfile);
      write_accel_flags( accelhdr.fFlags, outfile, ascii_value_used);

   }

   decrease_indent();

   writeline("}", outfile);

} /* process_accelerator - end */


/***************************************************
  Process the rcdata resource
****************************************************/
void process_rcdata(int restype, FILE *infile, FILE *outfile) {

   DWORD reslen;
   int ch;
   unsigned short ch_count;

   write_resource_name(infile, outfile);
   write_char(' ', outfile);
   writestring(res_array[restype], outfile);
   write_mem_flags(infile, outfile);
   writeline(" {", outfile);

   reslen = get_resource_length(infile);
   increase_indent();

   /* copy the data out to the .rc file */
   ch_count = 0;
   while (reslen--) {
      if (ch_count == 0) {
         write_indent(outfile);
         write_char('"', outfile);
      }
      ch = fgetc( infile );
      if ((ch >= 32) && (ch <= 126)) {
         fputc( ch, outfile);
         ++ch_count;
      }
      else {
         write_char('\\', outfile);
         fprintf( outfile, "%o", ch);
         ch_count += 4;
      }
      if (ch_count >= 60) {
         writeline("\"", outfile);
         ch_count = 0;
      }
   }

   /* if last string wasn't terminated with end quotes, do so now */
   if (ch_count > 0)
      writeline("\"", outfile);

   decrease_indent();

   writeline("}", outfile);

} /* process_rcdata - end */


/***************************************************
  Search infile for image #image_num of type image_type
****************************************************/
int search_for_image( WORD image_num, FILE *infile, int image_type ) {

   BYTE ch;
   long reslen;
   int  restype;
   WORD dest_image_num;

   fseek(infile, 0, 0);

   ch = get_byte(infile);
   while(!feof(infile)) {

      /* get the resource type */
      if(ch == 0xFF) {

         fread(&restype, sizeof(int), 1, infile);

         /* If it's not the right image type, skip it */
         if (restype != image_type) {
            
            get_resource_name(infile);
            get_mem_flags(infile);
            reslen = get_resource_length(infile);

            fseek(infile, reslen, 1);
         }
         else {
            dest_image_num = get_resource_number(infile);
            get_mem_flags(infile);
            reslen = get_resource_length(infile);

            if (dest_image_num == image_num)
               return(1);
            else
               fseek(infile, reslen, 1);
         }

      }
      else {

         /* read the name of the resource */
         while((ch = get_byte(infile)) != 0x00) {}

         get_resource_name(infile);
         get_mem_flags(infile);
         reslen = get_resource_length(infile);

         fseek(infile, reslen, 1);
      }

      ch = get_byte(infile);

   } /* while(not eof(infile)) - end */

   return(0);

} /* search_for_image - end */


/***************************************************
  Write the Cursor Directory entry to the cursor file
****************************************************/
void write_cursor_direntry( CURSORDIRENTRY cursorentry, FILE *datafile, 
                            FILE *infile, DWORD cursor_size) {

   CURSORRESENTRY cursorres;
   long filepos;
   WORD hotspot;

   cursorres.bWidth      = (BYTE) cursorentry.wWidth;
   cursorres.bHeight     = (BYTE) (cursorentry.wHeight - cursorentry.wWidth);
   cursorres.bColorCount = 0;
   cursorres.bReserved   = 0;

   /* Initialize to 0's */
   cursorres.wXHotSpot = 0;
   cursorres.wYHotSpot = 0;

   /* Save the current position of the input file */
   filepos = ftell(infile);

   /* Search for cursor resource number #wImageOffset to get the hotspots */
   if (search_for_image( cursorentry.wImageOffset, infile, CURSOR_TYPE)) {
      hotspot = get_word( infile );
      cursorres.wXHotSpot = hotspot;

      hotspot = get_word( infile );
      cursorres.wYHotSpot = hotspot;
   }

   /* return to the prior position in the input file */
   fseek(infile, filepos, 0);

   /* subtract size of 2 WORD values - X & Y Hot Spot - they occur */
   /* at the beginning of the cursor resource data, but really     */
   /* belong in the header, so they get subtracted from the length */
   cursorres.dwBytesInRes  = cursorentry.dwBytesInRes - (2 * sizeof(WORD));

   cursorres.dwImageOffset = cursor_size;

   fwrite( &cursorres, sizeof(CURSORRESENTRY), 1, datafile);

} /* write_cursor_direntry - end */


/***************************************************
  Process the group cursor resource
****************************************************/
void process_group_cursor(FILE *infile, FILE *outfile) {

   DWORD reslen;
   char  datafilename[13];
   FILE  *datafile;
   long  CurrPos;
   DWORD cursor_size;

   CURSORHEADER   cursorinfo;
   CURSORDIRENTRY cursorentry;
   WORD count;

   write_resource_name(infile, outfile);
   writestring(" CURSOR", outfile);

   /* Now write out the memory flags for the previous resource */
   /* (cursor), since that has the correct value; this must be */
   /* done before the succeeding call to get_mem_flags, since  */
   /* that will change the value of prev_mem_flag.             */
   write_special_mem_flag_values( prev_mem_flag, outfile);

   /* Now skip over the memory flag for this resource */
   get_mem_flags(infile);

   write_char(' ', outfile);

   reslen = get_resource_length(infile);

   /* Determine a unique .cur filename in the current directory */
   strcpy(datafilename, get_data_filename("CU", "CUR"));
   if(strlen(datafilename) == 0) {
      writeline("<Unable to open output file>", outfile);
      fseek( infile, reslen, 1);
      return;
   }

   /* Write the name of the new cursor file to the .rc file */
   writeline( datafilename, outfile );

   /* open .cur output file */
   if((datafile = fopen(datafilename, "wb")) == NULL) {
      printf("Unable to open file %s\n", datafilename);
      fseek( infile, reslen, 1);
      return;
   }

   fread( &cursorinfo, sizeof(CURSORHEADER), 1, infile);
   fwrite( &cursorinfo, sizeof(CURSORHEADER), 1, datafile);

   cursor_size = sizeof(CURSORHEADER) + (sizeof(CURSORRESENTRY) * cursorinfo.cdCount);
   CurrPos = ftell(infile);

   /* Loop through each Cursor entry in the Group Cursor resource */
   count = cursorinfo.cdCount;
   while (count--) {

      /* Read the header for this cursor, and save to the output file */
      fread( &cursorentry, sizeof(CURSORDIRENTRY), 1, infile);

      write_cursor_direntry( cursorentry, datafile, infile, cursor_size);
      cursor_size += cursorentry.dwBytesInRes;

   }

   fseek(infile, CurrPos, 0);

   /* Loop through each Cursor entry in the Group Cursor resource */
   count = cursorinfo.cdCount;
   while (count--) {

      fread( &cursorentry, sizeof(CURSORDIRENTRY), 1, infile);

      CurrPos += sizeof(CURSORDIRENTRY);

      /* Search for cursor resource number #wImageOffset */
      if (search_for_image( cursorentry.wImageOffset, infile, CURSOR_TYPE)) {

         /* skip the 2 WORDS for XHotSpot and YHotSpot */
         fseek(infile, 4, 1);

         /* subtract the size of the 2 WORDs at the start: hotspot data */
         write_data( infile, datafile, 
                     (cursorentry.dwBytesInRes - (2 * sizeof(WORD))));
      }

      fseek(infile, CurrPos, 0);

   }

   fclose( datafile );

} /* process_group_cursor - end */


/***************************************************
  Write the Icon Directory entry to the icon file
****************************************************/
void write_icon_direntry( ICONDIRENTRY iconentry, FILE *datafile, 
                          DWORD icon_size) {

   ICONRESENTRY iconres;

   iconres.bWidth        = iconentry.bWidth;
   iconres.bHeight       = iconentry.bHeight;
   iconres.bColorCount   = iconentry.bColorCount;
   iconres.bReserved     = iconentry.bReserved;

/* The Planes and BitCount values stored in the .ico file 
   seem to be ignored.

   iconres.wPlanes       = iconentry.wPlanes;
   iconres.wBitCount     = iconentry.wBitCount;

*/

   iconres.wPlanes       = 0;
   iconres.wBitCount     = 0;

   iconres.dwBytesInRes  = iconentry.dwBytesInRes;
   iconres.dwImageOffset = icon_size;

   fwrite( &iconres, sizeof(ICONRESENTRY), 1, datafile);

} /* write_icon_direntry - end */


/***************************************************
  Process the group icon resource
****************************************************/
void process_group_icon(FILE *infile, FILE *outfile) {

   DWORD reslen;
   char  datafilename[13];
   FILE  *datafile;
   long  CurrPos;
   DWORD icon_size;

   ICONHEADER   iconinfo;
   ICONDIRENTRY iconentry;
   WORD count;

   write_resource_name(infile, outfile);
   writestring(" ICON", outfile);

   /* Now write out the memory flags for the previous resource */
   /* (icon), since that has the correct value; this must be   */
   /* done before the succeeding call to get_mem_flags, since  */
   /* that will change the value of prev_mem_flag.             */
   write_special_mem_flag_values( prev_mem_flag, outfile);

   /* Now skip over the memory flag for this resource */
   get_mem_flags(infile);

   write_char(' ', outfile);

   reslen = get_resource_length(infile);

   /* Determine a unique .ico filename in the current directory */
   strcpy(datafilename, get_data_filename("IC", "ICO"));
   if(strlen(datafilename) == 0) {
      writeline("<Unable to open output file>", outfile);
      fseek( infile, reslen, 1);
      return;
   }

   /* Write the name of the new icon file to the .rc file */
   writeline( datafilename, outfile );

   /* Open the output file containing the icon resource data */
   if((datafile = fopen(datafilename, "wb")) == NULL) {
      printf("Unable to open file %s\n", datafilename);
      fseek( infile, reslen, 1);
      return;
   }

   fread( &iconinfo, sizeof(ICONHEADER), 1, infile);
   fwrite( &iconinfo, sizeof(ICONHEADER), 1, datafile);

   icon_size = sizeof(ICONHEADER) + (sizeof(ICONRESENTRY) * iconinfo.idCount);
   CurrPos = ftell(infile);

   /* Loop through each Icon entry in the Group Icon resource */
   count = iconinfo.idCount;
   while (count--) {

      fread( &iconentry, sizeof(ICONDIRENTRY), 1, infile);

      write_icon_direntry( iconentry, datafile, icon_size);
      icon_size += iconentry.dwBytesInRes;

   }

   fseek(infile, CurrPos, 0);

   /* Loop through each Icon entry in the Group Icon resource */
   count = iconinfo.idCount;
   while (count--) {

      fread( &iconentry, sizeof(ICONDIRENTRY), 1, infile);

      CurrPos += sizeof(ICONDIRENTRY);

      /* Search for icon resource number #wImageOffset */
      if (search_for_image( iconentry.wImageOffset, infile, ICON_TYPE)) {
         write_data( infile, datafile, iconentry.dwBytesInRes );
      }

      fseek(infile, CurrPos, 0);

   }

   fclose( datafile );

} /* process_group_icon - end */


/***************************************************
  Save a user-defined resource data to a file.
****************************************************/
void save_user_resource( FILE *infile, FILE *outfile) {

   DWORD reslen;
   char  datafilename[13];
   FILE  *datafile;

   reslen = get_resource_length(infile);

   strcpy(datafilename, get_data_filename("UR", "USR"));
   if(strlen(datafilename) == 0) {
      writeline("<Unable to open output file>", outfile);
      fseek( infile, reslen, 1);
      return;
   }

   writeline( datafilename, outfile );

   if((datafile = fopen(datafilename, "wb")) == NULL) {
      printf("Unable to open file %s\n", datafilename);
      fseek( infile, reslen, 1);
      return;
   }

   write_data(infile, datafile, reslen);
   fclose( datafile );

} /* save_user_resource - end */


/***************************************************
  Process a user-defined resource (by number).
****************************************************/
void process_user_resource_num(int restype, FILE *infile, FILE *outfile) {

   write_resource_name(infile, outfile);
   write_char(' ', outfile);
   write_number( restype, outfile);
   write_mem_flags(infile, outfile);
   write_char(' ', outfile);

   save_user_resource( infile, outfile );

} /* process_user_resource_num - end */


/***************************************************
  Read the name table, but ignore it.
****************************************************/
void process_name_table( FILE *infile, FILE *outfile) {

   DWORD reslen;

   writeline( "", outfile);
   writeline( "//", outfile);
   writeline( "// Name table found, but ignored.", outfile );
   writeline( "//", outfile);

   get_resource_name(infile);
   get_mem_flags(infile);

   reslen = get_resource_length(infile);
   fseek(infile, reslen, 1);

} /* process_name_table - end */


/***************************************************
  Checks if ver.h has been included; if not, does so
****************************************************/
void check_if_ver_header_included( FILE *outfile ) {

   if (VersionUsed == 0) {

      /* update VersionUsed so <ver.h> can't be included twice */
      VersionUsed = 1;

      writeline( "", outfile);
      writeline( "#include <ver.h>", outfile);

   }

} /* check_if_ver_header_included - end */


/***************************************************
  Process version fileflags
****************************************************/
void write_version_fileflags( DWORD style, FILE *outfile ) {

   WORD first_style = 1;
   WORD line_len = 0;

   writestring( " FILEFLAGS ", outfile);

   if (style == 0L)
      writeline( "0x0L", outfile);
   else {
      check_for_dlg_style( VS_FF_DEBUG & style, &first_style,
                           &line_len, "VS_FF_DEBUG", outfile);
      check_for_dlg_style( VS_FF_INFOINFERRED & style, &first_style, 
                           &line_len, "VS_FF_INFOINFERRED", outfile);
      check_for_dlg_style( VS_FF_PATCHED & style, &first_style, 
                           &line_len, "VS_FF_PATCHED", outfile);
      check_for_dlg_style( VS_FF_PRERELEASE & style, &first_style, 
                           &line_len, "VS_FF_PRERELEASE", outfile);
      check_for_dlg_style( VS_FF_PRIVATEBUILD & style, &first_style, 
                           &line_len, "VS_FF_PRIVATEBUILD", outfile);
      check_for_dlg_style( VS_FF_SPECIALBUILD & style, &first_style, 
                           &line_len, "VS_FF_SPECIALBUILD", outfile);

      writeline( "", outfile);
   }

} /* write_version_fileflags - end */


/***************************************************
  Process the version driver subtype
****************************************************/
void process_version_driver_subtype( DWORD driver_subtype, FILE *outfile ) {

   switch (driver_subtype) {

      case VFT2_UNKNOWN:
         writeline("VFT2_UNKNOWN", outfile);
         break;

      case VFT2_DRV_COMM:
         writeline("VFT2_DRV_COMM", outfile);
         break;

      case VFT2_DRV_PRINTER:
         writeline("VFT2_DRV_PRINTER", outfile);
         break;

      case VFT2_DRV_KEYBOARD:
         writeline("VFT2_DRV_KEYBOARD", outfile);
         break;

      case VFT2_DRV_LANGUAGE:
         writeline("VFT2_DRV_LANGUAGE", outfile);
         break;

      case VFT2_DRV_DISPLAY:
         writeline("VFT2_DRV_DISPLAY", outfile);
         break;

      case VFT2_DRV_MOUSE:
         writeline("VFT2_DRV_MOUSE", outfile);
         break;

      case VFT2_DRV_NETWORK:
         writeline("VFT2_DRV_NETWORK", outfile);
         break;

      case VFT2_DRV_SYSTEM:
         writeline("VFT2_DRV_SYSTEM", outfile);
         break;

      case VFT2_DRV_INSTALLABLE:
         writeline("VFT2_DRV_INSTALLABLE", outfile);
         break;

      case VFT2_DRV_SOUND:
         writeline("VFT2_DRV_SOUND", outfile);
         break;

      default:
         fprintf( outfile, "%lu", driver_subtype );
         writeline("", outfile);
         break;

   } /* switch (driver_subtype) - end */

} /* process_version_driver_subtype - end */


/***************************************************
  Process the version font subtype
****************************************************/
void process_version_font_subtype( DWORD font_subtype, FILE *outfile ) {

   switch (font_subtype) {

      case VFT2_UNKNOWN:
         writeline("VFT2_UNKNOWN", outfile);
         break;

      case VFT2_FONT_RASTER:
         writeline("VFT2_FONT_RASTER", outfile);
         break;

      case VFT2_FONT_VECTOR:
         writeline("VFT2_FONT_VECTOR", outfile);
         break;

      case VFT2_FONT_TRUETYPE:
         writeline("VFT2_FONT_TRUETYPE", outfile);
         break;

      default:
         fprintf( outfile, "%lu", font_subtype );
         writeline("", outfile);
         break;

   } /* switch (font_subtype) - end */

} /* process_version_font_subtype - end */


/***************************************************
  Process version information root block
****************************************************/
void process_version_root_block( FILE *infile, FILE *outfile) {

   VS_FIXEDFILEINFO infostruct;

   /* Process each field of the root block */
   fread( &infostruct, sizeof(VS_FIXEDFILEINFO), 1, infile);

   fprintf( outfile, " FILEVERSION %u,%u,%u,%u",
               HIWORD(infostruct.dwFileVersionMS),
               LOWORD(infostruct.dwFileVersionMS),
               HIWORD(infostruct.dwFileVersionLS),
               LOWORD(infostruct.dwFileVersionLS));
   writeline( "", outfile);

   fprintf( outfile, " PRODUCTVERSION %u,%u,%u,%u",
               HIWORD(infostruct.dwProductVersionMS),
               LOWORD(infostruct.dwProductVersionMS),
               HIWORD(infostruct.dwProductVersionLS),
               LOWORD(infostruct.dwProductVersionLS));
   writeline( "", outfile);

   write_version_fileflags( infostruct.dwFileFlags, outfile );

   if(infostruct.dwFileFlagsMask == VS_FFI_FILEFLAGSMASK)
      fprintf( outfile, " FILEFLAGSMASK VS_FFI_FILEFLAGSMASK");
   else
      fprintf( outfile, " FILEFLAGSMASK %lu", infostruct.dwFileFlagsMask);
   writeline( "", outfile);

   writestring( " FILEOS ", outfile);
   switch (infostruct.dwFileOS) {

      case VOS_UNKNOWN:
         writeline("VOS_UNKNOWN", outfile);
         break;

      case VOS_DOS:
         writeline("VOS_DOS", outfile);
         break;

      case VOS_OS216:
         writeline("VOS_OS216", outfile);
         break;

      case VOS_OS232:
         writeline("VOS_OS232", outfile);
         break;

      case VOS_NT:
         writeline("VOS_NT", outfile);
         break;

      case VOS_DOS_WINDOWS16:
         writeline("VOS_DOS_WINDOWS16", outfile);
         break;

      case VOS_DOS_WINDOWS32:
         writeline("VOS_DOS_WINDOWS32", outfile);
         break;

      case VOS_OS216_PM16:
         writeline("VOS_OS216_PM16", outfile);
         break;

      case VOS_OS232_PM32:
         writeline("VOS_OS232_PM32", outfile);
         break;

      case VOS_NT_WINDOWS32:
         writeline("VOS_NT_WINDOWS32", outfile);
         break;

      default:
         fprintf( outfile, "%lu", infostruct.dwFileOS );
         writeline("", outfile);
         break;

   } /* switch (FileOS) - end */

   writestring( " FILETYPE ", outfile);
   switch (infostruct.dwFileType) {

      case VFT_UNKNOWN:
         writeline("VFT_UNKNOWN", outfile);
         break;

      case VFT_APP:
         writeline("VFT_APP", outfile);
         break;

      case VFT_DLL:
         writeline("VFT_DLL", outfile);
         break;

      case VFT_DRV:
         writeline("VFT_DRV", outfile);
         break;

      case VFT_FONT:
         writeline("VFT_FONT", outfile);
         break;

      case VFT_VXD:
         writeline("VFT_VXD", outfile);
         break;

      case VFT_STATIC_LIB:
         writeline("VFT_STATIC_LIB", outfile);
         break;

      default:
         fprintf( outfile, "%lu", infostruct.dwFileType );
         writeline("", outfile);
         break;

   } /* switch (FileType) - end */

   writestring( " FILESUBTYPE ", outfile);
   switch (infostruct.dwFileType) {

      case VFT_DRV:
         process_version_driver_subtype( infostruct.dwFileSubtype, outfile);
         break;

      case VFT_FONT:
         process_version_font_subtype( infostruct.dwFileSubtype, outfile);
         break;

      default:
         fprintf( outfile, "%lu", infostruct.dwFileSubtype );
         writeline("", outfile);
         break;

   } /* switch (FileSubtype) - end */


} /* process_version_root_block - end */


/***************************************************
  Write out the name of the current block, and return
   whether it was string, variable or other type of
   block.
****************************************************/
int write_block_name( FILE *infile, FILE *outfile ) {

   int  count=1;      /* record number of chars read */
   int  blocktype=0;  /* record which type of block it is */
   BYTE ch;

   ch = get_byte(infile);
   if (ch == 'S')
      blocktype = STRINGBLOCK;
   else if (ch == 'V')
      blocktype = VARBLOCK;
   else
      blocktype = OTHERBLOCK;

   while((!feof(infile)) && (ch != 0x00)) {
      ++count;
      write_char( ch, outfile);
      ch = get_byte(infile);
   }

   /* Ensure number of characters read is a multiple of 4.    */
   /* According to the MS documentation, this is the format.  */
   /* See "MS Windows 3.1 Programmer's Reference" Vol.4, p.99 */
   count = count % 4;
   count = (count > 0) ? (4 - count) : 0;
   while ((!feof(infile)) && (count > 0)) {
      ch = get_byte(infile);
      --count;
   }

   return(blocktype);

} /* write_block_name - end */


/***************************************************
  Write out the name of the current block, and return
   the total number of characters in the field.
****************************************************/
WORD write_ver_field_name( FILE *infile, FILE *outfile ) {

   WORD count=1;      /* record number of chars read */
   WORD fieldsize=1;  /* record number of chars read */
   BYTE ch;

   ch = get_byte(infile);

   while((!feof(infile)) && (ch != 0x00)) {
      ++count;
      write_char( ch, outfile);
      ch = get_byte(infile);
   }
   fieldsize = count;

   /* Ensure number of characters read is a multiple of 4.    */
   /* According to the MS documentation, this is the format.  */
   /* See "MS Windows 3.1 Programmer's Reference" Vol.4, p.99 */
   count = count % 4;
   count = (count > 0) ? (4 - count) : 0;
   while ((!feof(infile)) && (count > 0)) {
      ++fieldsize;
      ch = get_byte(infile);
      --count;
   }

   return(fieldsize);

} /* write_ver_field_name - end */


/***************************************************
  Read "wordsize" # of bytes and write them to the   
   output file; these bytes are the second part of 
   the  'VALUE "---", "---"' line in a version info
   string block.  Return the number of bytes needed
   to align the string on a 32-bit boundary.
****************************************************/
WORD write_ver_field_name_size_n( FILE *infile, FILE *outfile,
                                  WORD wordsize ) {

   WORD count=0;      /* record number of chars read */
   WORD diff=0;
   BYTE ch;

   while((!feof(infile)) && (count < wordsize)) {
      ++count;
      ch = get_byte(infile);
      write_char( ch, outfile);
   }

   /* Ensure number of characters read is a multiple of 4.    */
   /* According to the MS documentation, this is the format.  */
   /* See "MS Windows 3.1 Programmer's Reference" Vol.4, p.99 */
   count = count % 4;
   count = (count > 0) ? (4 - count) : 0;
   diff = count;
   while ((!feof(infile)) && (count > 0)) {
      ch = get_byte(infile);
      --count;
   }

   return(diff);

} /* write_ver_field_name_size_n - end */


/***************************************************
  Process a version string block
****************************************************/
void process_version_string_block( WORD blocksize, FILE *infile, 
                                   FILE *outfile ) {

   WORD stringblocksize = 0;
   WORD subblocksize = 0;
   WORD subdatasize  = 0;
   WORD fieldsize = 0;
   WORD diff = 0;

   WORD totalblocksize = 0;  /* size so far of the entire block */
   WORD totalsubsize   = 0;  /* size so far of a subblock */

   /* add up: len(StringFileInfo\0) + sizeof(blocksize) +  */
   /* sizeof(datasize) (values already read in)         */
   totalblocksize = 16 + (2 * sizeof(WORD));

   while (totalblocksize < blocksize) {

      /* get the name of this entire string block */
      stringblocksize = get_word( infile );
      (void)get_word( infile );
      totalblocksize += stringblocksize;

      write_indent( outfile );
      writestring( "BLOCK \"", outfile);
      fieldsize = write_ver_field_name( infile, outfile );
      writeline( "\"", outfile);

      write_indent( outfile );
      writeline( "BEGIN", outfile);

      increase_indent();

      totalsubsize = fieldsize + (2 * sizeof(WORD)); 
      while (totalsubsize < stringblocksize) {

         subblocksize = get_word(infile);
         subdatasize  = get_word(infile);

         /* increment the byte counter by the size of the current block */
         totalsubsize += subblocksize;

         write_indent( outfile );
         writestring( "VALUE \"", outfile);
         (void)write_ver_field_name( infile, outfile );
         writestring( "\", \"", outfile);

         diff = write_ver_field_name_size_n( infile, outfile, subdatasize );

         totalsubsize += diff;
         totalblocksize += diff;

         writeline( "\"", outfile);

      } /* while (totalsubsize < stringblocksize) - end */

      decrease_indent();
      write_indent( outfile );
      writeline( "END", outfile);

   } /* while (totalblocksize < blocksize) - end */

} /* process_version_string_block - end */


/***************************************************
  Process a version variable block
****************************************************/
void process_version_var_block( WORD blocksize, FILE *infile, 
                                FILE *outfile ) {

   WORD subblocksize = 0;
   WORD subdatasize  = 0;
   WORD totalsize    = 0;

   WORD langid    = 0;
   WORD charsetid = 0;

   /* add up: len(VarFileInfo\0) + sizeof(blocksize) +  */
   /* sizeof(datasize) (values already read in)         */
   totalsize = 12 + (2 * sizeof(WORD));

   while (totalsize < blocksize) {

      subblocksize = get_word( infile );
      subdatasize  = get_word( infile );

      write_indent( outfile );
      writestring( "VALUE \"", outfile);
      (void)write_block_name( infile, outfile );
      writestring( "\", ", outfile);

      /* increment the byte counter by the size of the current block */
      totalsize += subblocksize;

      while (subdatasize) {

         langid = get_word( infile );
         charsetid = get_word( infile );

         fprintf( outfile, "0x%X, %d", langid, charsetid);

         /* take off the size of the 2 variables read above */
         subdatasize -= (2 * sizeof(WORD)); 

         /* if another entry after this one, write out a comma */
         if (subdatasize) {
            writeline( ",", outfile);
            write_indent( outfile );
         }
         else
            writeline( "", outfile);

      } /* while (subdatasize) - end */

   } /* while (totalsize < blocksize) - end */

} /* process_version_var_block - end */


/***************************************************
  Process an unknown type of version block
****************************************************/
void process_version_other_block( WORD blocksize, long currpos,
                                  FILE *infile, FILE *outfile ) {

   long newPos = 0L;

   write_indent( outfile );
   writeline("// Unknown block type - skipping", outfile);

   newPos = currpos + ((long) blocksize);
   fseek( infile, newPos, 0);

} /* process_version_other_block - end */


/***************************************************
  Process a version block, which can be either a
   StringFileInfo or VarFileInfo block.
****************************************************/
void process_version_block( FILE *infile, FILE *outfile ) {

   WORD blocksize = 0;   /* size of the next (complete) block */

   long currpos = 0L;    /* use in case the block type is unknown */

   int  block_type = 0;  /* mark block as StringFileInfo or VarStringInfo */

   INDENT = 0;           /* reset the amount of indentation */

   currpos = ftell(infile);

   blocksize = get_word(infile);
   (void) get_word(infile);     /* skip over the size of current dataset */

   increase_indent();
   write_indent( outfile );

   writestring( "BLOCK \"", outfile);
   block_type = write_block_name( infile, outfile );
   writeline( "\"", outfile);

   write_indent( outfile );
   writeline( "BEGIN", outfile);

   increase_indent();

   /* call the appropriate procedure for block_type */
   switch (block_type) {

      case STRINGBLOCK:
         process_version_string_block( blocksize, infile, outfile );
         break;

      case VARBLOCK:
         process_version_var_block( blocksize, infile, outfile );
         break;

      case OTHERBLOCK:
      default:
         process_version_other_block( blocksize, currpos, infile, outfile );
         break;

   } /* switch (block_type) - end */

   decrease_indent();
   write_indent( outfile );

   writeline( "END", outfile);

   decrease_indent();

} /* process_version_block - end */


/***************************************************
  Process version information
****************************************************/
void process_version_info( FILE *infile, FILE *outfile) {

   DWORD reslen = 0L;     /* total size of version info     */
   DWORD endpos = 0L;     /* ftell() of end of version info */
   DWORD currpos = 0L;    /* current position in input file */

   /* see if "#include <ver.h>" has already been written */
   check_if_ver_header_included( outfile );

   /* start writing out the version information */
   write_version_number(infile, outfile);
   writeline( "VERSIONINFO", outfile);

   /* version info doesn't seem to use memory flags, so skip them */
   get_mem_flags(infile);

   reslen = get_resource_length(infile);
   currpos = ftell(infile);
   endpos = currpos + reslen;

   /* get the name and size of the root block - we can ignore */
   /* this because we know what it's going to be.             */
   (void)get_word(infile); /* cbBlock */
   (void)get_word(infile); /* cbValue */

   get_version_name(infile); /* szKey[] */

   process_version_root_block(infile, outfile);

   writeline( "BEGIN", outfile);

   /* now go through all of the remaining blocks */
   currpos = ftell(infile);
   while (currpos < endpos) {

      process_version_block( infile, outfile );
      currpos = ftell(infile);

   }

   writeline( "END", outfile);

} /* process_version_info - end */


/***************************************************
  Call the appropriate function for each resource type
****************************************************/
void process_resource_by_number(FILE *infile, FILE *outfile) {

   int restype = 0;

   INDENT = 0;
   fread(&restype, sizeof(int), 1, infile);

   /* If we're in the middle of a string table, and the new resource */
   /* isn't a string table, finish it off */
   if ((restype != STRING_TYPE) && (StringCount)) {
      writeline("}", outfile);
      StringCount = 0;
   }

   switch(restype) {
      case RT_CURSOR:
         process_cursor( infile );
         break;

      case RT_BITMAP:
         process_bitmap(restype, infile, outfile);
         break;

      case RT_ICON:
         process_icon( infile );
         break;

      case RT_MENU:
         process_menu(restype, infile, outfile);
         break;

      case RT_DIALOG:
         process_dialog(restype, infile, outfile);
         break;

      case RT_STRING:
         process_string(restype, infile, outfile);
         break;

      case RT_FONTDIR:
         process_fontdir( infile );
         break;

      case RT_FONT:
         process_font(restype, infile, outfile);
         break;

      case RT_ACCELERATOR:
         process_accelerator(restype, infile, outfile);
         break;

      case RT_RCDATA:
         process_rcdata(restype, infile, outfile);
         break;

      case RT_GROUP_CURSOR:
         process_group_cursor(infile, outfile);
         break;

      case RT_GROUP_ICON:
         process_group_icon(infile, outfile);
         break;

      /* name tables aren't used in Win3.1, so no predefined "RT_????" */
      case 15:
         process_name_table(infile, outfile);
         break;

      /* there doesn't seem to be an RT_????? for version info */
      case 16:
         process_version_info( infile, outfile);
         break;

      default:
         process_user_resource_num(restype, infile, outfile);
         break;
   
    }

} /* process_resource_by_number - end */


/***************************************************
   Process user-defined resource (by name)
****************************************************/
void process_resource_by_name( BYTE ch, FILE *infile, FILE *outfile) {

   long  typeid_pos;  /* file position of type id */
   long  nameid_pos;  /* file position of name id */

   typeid_pos = ftell(infile);

   /* Skip the resource name */
   fseek( infile, -1, 1);
   get_resource_name(infile);

   /* Get the name of the resource itself */
   write_resource_name(infile, outfile);
   nameid_pos = ftell(infile);

   write_char(' ', outfile);

   /* now go back to the resource name and print it out */
   fseek( infile, typeid_pos, 0);
   get_custom_type( ch, infile, outfile );

   fseek( infile, nameid_pos, 0);
   write_mem_flags(infile, outfile);
   write_char(' ', outfile);

   save_user_resource( infile, outfile );

} /* process_resource_by_name - end */


/***************************************************
   Check the parameters passed on program invokation
****************************************************/
void check_usage(int argc) {

   if (argc != 3) {
      printf("Usage: res2rc <.res filename> <.rc output filename>\n");
      exit(1);
   }

} /* check_usage - end */


/***************************************************
   Write the header to the output file.
****************************************************/
void write_header( char *infname, char *outfname, FILE *outfile ) {

   writeline( "//", outfile);
   writestring( "// ", outfile);
   writestring( outfname, outfile);
   writestring( " - resource file decompiled from ", outfile);
   writeline( infname, outfile);
   writeline( "//", outfile);
   writeline( "#include <windows.h>", outfile);

} /* write_header - end */


/***************************************************
   Check if input file is a Win32 resource file
****************************************************/
void check_for_win32_res( FILE *infile ) {

   char ch;

   ch = get_byte(infile);
   if(ch == 0x00) {
      printf("Input file is a Win32 .res file.  Stopping.\n");
      exit(1);
   }
   rewind(infile);

} /* check_for_win32_res - end */


/***************************************************
   Read .res file and process each resource.
****************************************************/
int main(int argc, char *argv[]) {

   FILE *infile;
   FILE *outfile;
   BYTE ch;

   check_usage(argc);

   if((infile = fopen(argv[1], "rb")) == NULL) {
      printf("Error: Unable to open input file.\nStopping.\n");
      exit(1);
   }

   check_for_win32_res( infile );

   if((outfile = fopen(argv[2], "wb")) == NULL) {
      printf("Error: Unable to open output file.\nStopping.\n");
      exit(1);
   }

   write_header( argv[1], argv[2], outfile );

   StringCount   = 0;

   ch = get_byte(infile);

   while(!feof(infile)) {

      /* get the resource type */
      if(ch == 0xFF)
         process_resource_by_number(infile, outfile);
      else
         process_resource_by_name(ch, infile, outfile);

      ch = get_byte(infile);

   } /* while(not eof(infile)) - end */

   finish_off_stringtable(outfile);

   fclose(infile);

   return(0);

} /* main - end */

/* Res2Rc.c - end */

