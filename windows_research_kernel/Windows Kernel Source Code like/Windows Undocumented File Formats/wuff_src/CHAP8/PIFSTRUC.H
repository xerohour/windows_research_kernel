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

#define MAX_PIFFILE_SIZE        0x3FF
#define PIFEX_OFFSET            0x171

typedef struct {
    char name_string[16];
} SECTIONNAME;

typedef struct {
    WORD next_section;        /* offset of section after this      */
                              /* last section if contents = FFFF   */
    WORD current_section;     /* offset of data                    */
    WORD size_section;        /* size of data section              */
} SECTIONHDR;


/* This is the structure of the CLOSEONEXIT byte; it's not used in the   */
/* code because we want to avoid bit-type structures; there is a routine */
/* in the code to extract these values from the byte.                    */
/* These bits are specific to "Standard" mode, except Close_OnExit (both) */
/*
typedef struct {
    int Unused0 :1;           * Unused?                                 * 
    int Graph286 :1;          * True if "Graphics/Mult. Text" is set;   * 
                       * If this field is not set, Video Mode is "Text" * 
    int PreventSwitch :1;     * True if "Prevent Program Switch" is set * 
    int NoScreenExch :1;      * True if "No Screen Exchange" is not set * 
    int Close_OnExit :1;      * True if "Close Window on Exit" is set   * 
    int Unused1 :1;           * Unused?                                 * 
    int Com2 :1;              * True if "Directly Modifies Com2" is set * 
    int Com1 :1;              * True if "Directly Modifies Com1" is set * 
} CLOSEONEXIT;
*/

/* This structure is used to hold the values from */
/* the close_on_exit BYTE in the PIF structure.   */
typedef struct {
    BYTE Unused0;            /* Unused?                                 */
    BYTE Graph286;           /* True if "Graphics/Mult. Text" is set;   */
                      /* If this field is not set, Video Mode is "Text" */
    BYTE PreventSwitch;      /* True if "Prevent Program Switch" is set */
    BYTE NoScreenExch;       /* True if "No Screen Exchange" is not set */
    BYTE Close_OnExit;       /* True if "Close Window on Exit" is set   */
    BYTE Unused1;            /* Unused?                                 */
    BYTE Com2;               /* True if "Directly Modifies Com2" is set */
    BYTE Com1;               /* True if "Directly Modifies Com1" is set */
} CLOSEONEXIT;


/* Assorted Enhanced mode normal & advanced flags */
/* First, here's the structure of the 386 flags in the file */
/*
typedef struct {
    int AllowCloseAct :1;    * True if "Allow Close When Active" (A) is set* 
    int BackgroundOn :1;     * True if "Execution: Background" is set    *
    int ExclusiveOn :1;      * True if "Execution: Exclusive" is set     *
    int FullScreenYes :1;    * True if "Display Usage: Full Screen" is   *
                            * set, else "Display Usage: Windowed" is set *
    int Unused0 :1;          * Unused?                                   *
    int SK_AltTab :1;        * True if shortcut key "Alt+Tab" (Adv) is set*
    int SK_AltEsc :1;        * True if shortcut key "Alt+Esc" (Adv) is set*
    int SK_AltSpace :1;      * True if shortcut key "Alt+Space" (Adv) set *
    int SK_AltEnter :1;      * True if shortcut key "Alt+Enter" (Adv) set *
    int SK_AltPrtSc :1;      * True if shortcut key "Alt+PrtSc" (Adv) set *
    int SK_PrtSc :1;         * True if shortcut key "PrtSc" (Adv) is set *
    int SK_CtrlEsc :1;       * True if shortcut key "Ctrl+Esc" (Adv) set *
    int Detect_Idle :1;      * True if "Detect Idle Time" (Adv.) is set  *
    int UseHMA :1;           * True if "Uses High Memory" is NOT set(Adv.)*
    int Unused1 :1;          * Unused?                                   *
    int EMS_Locked :1;       * True if "EMS Memory Locked" (Adv.) is set *
} FLAGS386;
*/

