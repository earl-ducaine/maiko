/* $Id: initkbd.c,v 1.2 1999/01/03 02:07:09 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1995 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef DOS
#include <sys/file.h>
#include <sys/select.h>
#endif /* DOS */

#ifdef DOS
#include <i32.h>   /* "#pragma interrupt" & '_chain_intr'*/
#include <dos.h>   /* defines REGS & other structs       */
#include <stdio.h> /* define NULL                        */
#include <conio.h>
#include <time.h>
#include <stk.h>
#endif /* DOS */

#ifdef SUNDISPLAY
#include <sundev/kbd.h>
#include <sundev/kbio.h>
#include <sunwindow/window_hs.h>
#include <sunwindow/cms.h>
#include <sys/ioctl.h>
#include <sunwindow/win_ioctl.h>
#include <pixrect/pixrect_hs.h>
#endif /* SUNDISPLAY */

#ifdef XWINDOW
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include "XKeymap.h"
#include "xdefs.h"
#endif /* XWINDOW */

#include "lispemul.h"
#include "lispmap.h"
#include "lspglob.h"
#include "adr68k.h"
#include "address.h"

#include "devconf.h"
#include "iopage.h"
#include "ifpage.h"
#include "keyboard.h"

#include "initkbddefs.h"
#include "initdspdefs.h"

#ifdef XWINDOW
#include "devif.h"
#include "xinitdefs.h"
extern DspInterface currentdsp;
#endif /* XWINDOW */

#ifdef DOS
#include "devif.h"
extern MouseInterface currentmouse;
extern KbdInterface currentkbd;
extern DspInterface currentdsp;
#endif /* DOS */
#ifdef SUNDISPLAY
extern struct screen LispScreen;
#endif /* SUNDISPLAY */

extern int LispWindowFd;
int LispKbdFd = -1;

/*   for debug    */
int DebugKBD = NIL;
FILE *KBlog;

extern fd_set LispReadFds;
#ifdef SUNDISPLAY
struct inputmask LispEventMask;
#endif /* SUNDISPLAY */

IOPAGE *IOPage68K;

DLword *EmMouseX68K;
DLword *EmMouseY68K;
DLword *EmCursorX68K;
DLword *EmCursorY68K;
DLword *EmRealUtilin68K;
DLword *EmUtilin68K;
DLword *EmKbdAd068K;
DLword *EmKbdAd168K;
DLword *EmKbdAd268K;
DLword *EmKbdAd368K;
DLword *EmKbdAd468K;
DLword *EmKbdAd568K;
DLword *EmDispInterrupt68K;
DLword *EmCursorBitMap68K;

/*u_char SUNLispKeyMap[128];*/
u_char *SUNLispKeyMap;


/* keymap for type3 */
u_char XKBLispKeyMap[128] = {
    /*   0 */ 255,  61, 255,  91, 255,  97,  99, 255,
    /*   8 */ 100, 255,  67, 255,  68, 255, 101, 255,
    /*  16 */  66, 104,  80,  47, 255,  73,  74,  75,
    /*  24 */ 255,  92,  63, 255, 255,  33,  32,  17,
    /*  32 */  16,   1,   0,   2,   4,  53,  22,   8,
    /*  40 */  10,  59,  45,  13, 255,  81,  82,  83,
    /*  48 */ 255,  14, 255,  62, 255,  34,  19,  18,
    /*  56 */   3,  48,  49,  51,   6,  23,  25,  11,
    /*  64 */  58,  29,  15, 255,  84,  85,  87, 255,
    /*  72 */ 111,  89, 255, 255,  36,  21,  20,   5,
    /*  80 */  35,  50,  52,  38,   9,  26,  43,  28,
    /*  88 */ 105,  44, 255,  94,  69,  70, 255,  90,
    /*  96 */ 255,  46, 255,  41,  40,  24,  37,   7,
    /* 104 */  39,  54,  55,  27,  42,  12,  60,  71,
    /* 112 */  98,  76,  72, 255, 255, 255, 255,  56,
    /* 120 */  31,  57,  93, 255, 255, 255, 255, 255
};

/* keymap for type3 */
u_char SUNLispKeyMap_for3[128] = {
    /*   0 */ 255,  61, 255,  91, 255,  97,  99, 255,
    /*   8 */ 100, 255,  67, 255,  68, 255, 101, 255,
    /*  16 */  66, 104,  80,  47, 255,  73,  74,  75,
    /*  24 */ 255,  92,  63, 255, 255,  33,  32,  17,
    /*  32 */  16,   1,   0,   2,   4,  53,  22,   8,
    /*  40 */  10,  59,  45,  13, 255,  81,  82,  83,
    /*  48 */ 255,  14, 255,  62, 255,  34,  19,  18,
    /*  56 */   3,  48,  49,  51,   6,  23,  25,  11,
    /*  64 */  58,  29,  15, 255,  84,  85,  87, 255,
    /*  72 */ 111,  89, 255, 255,  36,  21,  20,   5,
    /*  80 */  35,  50,  52,  38,   9,  26,  43,  28,
    /*  88 */ 105,  44, 255,  94,  69,  70, 255,  90,
    /*  96 */ 255,  46, 255,  41,  40,  24,  37,   7,
    /* 104 */  39,  54,  55,  27,  42,  12,  60,  71,
    /* 112 */  98,  76,  72, 255, 255, 255, 255,  56,
    /* 120 */  31,  57,  93, 255, 255, 255, 255, 255
};

/* for type4 */

