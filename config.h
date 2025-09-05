#include "modules.h"
#include <stdio.h>


static const char *ascii_dir     = "arts/ascii.txt"; 
static const int ascii_pad       = 0; /* padding ascii/info */
static const int info_align      = 1; /* align info by separator */

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
  { "OS", get_os },

};

const size_t config_items_len = sizeof config_items / sizeof config_items[0];