/* Here's the structure used by pifdump to store the data */
typedef struct {
    BYTE AllowCloseAct;    /* True if "Allow Close When Active" (A) is set*/
    BYTE BackgroundOn;     /* True if "Execution: Background" is set      */
    BYTE ExclusiveOn;      /* True if "Execution: Exclusive" is set       */
    BYTE FullScreenYes;    /* True if "Display Usage: Full Screen" is     */
                           /* set, else "Display Usage: Windowed" is set  */
    BYTE Unused0;          /* Unused?                                     */
    BYTE SK_AltTab;        /* True if shortcut key "Alt+Tab" (Adv) is set */
    BYTE SK_AltEsc;        /* True if shortcut key "Alt+Esc" (Adv) is set */
    BYTE SK_AltSpace;      /* True if shortcut key "Alt+Space" (Adv) set  */
    BYTE SK_AltEnter;      /* True if shortcut key "Alt+Enter" (Adv) set  */
    BYTE SK_AltPrtSc;      /* True if shortcut key "Alt+PrtSc" (Adv) set  */
    BYTE SK_PrtSc;         /* True if shortcut key "PrtSc" (Adv) is set   */
    BYTE SK_CtrlEsc;       /* True if shortcut key "Ctrl+Esc" (Adv) set   */
    BYTE Detect_Idle;      /* True if "Detect Idle Time" (Adv.) is set    */
    BYTE UseHMA;           /* True if "Uses High Memory" is NOT set(Adv.) */
    BYTE Unused1;          /* Unused?                                     */
    BYTE EMS_Locked;       /* True if "EMS Memory Locked" (Adv.) is set   */
} FLAGS386;


/* These bits are specific to "Enhanced" mode (Advanced) */
/* First, here's the structure of the XMS flags in the file */
/*
typedef struct {
    int XMS_Locked :1;       * Flag - True if "XMS Memory Locked" is set * 
    int Allow_FastPst :1;    * Flag - True if "Allow Fast Paste" is set  * 
    int Lock_App :1;         * Flag - True if "Lock App Memory" is set   * 
    int Unused3 :5+8;        * Unused?                                   * 
} FLAGSXMS;
*/

/* Here's the structure used by pifdump to store the data */
typedef struct {
    BYTE XMS_Locked;        /* Flag - True if "XMS Memory Locked" is set */
    BYTE Allow_FastPst;     /* Flag - True if "Allow Fast Paste" is set  */
    BYTE Lock_App;          /* Flag - True if "Lock App Memory" is set   */
    BYTE Unused3[13];       /* Unused?                                   */
} FLAGSXMS;


/* These bits are specific to "Enhanced" mode (Advanced and Normal) */
/* First, here's the structure of the video flags in the file */
/*
typedef struct {
    int EmulateText :1;     * True if "Monitor Ports: Emulate Text" (A) set* 
    int MonitorText :1;     * True if "Mon Ports: Text" (A) is NOT set     * 
    int MonitorLoGr :1;     * True if "Mon Ports: Low Graphics" (A) NOT set* 
    int MonitorHiGr :1;     * True if "Mon Ports: Hi Graphics" (A) NOT set * 
    int InitModeText :1;    * True if "Video Memory: Text" is set          * 
    int InitModeLoGr :1;    * True if "Video Memory: Low Graphics" is set  * 
    int InitModeHiGr :1;    * True if "Video Memory: Hi Graphics" is set   * 
    int RetainVideo :1;     * True if "Monitor Ports: Retain Video"(A) set * 
    int VideoUnused :8;     * Unused?                                      * 
} VIDEO;
*/