u_char SUNLispKeyMap_for4[128] = {
    /*   0 */ 255,  61, 255,  91, 255,  97,  99, 106,
    /*   8 */ 100, 107,  67, 108,  68,  47, 101,  30,
    /*  16 */  66, 104,  80,  31, 255,  75, 110,  74,
    /*  24 */ 255, 109,  63, 255, 255,  33,  32,  17,
    /*  32 */  16,   1,   0,   2,   4,  53,  22,   8,
    /*  40 */  10,  59,  45,  15, 255,  64,  65,  95,
    /*  48 */ 255,  14,  13,  89, 255,  34,  19,  18,
    /*  56 */   3,  48,  49,  51,   6,  23,  25,  11,
    /*  64 */  58,  29,  13,  93,  81,  82,  83,  96,
    /*  72 */ 111,  62, 255, 255,  36,  21,  20,   5,
    /*  80 */  35,  50,  52,  38,   9,  26,  43,  28,
    /*  88 */ 105,  44,  76,  84,  85,  87,  98,  90,
    /*  96 */ 255,  46,  73,  41,  40,  24,  37,   7,
    /* 104 */  39,  54,  55,  27,  42,  12,  60,  71,
    /* 112 */  94,  69,  70, 255, 255, 255,  92,  56,
    /* 120 */  86,  57,  88, 255, 103, 102, 255, 255
};

/* for jle */

u_char SUNLispKeyMap_jle[128] = {
    /*   0 */ 255,  61, 255,  91, 255,  97,  99, 106,
    /*   8 */ 100, 107,  67, 108,  68,  47, 101,  71,
    /*  16 */  66, 104,  80,  31, 255,  75, 110,  74,
    /*  24 */ 255, 109,  63, 255, 255,  33,  32,  17,
    /*  32 */  16,   1,   0,   2,   4,  53,  22,   8,
    /*  40 */  59,  45,  30,  15, 255,  64,  65,  95,
    /*  48 */ 255,  14,  13,  89, 255,  34,  19,  18,
    /*  56 */   3,  48,  49,  51,   6,  23,  25,  11,
    /*  64 */  58,  29,  13,  93,  81,  82,  83,  96,
    /*  72 */ 111,  62, 255, 255,  36,  21,  20,   5,
    /*  80 */  35,  50,  52,  38,   9,  26,  43,  28,
    /*  88 */ 105,  44,  76,  84,  85,  87,  98,  90,
    /*  96 */ 255,  46,  73,  41,  40,  24,  37,   7,
    /* 104 */  39,  54,  55,  27,  42,  12,  60,  10,
    /* 112 */  94,  69,  70,  72, 103, 109,  92,  56,
    /* 120 */  86,  57,  88, 255, 255, 102, 255, 255
};
/* [40]   10 -> 59  */
/* [41]   59 -> 45  */
/* [42]   45 -> 30  */
/* [111]  71 -> 10  */
/* [115] 255 -> 72  Kakutei */
/* [116] 255 -> 103 Henkan */
/* [117] 255 -> 109 Nihongo On-Off */

u_char *XGenericKeyMap; /* filled in with malloc if needed */

/* For the IBM-101 kbd FF marks exceptions */

