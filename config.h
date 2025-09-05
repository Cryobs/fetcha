#include "modules.h"
#include <stdio.h>

static const char *ascii_dir     = "arts/ascii.txt";
static const int ascii_pad       = 0;
static const int info_align      = 1;
static const int colors[10]      = {30, 31, 32, 33, 34, 35, 36, 37, 90, 91};
static const char *boundary_char = "=";
static const char *header_sep    = "@";


static info_item config_items[] = {
  { "OS", get_os },

};

const size_t config_items_len = sizeof config_items / sizeof config_items[0];