/* Here's the structure used by pifdump to store the data */
typedef struct {
    BYTE EmulateText;      /* True if "Monitor Ports: Emulate Text" (A) set*/
    BYTE MonitorText;      /* True if "Mon Ports: Text" (A) is NOT set     */
    BYTE MonitorLoGr;      /* True if "Mon Ports: Low Graphics" (A) NOT set*/
    BYTE MonitorHiGr;      /* True if "Mon Ports: Hi Graphics" (A) NOT set */
    BYTE InitModeText;     /* True if "Video Memory: Text" is set          */
    BYTE InitModeLoGr;     /* True if "Video Memory: Low Graphics" is set  */
    BYTE InitModeHiGr;     /* True if "Video Memory: Hi Graphics" is set   */
    BYTE RetainVideo;      /* True if "Monitor Ports: Retain Video"(A) set */
    BYTE VideoUnused[8];   /* Unused?                                      */
} VIDEO;


/* These bits are specific to "Enhanced" mode (Advanced) */
/* First, here's the structure of the hotkey flags in the file */
/*
typedef struct {
    int HOT_KEYSHIFT :1;     * Flag - marks whether hot key uses "shift" *
    int Unused4 :1;          * Unused?                                   *
    int HOT_KEYCTRL :1;      * Flag - marks whether hot key uses "ctrl"  *
    int HOT_KEYALT :1;       * Flag - marks whether hot key uses "alt"   *
    int Unused5 :4+8;        * Unused?                                   *
} HOTKEY;
*/

/* Here's the structure used by pifdump to store the data */
typedef struct {
    BYTE HOT_KEYSHIFT;      /* Flag - marks whether hot key uses "shift" */
    BYTE Unused4;           /* Unused?                                   */
    BYTE HOT_KEYCTRL;       /* Flag - marks whether hot key uses "ctrl"  */
    BYTE HOT_KEYALT;        /* Flag - marks whether hot key uses "alt"   */
    BYTE Unused5[12];       /* Unused?                                   */
} HOTKEY;


/* These bits are specific to "Standard" mode */
/* First, here's the structure of some 286 flags in the file */
/*
typedef struct {
    int AltTab286 :1;        * True if "Alt+Tab" shortcut key is set   * 
    int AltEsc286 :1;        * True if "Alt+Esc" shortcut key is set   * 
    int AltPrtScr286 :1;     * True if "Alt+PrtSc" shortcut key is set * 
    int PrtScr286 :1;        * True if "PrtSc" shortcut key is set     * 
    int CtrlEsc286 :1;       * True if "Ctrl+Esc" shortcut key is set  * 
    int NoSaveScreen :1;     * True if "No Save Screen" is set         * 
    int Unused10 :2;         * Unused?                                 * 
} FLAGS286;
*/

/* Here's the structure used by pifdump to store the data */
typedef struct {
    BYTE AltTab286;          /* True if "Alt+Tab" shortcut key is set   */
    BYTE AltEsc286;          /* True if "Alt+Esc" shortcut key is set   */
    BYTE AltPrtScr286;       /* True if "Alt+PrtSc" shortcut key is set */
    BYTE PrtScr286;          /* True if "PrtSc" shortcut key is set     */
    BYTE CtrlEsc286;         /* True if "Ctrl+Esc" shortcut key is set  */
    BYTE NoSaveScreen;       /* True if "No Save Screen" is set         */
    BYTE Unused10[2];        /* Unused?                                 */
} FLAGS286;


/* These bits are specific to "Standard" mode */
/* First, here's the structure of the COM port data in the file */
/*
typedef struct {
    int Unused11 :4+2;       * Unused?                                 *
    int Com3 :1;             * True if "Directly Modifies Com3" is set *
    int Com4 :1;             * True if "Directly Modifies Com4" is set *
} COMPORT;
*/

/* Here's the structure used by pifdump to store the data */
typedef struct {
    BYTE Unused11[6];       /* Unused?                                 */
    BYTE Com3;              /* True if "Directly Modifies Com3" is set */
    BYTE Com4;              /* True if "Directly Modifies Com4" is set */
} COMPORT;