#ifdef NEVER
u_char DOSLispKeyMap_101[0x80] = {
    /*         0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f */

    /* 0*/ 0xff, 33,   32,   17,   16,   1,    0,    2,    4,    53,   22,   8,    10,   59,   15,   34,
    /* 1*/ 19,   18,   3,    48,   49,   51,   6,    23,   25,   11,   58,   29,   44,   36,   21,   20,
    /* 2*/ 5,    35,   50,   52,   38,   9,    26,   43,   28,   45,   41,   105,  40,   24,   37,   7,
    /* 3*/ 39,   54,   55,   27,   42,   12,   60,   95,   31,   57,   56,   97,   99,   100,  67,   68,
    /* 4*/ 101,  66,   104,  80,   106,  73,   74,   81,   82,   83,   96,   84,   85,   87,   102,  94,
    /* 5*/ 69,   70,   98,   13,   0xff, 0xff, 0xff, 107,  108,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    /* 6*/ 89,   62,   63,   46,   90,   91,   130,  129,  131,  132,  92,   61,   0xff, 0xff, 0xff, 0xff,
    /* 7*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif /* NEVER */

/* For the IBM-101 kbd FF marks exceptions */
u_char DOSLispKeyMap_101[0x80] = {
    /*         0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f */

    /* 0*/ 0xff, 33,   32,   17,   16,   1,    0,    2,    4,    53,   22,   8,    10,   59,   15,   34,
    /* 1*/ 19,   18,   3,    48,   49,   51,   6,    23,   25,   11,   58,   29,   44,   36,   21,   20,
    /* 2*/ 5,    35,   50,   52,   38,   9,    26,   43,   28,   45,   41,   105,  40,   24,   37,   7,
    /* 3*/ 39,   54,   55,   27,   42,   12,   60,   95,   31,   57,   56,   97,   99,   100,  67,   68,
    /* 4*/ 101,  66,   104,  80,   106,  73,   74,   62,   130,  63,   96,   129,  85,   132,  102,  90,
    /* 5*/ 131,  91,   89,   46,   0xff, 0xff, 0xff, 107,  108,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    /* 6*/ 89,   62,   63,   46,   90,   91,   130,  129,  131,  132,  92,   61,   0xff, 0xff, 0xff, 0xff,
    /* 7*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

void init_keyboard(int flg) /* if 0 init else re-init */
{
#ifdef SUNDISPLAY
  int keytrans;
#endif

  set_kbd_iopointers();

#ifdef SUNDISPLAY
  if ((LispKbdFd = open(LispScreen.scr_kbdname, O_RDWR)) == -1) {
    fprintf(stderr, "can't open %s\n", LispScreen.scr_kbdname);
    exit(-1);
  }
#endif /* SUNDISPLAY */

  if (flg == 0) { keyboardtype(LispKbdFd); }

#ifdef SUNDISPLAY
  keytrans = TR_UNTRANS_EVENT; /* keyboard does not encode key */
  if (ioctl(LispKbdFd, KIOCTRANS, &keytrans) == -1) {
    fprintf(stderr, "Error at ioctl errno =%d\n", errno);
    exit(-1);
  }
  close(LispKbdFd);
#ifdef KBINT
  int_io_open(LispWindowFd); /* from init_dsp, try to prevent mouse-move-no-kbd bug */
#endif                       /*  KBINT */
  seteventmask(&LispEventMask);
#elif XWINDOW
  init_Xevent(currentdsp);

#elif DOS
  if (flg == 0) { /* Install the handlers ONLY when we */
    /* init the kbd the init the kbd the */
    /* first time. */

    /* turn on kbd */
    make_kbd_instance(currentkbd);
    (currentkbd->device.enter)(currentkbd);

    /* turn on mouse */
    make_mouse_instance(currentmouse);
    (currentmouse->device.enter)(currentmouse, currentdsp);
  }
#endif /* XWINDOW DOS */
}

/*  ----------------------------------------------------------------*/

void device_before_exit() {
#ifdef SUNDISPLAY
  int keytrans;

  if ((LispKbdFd = open(LispScreen.scr_kbdname, O_RDWR)) == -1) {
    fprintf(stderr, "can't open %s\n", LispScreen.scr_kbdname);
    exit(-1);
  }

  keytrans = TR_EVENT; /* keyboard encodes key */
  if (ioctl(LispKbdFd, KIOCTRANS, &keytrans) == -1) {
    fprintf(stderr, "Error at ioctl errno =%d\n", errno);
    exit(-1);
  }
  close(LispKbdFd);

#elif DOS
  (currentmouse->device.exit)(currentmouse, currentdsp);
  (currentkbd->device.exit)(currentkbd);
#endif /* SUNDISPLAY DOS*/
  display_before_exit();
}

/*  ----------------------------------------------------------------*/

void set_kbd_iopointers() {
  IOPage68K = (IOPAGE *)IOPage;
  EmMouseX68K = (DLword *)&(IOPage68K->dlmousex);
  EmMouseY68K = (DLword *)&(IOPage68K->dlmousey);
  EmCursorX68K = (DLword *)&(IOPage68K->dlcursorx);
  EmCursorY68K = (DLword *)&(IOPage68K->dlcursory);
  EmRealUtilin68K = (DLword *)&(IOPage68K->dlutilin);
  /* EmUtilin68K is for KEYDOWNP1 macro or etc. */
  EmUtilin68K = (DLword *)&(InterfacePage->fakemousebits);
  EmKbdAd068K = (DLword *)&(IOPage68K->dlkbdad0);
  EmKbdAd168K = (DLword *)&(IOPage68K->dlkbdad1);
  EmKbdAd268K = (DLword *)&(IOPage68K->dlkbdad2);
  EmKbdAd368K = (DLword *)&(IOPage68K->dlkbdad3);
  EmKbdAd468K = (DLword *)&(IOPage68K->dlkbdad4);
  EmKbdAd568K = (DLword *)&(IOPage68K->dlkbdad5);
  EmDispInterrupt68K = (DLword *)&(IOPage68K->dldispinterrupt);
  EmCursorBitMap68K = (DLword *)(IOPage68K->dlcursorbitmap);

  *EmRealUtilin68K = KB_ALLUP;
  *EmKbdAd068K = KB_ALLUP;
  *EmKbdAd168K = KB_ALLUP;
  *EmKbdAd268K = KB_ALLUP;
  *EmKbdAd368K = KB_ALLUP;
  *EmKbdAd468K = KB_ALLUP;
  *EmKbdAd568K = KB_ALLUP;
}

/*  ----------------------------------------------------------------*/

#ifdef SUNDISPLAY
void seteventmask(struct inputmask *eventmask)
{
  input_imnull(eventmask);
  eventmask->im_flags |= IM_ASCII | IM_NEGASCII | IM_NEGEVENT;

  win_setinputcodebit(eventmask, MS_LEFT);
  win_setinputcodebit(eventmask, MS_MIDDLE);
  win_setinputcodebit(eventmask, MS_RIGHT);
  win_setinputcodebit(eventmask, LOC_MOVE);
  win_unsetinputcodebit(eventmask, LOC_STILL);
  win_unsetinputcodebit(eventmask, LOC_MOVEWHILEBUTDOWN);
  win_unsetinputcodebit(eventmask, LOC_WINENTER);
  win_unsetinputcodebit(eventmask, LOC_WINEXIT);

  win_setinputmask(LispWindowFd, eventmask, eventmask, WIN_NULLLINK);
}
#endif /* SUNDISPLAY */

#define MIN_KEYTYPE 3
#define KB_AS3000J (7 + MIN_KEYTYPE)
#define KB_RS6000 (8 + MIN_KEYTYPE) /* TODO: Can we remove this? */
#define KB_DEC3100 (9 + MIN_KEYTYPE) /* TODO: Can we remove this? */
#define KB_HP9000 (10 + MIN_KEYTYPE)  /* TODO: Can we remove this? */
#define KB_X (11 + MIN_KEYTYPE)
#define KB_DOS (12 + MIN_KEYTYPE)

/* KB_SUN4 not defined in older OS versions */
#ifndef KB_SUN4
#define KB_SUN4 4
#endif

#ifndef KB_SUN2
/* These KB types nog defined outside Sun world,so define them here */
#define KB_SUN2 2
#define KB_SUN3 3
#endif /* KB_SUN2 */

/* For the JLE keyboard */
#define KB_JLE 5


#ifdef XWINDOW
/*
 * 
 */

static int find_unused_key(KeySym *map, int minkey, int codecount, int symspercode, int sym, u_char *table)
{
  int i;

  for (i = 0; i < (codecount * symspercode); i++) {
    if (sym == map[i]) {
      int code = minkey + (i / symspercode);
      if (table[code - 7] != 255) {
#ifdef DEBUG
          printf("table[%d - 7] != 255\n", code);
#endif
          continue;
      }
      return (code);
    }
  }
  return (0);
}

/************************************************************************/
/*									*/
/*			m a k e _ X _ s y m b o l m a p			*/
/*									*/
/*	Starting from the generic-X-keyboard mapping in XKeymap.h,	*/
/*	construct a keyboard map for this machine, using the rules	*/
/*	shown in the header file.					*/
/*									*/
/*									*/
/************************************************************************/

// Our goal is to create a symbol map, similar to the keymap that maps
// X11 symbols found in X11/keysymdef.h to Xerox-style keycodes. The
// first step in this is to provide a table that translates symbols
// into their Sun Workstation keycode:


// From xkb/keycodes/sun

#define ESC 36
#define AE01 37
#define AE02 38
#define AE03 39
#define AE04 40
#define AE05 41
#define AE06 42
#define AE07 43
#define AE08 44
#define AE09 45
#define AE10 46
#define AE11 47
#define AE12 48
#define TLDE 49
#define BKSP 50

#define TAB 60
#define AD01 61
#define AD02 62
#define AD03 63
#define AD04 64
#define AD05 65
#define AD06 66
#define AD07 67
#define AD08 68
#define AD09 69
#define AD10 70
#define AD11 71
#define AD12 72
#define DELE 73
#define COMP 74
#define ALGR 20


#define LCTL 83
#define AC01 84
#define AC02 85
#define AC03 86
#define AC04 87
#define AC05 88
#define AC06 89
#define AC07 90
#define AC08 91
#define AC09 92
#define AC10 93
#define AC11 94
#define BKSL 95
#define RTRN 96

#define LFSH 106
#define AB01 107
#define AB02 108
#define AB03 109
#define AB04 110
#define AB05 111
#define AB06 112
#define AB07 113
#define AB08 114
#define AB09 115
#define AB10 116
#define RTSH 117

#define LALT 26
#define CAPS 126
#define LMTA 127
#define SPCE 128
#define RMTA 129

#define FK01 12
#define FK02 13
#define FK03 15
#define FK04 17
#define FK05 19
#define FK06 21
#define FK07 23
#define FK08 24
#define FK09 25
#define FK10 14
#define FK11 16
#define FK12 18
#define STOP 8
#define AGAI 10
#define PROP 32
#define UNDO 33
#define FRNT 56
#define COPY 58
#define OPEN 79
#define PAST 80
#define FIND 102
#define CUT 104

#define PRSC 29
#define SCLK 30
#define PAUS 28

#define NMLK 105
#define KPDV 53
#define KPMU 54
#define KPSU 78

#define KP7 75
#define KP8 76
#define KP9 77
#define KPAD 132

#define KP4 98
#define KP5 99
#define KP6 100

#define KP1 119
#define KP2 120
#define KP3 121
#define KPEN 97

#define KP0 101
#define KPDL 57

#define UP  27
#define LEFT 31
#define DOWN 34
#define RGHT 35

#define INS 51
#define HOME 59
#define END 81
#define PGUP 103
#define PGDN 130
#define HELP 125

#define MUTE 52
#define VOL_MINUS 9
#define VOL_PLUS 11
#define POWR 55

    


// keycodes to symbols. Note, capitals and other characters requiring
// key combinations need to be handled specially.

#define grave '`'
#define minus '-'
#define equal '='
#define backetleft '['
#define bracketright ']'
#define semicolon ';'
#define comma ','
#define period '.'
#define slash '/'
#define backslash '\\'
#define bracketleft '['
#define apostrophe '['


char* symbolic_mapping =
  {
   // SHIFT + grave ==> asciitilde
   TLDE, grave,

    // SHIFT + 1 ==> exclam  
AE01, '1',

    // SHIFT + 2 ==> at
AE02, '2',

    // SHIFT + 3 ==> numbersign
AE03, '3',

    // SHIFT + 4 ==> dollar
AE04, '4',

    // SHIFT + 5 ==> percent
AE05, '5',

    // SHIFT + 6 ==> asciicircum
AE06, '6',
  
    // SHIFT + 7 ==> ambersand
AE07, '7',

    // SHIFT + 8 ==> asterisk
AE08, '8',

    // SHIFT + 9 ==> parenleft
AE09, '9',

    // SHIFT + 0 ==> parenright  
AE10, '0',

   
    // SHIFT + minus ==> underscore
AE11, minus,

   
    // SHIFT + equal ==> plus
AE12, equal,

// These include lower and uppercase.
AD01, 'q',
   
AD02, 'w',
AD03, 'e',
AD04, 'r',
AD05, 't',
AD06, 'y',
AD07, 'u',
AD08, 'i',
AD09, 'o',
AD10, 'p',

   
// SHIFT + bracketleft ==> braceleft
AD11, bracketleft,

   
// SHIFT + bracketright ==> braceright
AD12, bracketright,

AC01, 'a',
AC02, 's',
AC03, 'd',
AC04, 'f',
AC05, 'g',
AC06, 'h',
AC07, 'j',
AC08, 'k',
AC09, 'l',

   
// SHIFT + semicolon ==> colon  
AC10, semicolon,

// SHIFT + apostrophe ==> quotedbl
AC11, apostrophe,

AB01, 'z',
AB02, 'x',
AB03, 'c',
AB04, 'v',
AB05, 'b',
AB06, 'n',
   AB07, 'm',

   
// SHIFT + comma ==> less  
AB08, comma,


   
// SHIFT + period ==> greater
   AB09, period,


// SHIFT + slash ==> question
AB10, slash,


// SHIFT + backslash ==> bar  
BKSL, backslash };






static u_char *make_X_symbolmap() {
  u_char *table = (u_char *)malloc(256); /* the final result table */
  int lisp_codes_used[256];              /* Keep track of the Lisp key #s we've used */
  int last_KEYSYM = -1;
  int sym_used = 0;
  int *key_sym_pairs = generic_X_keymap;
  int i = 0;
  KeySym *mapping;
  int codecount, symspercode, minkey, maxkey;

  for (; i < 256; i++) { /* clear the tables we just allocated */
    lisp_codes_used[i] = 0;
    table[i] = 255; /* The "no key assigned" code */
  }

  XLOCK;
  XDisplayKeycodes(currentdsp->display_id, &minkey, &maxkey);
  codecount = maxkey + 1 - minkey;
  mapping = XGetKeyboardMapping(currentdsp->display_id, minkey, codecount, &symspercode);
  XUNLOCK;

  for (; *key_sym_pairs != -1;) {
    int reusable = *key_sym_pairs++, code = *key_sym_pairs++, sym = *key_sym_pairs++, xcode;

    if (sym_used && (sym == last_KEYSYM)) continue;

    sym_used = 0;
    last_KEYSYM = sym;

    xcode = find_unused_key(mapping, minkey, codecount, symspercode, sym, table);

    if (xcode == 0) continue;
    if ((!reusable) && (lisp_codes_used[code] != 0)) continue;

    sym_used = 1;
    last_KEYSYM = sym;
    lisp_codes_used[code] = 1;
    table[xcode - 7] = code;
  }

  XFree(mapping); /* No locking required? */

#ifdef DEBUG
  printf("\n\n\tXGetKeyboardMapping table\n\n");
  for (i = 0; i < codecount * symspercode; i += symspercode) {
      printf("%d:", minkey + (i / symspercode));
      for (int j = 0; j < symspercode; j++) {
          printf("  %8x", mapping[i+j]);
      }
      printf("\n");
  }

  printf("\n\n\tKeyboard mapping table\n\n");
  for (i = 0; i < 256; i += 8) {
    printf("%d:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i, table[i], table[i + 1], table[i + 2], table[i + 3], table[i + 4],
           table[i + 5], table[i + 6], table[i + 7]);
  }
#endif /* DEBUG */
  
  return (table);
}


uint *key_sym_to_code_map;

/* return corresponding keycode from table, -1 means that keycode
   couldn't be found. */
int get_keycode_from_keysym(uint keysym) {
  for (int i = 0; i < 255; i++) {
    if (key_sym_to_code_map[i * 2] == keysym)
      return key_sym_to_code_map[(i * 2) + 1];
  }
  return -1;
}

make_X_symmap () {
  /* Table with two columns, first is keysym as uint, secord is
     keycode as int. Array is unordered, so full table must be scanned
     to file the corresponding keycode. */
  int i = 0;
  key_sym_to_code_map = malloc(sizeof(uint) * 255 * 2);
  /*   8:       0x0  */
  key_sym_to_code_map[i++] = 0x0;
  key_sym_to_code_map[i++] = 8;
  /*   9:    0xff1b  */
  key_sym_to_code_map[i++] = 0xff1b;
  key_sym_to_code_map[i++] = 9;
  /*  10(10):      0x31(1)  */
  key_sym_to_code_map[i++] = 0x31;
  key_sym_to_code_map[i++] = 10;
  /*  11(11):      0x32(2)  */
  key_sym_to_code_map[i++] = 0x32;
  key_sym_to_code_map[i++] = 11;
  /*  12(12):      0x33(3)  */
  key_sym_to_code_map[i++] = 0x33;
  key_sym_to_code_map[i++] = 12;
  /*  13(13):      0x34(4)  */
  key_sym_to_code_map[i++] = 0x34;
  key_sym_to_code_map[i++] = 13;
  /*  14(14):      0x35(5)  */
  key_sym_to_code_map[i++]= 0x35;
  key_sym_to_code_map[i++] = 14;
  /*  15(15):      0x36(6)  */
  key_sym_to_code_map[i++] = 0x36;
  key_sym_to_code_map[i++] = 15;
  /*  16(16):      0x37(7)  */
  key_sym_to_code_map[i++] = 0x37;
  key_sym_to_code_map[i++] = 16;
  /*  17(17):      0x38(8)  */
  key_sym_to_code_map[i++] = 0x38;
  key_sym_to_code_map[i++] = 17;
  /*  18(18):      0x39(9)  */
  key_sym_to_code_map[i++] = 0x39;
  key_sym_to_code_map[i++] = 18;
  /*  19(19):      0x30(0)  */
  key_sym_to_code_map[i++] = 0x30;
  key_sym_to_code_map[i++] = 19;
  /*  20(61):      0x2f(/)  */
  key_sym_to_code_map[i++] = 0x2f;
  key_sym_to_code_map[i++] = 61;
  /*  21(21):      0x3d(=)  */
  key_sym_to_code_map[i++] = 0x3d;
  key_sym_to_code_map[i++] = 21;
  /*  22:    0xff08  */
  key_sym_to_code_map[i++] = 0xff08;
  key_sym_to_code_map[i++] = 22;
  /*  23:    0xff09  */
  key_sym_to_code_map[i++] = 0xff09;
  key_sym_to_code_map[i++] = 23;
  /*  24(48):      0x27(')  */
  key_sym_to_code_map[i++] = 0x27;
  key_sym_to_code_map[i++] = 48;
  /*  25(??):      0x2c(,)  */
  key_sym_to_code_map[i++] = 0x2c;
  key_sym_to_code_map[i++] = 25;
  /*  26(??):      0x2e(.)  */
  key_sym_to_code_map[i++] = 0x2e;
  key_sym_to_code_map[i++] = 26;
  /*  27(32):      0x70(p)  */
  key_sym_to_code_map[i++] = 0x70;
  key_sym_to_code_map[i++] = 33;
  /*  28(45):      0x79(y)  */
  key_sym_to_code_map[i++] = 0x79;
  key_sym_to_code_map[i++] = 29;
  /*  29(41):      0x66(f)  */
  key_sym_to_code_map[i++] = 0x66;
  key_sym_to_code_map[i++] = 41;
  /*  30(42):      0x67(g)  */
  key_sym_to_code_map[i++] = 0x67;
  key_sym_to_code_map[i++] = 42;
  /*  31(42):      0x63(c)  */
  key_sym_to_code_map[i++] = 0x63;
  key_sym_to_code_map[i++] = 54;
  /*  32(27):      0x72(r)  */
  key_sym_to_code_map[i++] = 0x72;
  key_sym_to_code_map[i++] = 27;
  /*  33(46):      0x6c(l)  */
  key_sym_to_code_map[i++] = 0x6c;
  key_sym_to_code_map[i++] = 46;
  /*  34(??):      0x28(()  */
  key_sym_to_code_map[i++] = 0x28;
  key_sym_to_code_map[i++] = 34;
  /*  35(??):      0x29())  */
  key_sym_to_code_map[i++] = 0x29;
  key_sym_to_code_map[i++] = 35;
  /*  36:    0xff0d  */
  /*  37:    0xffe3  */
  /*  38(38):      0x61(a)  */
  key_sym_to_code_map[i++] = 0x61;
  key_sym_to_code_map[i++] = 38;
  /*  39(32):      0x6f(o)  */
  key_sym_to_code_map[i++] = 0x6f;
  key_sym_to_code_map[i++] = 32;
  /*  40(26):      0x65(e)  */
  key_sym_to_code_map[i++] = 0x65;
  key_sym_to_code_map[i++] = 26;
  /*  41(30):      0x75(u)  */
  key_sym_to_code_map[i++] = 0x75;
  key_sym_to_code_map[i++] = 30;
  /*  42(31):      0x69(i)  */
  key_sym_to_code_map[i++] = 0x69;
  key_sym_to_code_map[i++] = 31;
  /*  43(40):      0x64(d)  */
  key_sym_to_code_map[i++] = 0x64;
  key_sym_to_code_map[i++] = 40;
  /*  44(43):      0x68(h)  */
  key_sym_to_code_map[i++] = 0x68;
  key_sym_to_code_map[i++] = 43;
  /*  45(28):      0x74(t)  */
  key_sym_to_code_map[i++] = 0x74;
  key_sym_to_code_map[i++] = 28;
  /*  46(57):      0x6e(n)  */
  key_sym_to_code_map[i++] = 0x6e;
  key_sym_to_code_map[i++] = 57;
  /*  47(39):      0x73(s)  */
  key_sym_to_code_map[i++] = 0x73;
  key_sym_to_code_map[i++] = 39;
  /*  48(20):      0x2d(-)  */
  key_sym_to_code_map[i++] = 0x2d;
  key_sym_to_code_map[i++] = 20;
  /*  49(49):      0x60(`)  */
  key_sym_to_code_map[i++] = 0x60;
  key_sym_to_code_map[i++] = 49;
  /*  50:    0xffe1(<r-shift>)  */
  key_sym_to_code_map[i++] = 0xffe1;
  key_sym_to_code_map[i++] = 50;
  /*  51(51):      0x5c(\)  */
  key_sym_to_code_map[i++] = 0x5c;
  key_sym_to_code_map[i++] = 51;
  /*  52(??):      0x3b(;)  */
  key_sym_to_code_map[i++] = 0x3b;
  key_sym_to_code_map[i++] = 52;
  /*  53(24):      0x71(q)  */
  key_sym_to_code_map[i++] = 0x71;
  key_sym_to_code_map[i++] = 24;
  /*  54(31):      0x6a(j)  */
  key_sym_to_code_map[i++] = 0x6a;
  key_sym_to_code_map[i++] = 31;
  /*  55(45):      0x6b(k)  */
  key_sym_to_code_map[i++] = 0x6b;
  key_sym_to_code_map[i++] = 45;
  /*  56(53):      0x78(x)  */
  key_sym_to_code_map[i++] = 0x78;
  key_sym_to_code_map[i++] = 53;
  /*  57(56):      0x62(b)  */
  key_sym_to_code_map[i++] = 0x62;
  key_sym_to_code_map[i++] = 56;
  /*  58(m):      0x6d(m)  */
  key_sym_to_code_map[i++] = 0x6d;
  key_sym_to_code_map[i++] = 58;
  /*  59(25):      0x77(w)  */
  key_sym_to_code_map[i++] = 0x77;
  key_sym_to_code_map[i++] = 25;
  /*  60(55):      0x76(v)  */
  key_sym_to_code_map[i++] = 0x76;
  key_sym_to_code_map[i++] = 55;
  /*  61(52):      0x7a(z)  */
  key_sym_to_code_map[i++] = 0x7a;
  key_sym_to_code_map[i++] = 52;
  /*  62(62):    0xffe2  */
  key_sym_to_code_map[i++] = 0xffe2;
  key_sym_to_code_map[i++] = 62;
  /*  63:    ffaa  */
  /*  64:    ffe9  */
  /*  65(65):      0x20(<space>)  */
  key_sym_to_code_map[i++] = 0x20;
  key_sym_to_code_map[i++] = 65;
  /*  66:    ff67  */
  /*  67:    ffbe  */
  /*  68:    ffbf  */
  /*  69:    ffc0  */
  /*  70:    ffc1  */
  /*  71:    ffc2  */
  /*  72:    ffc3  */
  /*  73:    ffc4  */
  /*  74:    ffc5  */
  /*  75:    ffc6  */
  /*  76:    ffc7  */
  /*  77:    ff7f  */
  /*  78:    ff14  */
  /*  79:    ff95  */
  /*  80:    ff97  */
  /*  81:    ff9a  */
  /*  82:    ffad  */
  /*  83:    ff96  */
  /*  84:    ff9d  */
  /*  85:    ff98  */
  /*  86:    ffab  */
  /*  87:    ff9c  */
  /*  88:    ff99  */
  /*  89:    ff9b  */
  /*  90:    ff9e  */
  /*  91:    ff9f  */
  /*  92:    fe03  */
  /*  93:       0  */
  /*  94:      3c  */
  /*  95:    ffc8  */
  /*  96:    ffc9  */
  /*  97:       0  */
  /*  98:    ff26  */
  /*  99:    ff25  */
  /* 100:    ff23  */
  /* 101:    ff27  */
  /* 102:    ff22  */
  /* 103:       0  */
  /* 104:    ff8d  */
  /* 105:    ffe4  */
  /* 106:    ffaf  */
  /* 107:    ff61  */
  /* 108:    ffea  */
  /* 109:    ff0a  */
  /* 110:    ff50  */
  /* 111:    ff52  */
  /* 112:    ff55  */
  /* 113:    ff51  */
  /* 114:    ff53  */
  /* 115:    ff57  */
  /* 116:    ff54  */
  /* 117:    ff56  */
  /* 118:    ff63  */
  /* 119:    ffff  */
  /* 120:       0  */
  /* 121: 1008ff12 */
  /* 122: 1008ff11 */
  /* 123: 1008ff13 */
  /* 124: 1008ff2a */
  /* 125:    ffbd  */
  /* 126:      b1  */
  /* 127:    ff13  */
  /* 128: 1008ff4a */
  /* 129:    ffae  */
  /* 130:    ff31  */
  /* 131:    ff34  */
  /* 132:       0  */
  /* 133:    ffeb  */
  /* 134:    ffec  */
  /* 135:    ffe4  */
  /* 136:    ff69  */
  /* 137:    ff66  */
  /* 138: 1005ff70 */
  /* 139:    ff65  */
  /* 140: 1005ff71 */
  /* 141: 1008ff57 */
  /* 142: 1008ff6b */
  /* 143: 1008ff6d */
  /* 144:    ff68  */
  /* 145: 1008ff58 */
  /* 146:    ff6a  */
  /* 147: 1008ff65 */
  /* 148: 1008ff1d */
  /* 149:       0  */
  /* 150: 1008ff2f */
  /* 151: 1008ff2b */
  /* 152: 1008ff5d */
  /* 153: 1008ff7b */
  /* 154:       0  */
  /* 155: 1008ff8a */
  /* 156: 1008ff41 */
  /* 157: 1008ff42 */
  /* 158: 1008ff2e */
  /* 159: 1008ff5a */
  /* 160: 1008ff2d */
  /* 161: 1008ff74 */
  /* 162: 1008ff7f */
  /* 163: 1008ff19 */
  /* 164: 1008ff30 */
  /* 165: 1008ff33 */
  /* 166: 1008ff26 */
  /* 167: 1008ff27 */
  /* 168:       0  */
  /* 169: 1008ff2c */
  /* 170: 1008ff2c */
  /* 171: 1008ff17 */
  /* 172: 1008ff14 */
  /* 173: 1008ff16 */
  /* 174: 1008ff15 */
  /* 175: 1008ff1c */
  /* 176: 1008ff3e */
  /* 177: 1008ff6e */
  /* 178:       0  */
  /* 179: 1008ff81 */
  /* 180: 1008ff18 */
  /* 181: 1008ff73 */
  /* 182: 1008ff56 */
  /* 183:       0  */
  /* 184:       0  */
  /* 185: 1008ff78 */
  /* 186: 1008ff79 */
  /* 187:      28  */
  /* 188:      29  */
  /* 189: 1008ff68 */
  /* 190:    ff66  */
  /* 191: 1008ff81 */
  /* 192: 1008ff45 */
  /* 193: 1008ff46 */
  /* 194: 1008ff47 */
  /* 195: 1008ff48 */
  /* 196: 1008ff49 */
  /* 197:       0  */
  /* 198: 1008ffb2 */
  /* 199: 1008ffa9 */
  /* 200: 1008ffb0 */
  /* 201: 1008ffb1 */
  /* 202:       0  */
  /* 203:    ff7e  */
  /* 204:       0  */
  /* 205:       0  */
  /* 206:       0  */
  /* 207:       0  */
  /* 208: 1008ff14 */
  /* 209: 1008ff31 */
  /* 210: 1008ff43 */
  /* 211: 1008ff44 */
  /* 212: 1008ff4b */
  /* 213: 1008ffa7 */
  /* 214: 1008ff56 */
  /* 215: 1008ff14 */
  /* 216: 1008ff97 */
  /* 217:       0  */
  /* 218:    ff61  */
  /* 219:       0  */
  /* 220: 1008ff8f */
  /* 221: 1008ffb6 */
  /* 222:       0    */
  /* 223: 1008ff19 */
  /* 224: 1008ff8e */
  /* 225: 1008ff1b */
  /* 226: 1008ff5f */
  /* 227: 1008ff3c */
  /* 228: 1008ff5e */
  /* 229: 1008ff36 */
  /* 230:       0  */
  /* 231:    ff69  */
  /* 232: 1008ff03 */
  /* 233: 1008ff02 */
  /* 234: 1008ff32 */
  /* 235: 1008ff59 */
  /* 236: 1008ff04 */
  /* 237: 1008ff06 */
  /* 238: 1008ff05 */
  /* 239: 1008ff7b */
  /* 240: 1008ff72 */
  /* 241: 1008ff90 */
  /* 242: 1008ff77 */
  /* 243: 1008ff5b */
  /* 244: 1008ff93 */
  /* 245: 1008ff94 */
  /* 246: 1008ff95 */
  /* 247:       0  */
  /* 248:       0  */
  /* 249:       0  */
  /* 250:       0  */
  /* 251: 1008ff07 */
  /* 252:       0  */
  /* 253:       0  */
  /* 254: 1008ffb4 */
  /* 255: 1008ffb5 */
}




/************************************************************************/
/*									*/
/*			m a k e _ X _ k e y m a p			*/
/*									*/
/*	Starting from the generic-X-keyboard mapping in XKeymap.h,	*/
/*	construct a keyboard map for this machine, using the rules	*/
/*	shown in the header file.					*/
/*									*/
/*									*/
/************************************************************************/

static u_char *make_X_keymap() {
  u_char *table = (u_char *)malloc(256); /* the final result table */
  int lisp_codes_used[256];              /* Keep track of the Lisp key #s we've used */
  int last_KEYSYM = -1;
  int sym_used = 0;
  int *key_sym_pairs = generic_X_keymap;
  int i = 0;
  KeySym *mapping;
  int codecount, symspercode, minkey, maxkey;
  make_X_symmap();

  for (; i < 256; i++) { /* clear the tables we just allocated */
    lisp_codes_used[i] = 0;
    table[i] = 255; /* The "no key assigned" code */
  }

  XLOCK;
  XDisplayKeycodes(currentdsp->display_id, &minkey, &maxkey);
  codecount = maxkey + 1 - minkey;
  mapping = XGetKeyboardMapping(currentdsp->display_id, minkey, codecount, &symspercode);
  XUNLOCK;

  for (; *key_sym_pairs != -1;) {
    int reusable = *key_sym_pairs++, code = *key_sym_pairs++, sym = *key_sym_pairs++, xcode;

    if (sym_used && (sym == last_KEYSYM)) continue;

    sym_used = 0;
    last_KEYSYM = sym;

    xcode = find_unused_key(mapping, minkey, codecount, symspercode, sym, table);

    if (xcode == 0) continue;
    if ((!reusable) && (lisp_codes_used[code] != 0)) continue;

    sym_used = 1;
    last_KEYSYM = sym;
    lisp_codes_used[code] = 1;
    table[xcode - 7] = code;
  }

  XFree(mapping); /* No locking required? */

#ifdef DEBUG
  printf("\n\n\tXGetKeyboardMapping table\n\n");
  for (i = 0; i < codecount * symspercode; i += symspercode) {
      printf("%3d:", minkey + (i / symspercode));
      for (int j = 0; j < symspercode; j++) {
          printf(" %7x", mapping[i+j]);
      }
      printf("\n");
  }

  printf("\n\n\tKeyboard mapping table\n\n");
  for (i = 0; i < 256; i += 8) {
    printf("%d:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i, table[i], table[i + 1], table[i + 2], table[i + 3], table[i + 4],
           table[i + 5], table[i + 6], table[i + 7]);
  }

  // Key symbol to keycode table
  u_char *keysym_to_keycode_table = malloc(sizeof(u_char)*256);
  for (i = 0; i < 256; i++) {
    keysym_to_keycode_table[i] = XKeysymToKeycode(currentdsp->display_id, i);
  }

  for (i = 0; i < 256; i += 8) {
    printf("%d:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i,
	   keysym_to_keycode_table[i],
	   keysym_to_keycode_table[i + 1],
	   keysym_to_keycode_table[i + 2],
	   keysym_to_keycode_table[i + 3],
	   keysym_to_keycode_table[i + 4],
	   keysym_to_keycode_table[i + 5],
	   keysym_to_keycode_table[i + 6],
	   keysym_to_keycode_table[i + 7]);
  }

#endif /* DEBUG */




  
  return (table);
}

#endif /* XWINDOW */

/************************************************************************/
/*									*/
/*			  k e y b o a r d t y p e			*/
/*									*/
/*	Determine what kind of keyboard we're dealing with, by		*/
/*	checking the LDEKBDTYPE shell variable.  It it's not set,	*/
/*	either default it (for Sun, IBM), or complain and exit.		*/
/*	Valid LDEKBDTYPE values:					*/
/*		type3	Sun type-3 keyboard				*/
/*		type4	Sun type-4 keyboard				*/
/*		rs6000	IBM RS/6000					*/
/*		x	generic X keyboard map				*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void keyboardtype(int fd)
{
  int type;
  int i;
  char *key;

  for (i = 0; i < 5000; i++) { /* IDLE LOOP */
  }                            /* because of a SunOS bug */

  /* clear the keyboard field in devconfig */
  InterfacePage->devconfig &= 0xfff8;

  /************************************************************
   Due to the problems of SunOS 4.0 & 4.0.1
   calling ioctl never return the correct keyboard type.
   So,these 2 lines are commented out ...(Take)->AR11100
  *************************************************************/

  /* Get keytype from LDEKBDTYPE  */
  if ((key = getenv("LDEKBDTYPE")) == 0) {
#ifdef XWINDOW
    type = KB_X;
#elif DOS
    type = KB_DOS;
#elif SUNDISPLAY
    if (ioctl(fd, KIOCTYPE, &type) != 0) {
      error("keyboardtype:IOCTL(KIOCTYPE) fails (cont. w. type-3");
      type = KB_SUN3;
    } /* otherwise, type is set */
#endif /* XWINDOW */
  } /* if end */
  else {
    if (strcmp("as3000j", key) == 0)
      type = KB_AS3000J;
    else if (strcmp("type4", key) == 0)
      type = KB_SUN4;
    else if (strcmp("type2", key) == 0)
      type = KB_SUN2;
    else if (strcmp("jle", key) == 0)
      type = KB_JLE;
    else if (strcmp("X", key) == 0)
      type = KB_X;
    else if (strcmp("x", key) == 0)
      type = KB_X;
    else
      type = KB_SUN3; /* default */
  }

  switch (type) {
    case KB_SUN2: /* type2, we still use keymap for type3 */
      SUNLispKeyMap = SUNLispKeyMap_for3;
      /* MIN_KEYTYPE is 3,so we can't set devconfig correctly */
      /* Therefore type2 may treat as type3 */
      InterfacePage->devconfig |= 0;
      break;
    case KB_SUN3: /* type3 */
      SUNLispKeyMap = SUNLispKeyMap_for3;
      InterfacePage->devconfig |= type - MIN_KEYTYPE;
      break;
    case KB_SUN4: /* type4 */
      SUNLispKeyMap = SUNLispKeyMap_for4;
      InterfacePage->devconfig |= type - MIN_KEYTYPE;
      break;
    case KB_JLE: /* JLE */
      /*printf("jle\n"); */
      SUNLispKeyMap = SUNLispKeyMap_jle;
      InterfacePage->devconfig |= type - MIN_KEYTYPE;
      /* InterfacePage->devconfig |= 4-MIN_KEYTYPE; */
      break;
    case KB_AS3000J: /* for AS, use type4 map */
      SUNLispKeyMap = SUNLispKeyMap_for4;
      InterfacePage->devconfig |= type - MIN_KEYTYPE; /* 7 */
      break;
#ifdef XWINDOW
    case KB_X:
      XGenericKeyMap = (u_char *)make_X_keymap();
      SUNLispKeyMap = XGenericKeyMap;
      InterfacePage->devconfig |= KB_SUN3 - MIN_KEYTYPE; /* 10 */
      break;
#endif /* XWINDOW */

#ifdef DOS
    case KB_DOS:
      SUNLispKeyMap = DOSLispKeyMap_101;
      InterfacePage->devconfig |= KB_SUN3 - MIN_KEYTYPE; /* 10 */
      break;
#endif /* DOS */
    default: {
      char errmsg[200];
      sprintf(errmsg, "Unsupported keyboard type: %d", type);
      printf("%s\n", errmsg);
      printf("Configuring keyboard for type-3\n");
      SUNLispKeyMap = SUNLispKeyMap_for3;
      InterfacePage->devconfig |= KB_SUN3 - MIN_KEYTYPE;
      break;
    }
  }

} /* end keyboardtype*/
