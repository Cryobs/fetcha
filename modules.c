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


static char *
read_file_trim(const char *path)
{
  FILE *f = fopen(path, "r");
  if (!f) return NULL;
  char buf[256];
  if (!fgets(buf, sizeof buf, f)) {
    fclose(f);
    return NULL;
  }
  /* trim newline */
  size_t n = strlen(buf);
  while (n && (buf[n-1] == '\n' || buf[n-1] == '\r')) buf[--n] = '\0';
  return strdup(buf);
}

/* 
 * function that returns malloc string "OS Architecture" or NULL
 */
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


/*
 * function that returns malloc string "Product Version" or NULL
 */
char *
get_host(void)
{
  char *product = read_file_trim("/sys/class/dmi/id/product_name");
  char *version = read_file_trim("/sys/class/dmi/id/product_version");
  if (!product && !version) {
    free(product);
    free(version);
    return NULL;
  }
  if (!version) version = strdup("");
  size_t len = strlen(product ? product : "") + 1 + strlen(version) + 1 + 8;
  char *out = malloc(len);
  if (!out) {
    free(product);
    free(version);
    return NULL;
  }
  if(product && version[0]) 
    snprintf(out, len, "%s %s", product, version);
  else if (product)
    snprintf(out, len, "%s", product);
  else    
    snprintf(out, len, "%s", version);

  free(product);
  free(version);

  return out;
}
