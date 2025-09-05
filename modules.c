#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <stdlib.h>

/* void function placeholder */
typedef char *(*info_func_t)(void);

/* info */
typedef struct 
{
  const char *label;
  info_func_t func;
} info_item;


char *
get_os(void)
{
  FILE *f = fopen("/etc/os-release", "r");
  if (!f) f = fopen("/usr/lib/os-release", "r");
  char osname[128] = "Unknown";
  if (f) {
    char line[256];
    while (fgets(line, sizeof(line), f)) {
      if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
        char *val = line + 12;
        val[strcspn(val, "\n")] = 0;
        if (*val == '"' && val[strlen(val)-1] == '"') {
          val[strlen(val)-1] = 0;
          val++;
        }
        strncpy(osname, val, sizeof(osname)-1);
        break;
      }
    }
    fclose(f);
  }

  /* uname for arch */
  struct utsname buf;
  if (uname(&buf) != 0) {
    return NULL;
  }


  size_t len = strlen(osname) + strlen(buf.machine) + 2;
  char *out = malloc(len);
  if (!*out) return NULL;

  snprintf(out, len, "%s %s", osname, buf.machine);

  return out;
}


