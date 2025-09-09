#include "modules.h"
#include <stdio.h>


static const char *ascii_dir          = "arts/ascii.txt"; 
static const int ascii_pad            = 10; /* padding ascii/info */
static const int info_align           = 1; /* align info by separator */
static const int header_show          = 1; /* if 0 doot print header */
static const int color_palette_show   = 1;
static const char *info_sep           = ": ";
/* 
 * colors ANSI 
 * NAME  : Normal Light
 * Black : 30     (90)
 * Red   : 31     (91)
 * Green : 32     (92)
 * Yellow: 33     (93)
 * Blue  : 34     (94)
 * Purple: 35     (95)
 * Aqua  : 36     (96)
 * White : 37     (97)
 */
static const int colors[10]      = {30, 31, 32, 33, 34, 35, 36, 37, 90, 91};

/* 
 * character header/info
 * if 1 char: boundary len == header len
 * if 2 char: boundary len == 2 * header len 
 */
static const char *boundary_char = "=";

/* 
 * separator for header info 
 */
static const char *header_sep    = "@";

/*
 * information
 * Label, func
 */
static info_item config_items[] = {
  { "OS",     get_os },
  { "HOST",   get_host },
  { "Kernel", get_kernel },
  { "Uptime", get_uptime },

};

const size_t config_items_len = sizeof config_items / sizeof config_items[0];


static const char *ascii_art  =
"                   $1-` \n"
"                  .o+`\n"
"                 `ooo/\n"
"                `+oooo:\n"
"               `+oooooo:\n"
"               -+oooooo+:\n"
"             `/:-:++oooo+:\n"
"            $3`/++++/+++++++:\n"
"           `/++++++++++++++:\n"
"          `/+++$1ooooooooooooo/`\n"
"         ./ooosssso++osssssso+`\n"
"        .oossssso-````/ossssss+`\n"
"       -osssssso.      :ssssssso.\n"
"      :osssssss/        osssso+++. \n"
"     /ossssssss/        +ssssooo/-  \n"
"   `/ossssso+/:-        -:/+osssso+-   \n"
"  `+sso+:-`                 `.-/+oso:     \n"
" `++:.                           `-/+/   \n"
" .`                                 `/  \n"
;

