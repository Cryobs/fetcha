#include "modules.h"
#include <stdio.h>


static const int  ascii_pad            = 10; /* padding ascii/info */
static const bool info_align           = true; /* align info by separator */
static const bool header_show          = true; /* if 0 doot print header */
static const bool color_palette_show   = true;
static const char *info_sep            = ": ";
static const bool numerate_same        = true;
static const bool line_break           = true;

/* 
 * colors ANSI 
 * NAME    Normal Light
 * Black : 30     (90)
 * Red   : 31     (91)
 * Green : 32     (92)
 * Yellow: 33     (93)
 * Blue  : 34     (94)
 * Purple: 35     (95)
 * Aqua  : 36     (96)
 * White : 37     (97)
 *
 * colorpallete:
 * [0-4] - secondary colors
 * [5-9] - text colors:
 *   5 - info text
 *   6 - info separator
 *   7 - header separator
 *   8 - boundary
 *   9 - line breaker
 */

static const int colors[10]      = {30, 31, 32, 33, 34, 35, 36, 37, 90, 91};

/* 
 * character header/info
 */
static const char boundary_char = '=';

/* 
 * separator for header info 
 */
static const char *header_sep    = "@";

/* 
 * line breaker character
 */
static const char line_break_char = '>';

/*
 * information
 * Label, func
 */
static info_item config_items[] = {
  { "OS",       get_os },
  { "HOST",     get_host },
  { "Kernel",   get_kernel },
  { "Uptime",   get_uptime },
  { "Memory",   get_memory },
  { "CPU",      get_cpus },
  { "GPU",      get_gpus },
  { "WM",       get_wm },
  { "Shell",    get_shell },
  { "Editor",   get_editor },
  { "Terminal", get_terminal },

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