/* This is the structure for an "Enhanced" mode record */
typedef struct {
    short mem_limit;         /* Memory Requirements: KB Desired      */
    short mem_req;           /* Memory Requirements: KB Required     */
    WORD for_pri;            /* foreground priority of task          */
    WORD back_pri;           /* background priority of task          */
    short ems_max;           /* EMS Memory: KB Limit                 */
    WORD ems_min;            /* EMS Memory: KB Required              */
    short xms_max;           /* XMS Memory: KB Limit                 */
    WORD xms_min;            /* XMS Memory: KB Required              */
    BYTE flags_386[2];       /* mix of 386 normal/advanced flags     */
                             /* These make up the FLAGS386 structure */
    BYTE flags_XMS[2];       /* flags for 386-advanced memory use    */
                             /* These make up the FLAGSXMS structure */
    BYTE video[2];           /* flags for graphics mode/monitor port */
                             /* These make up the VIDEO structure    */
    WORD zero1;              /* Unknown - always zero?               */
    WORD hot_key_scan;       /* determined by the hot key character  */
    BYTE hot_key_state[2];   /* determines alt/ctrl/shift status     */
                             /* These make up the HOTKEY structure   */
    WORD hot_key_flag;       /* 0=no hot key, 0xF= hot key defined   */
    WORD hk_numflag;         /* Flag if hotkey uses numeric keypad   */
    WORD zero2[4];           /* Unknown - always zero?               */
    char opt_params[64];     /* opt params for enhanced mode         */
} DATA386;

/* This is the structure for a "Standard" mode record */
typedef struct {
    WORD xmsLimit286;    /* If xmsReq286 < 128, this is 128, else it */
                         /* equals xmsReq286                         */
    WORD xmsReq286;      /* Memory Req.: KB Required field           */
    BYTE flags_286;      /* Structure for 286-specific fields        */
                         /* This byte makes up the FLAGS286 structure*/
    BYTE com_ports;      /* Information on modified COM ports        */
                         /* This byte makes up the COMPORT structure */
} DATA286;

/* This is the header that starts every MS Windows PIF file. */
typedef struct {

    /* from 0 -170 hex, not used by Windows, unless so indicated. */
    /* Note that in some cases the PIF editor fills in a value,     */
    /* even though it does not SEEM to be used        */

    BYTE resv1;
    BYTE checksum;               /* used by Windows                   */
    char title[30];              /* 02 used by 286,386 mode for title */
    short max_mem;               /* 20h used byt 286, 386 mem size  */
    short min_mem;               /* 22h, these 2 are duplicates see 19c */
    char prog_path[63];          /* 24h used by x86 modes for program & path*/
    BYTE close_on_exit;          /* 63h, 286 and 386 modes     */
                         /* This byte makes up the CLOSEONEXIT structure */
    BYTE def_drv;                /* 64h  */
    char def_dir[64];            /* 65h used by 286,386 mode for start dir */
    char prog_param[64];         /* a5, used by 286 */
    BYTE initial_screenMode;     /* usually zero, sometimes 0x7F */
    BYTE text_pages;             /* always 0x1                   */
    BYTE first_interrupt;        /* always 0x0                   */
    BYTE last_interrupt;         /* always 0xFF                  */
    BYTE rows;                   /* usually 25                   */
    BYTE cols;                   /* usually 80                   */
    BYTE window_pos_row;
    BYTE window_pos_col;
    WORD sys_memory;             /* always 7                     */
    char shared_prog_name[64];
    char shared_data_file[64];
    BYTE flags1;                 /* 5th bit=Directly Mods Keybd  */
    BYTE flags2;                 /* 170, usually zero            */

} PIF; /* PIF structure    */

typedef struct {
    SECTIONNAME SName;
    SECTIONHDR Hdr;
} BLOCKNT;

typedef struct {
    BYTE Hdr1[3];
    BYTE HChkSum;
} SECTIONHDR1;

typedef struct {
    SECTIONHDR1 CHdr1;
    char CAux[8+1+3];
} COMMENTS;

/* pifstruc.h - end */

