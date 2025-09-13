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


/*
 * function that returns malloc string "Kernel" or NULL
 */
char *
get_kernel(void)
{
  struct utsname buf;
  if (uname(&buf) != 0) {
    return NULL;
  }
  return strdup(buf.release);
}


/*
 * function that concatinates to buf:
 * if val != 0
 * if val == 1: concat singular
 * if val > 1:  concat plural
 */
static void
append_part(char *buf, size_t buf_size, int val, 
            const char *singular, const char *plural, int *first)
{
  if (val <= 0) return;
  char part[32];
  snprintf(part, sizeof part,  "%d %s", val, (val == 1) ? singular : plural);
  if (!*first) 
    strncat(buf, ", ", buf_size - strlen(buf) - 1);
  strncat(buf, part, buf_size - strlen(buf) - 1);
  *first = 0;
}


/*
 * function that formats time:
 * - with plural/singular format
 * - if days/hours/mins == 0: dont add
 *
 * return malloc string
 */
char *
format_uptime(int days, int hours, int mins)
{
  char *buf = malloc(128);
  if (!buf)
    return NULL;
  buf[0] = '\0';
  int first = 1;
  append_part(buf, 128, days, "day", "days", &first);
  append_part(buf, 128, hours, "hour", "hours", &first);
  append_part(buf, 128, mins, "min", "mins", &first);
  if (first)
    snprintf(buf, 128, "0 mins");
  return buf;
}


char *
get_uptime(void)
{
  FILE *f = fopen("/proc/uptime", "r");
  if (!f) return NULL;
  double seconds;
  if (fscanf(f, "%lf", &seconds) != 1) {
    fclose(f);
    return NULL;
  }
  fclose(f);

  int mins = (int)(seconds / 60) % 60;
  int hours = (int)(seconds / 3600) % 24;
  int days = (int) hours / 24;

  return format_uptime(days, hours, mins); 
}


char *
get_memory(void) {
  char *buf = malloc(64);
  FILE *f = fopen("/proc/meminfo", "r");
  if (!f) {
    return NULL;
  }

  long mem_total = 0;
  char *mem_total_type = "KiB";
  long mem_free = 0;
  long buffers = 0;
  long cached = 0;

  char key[32];
  long value;
  char unit[16];

  while(fscanf(f, "%31s %ld %15s", key, &value, unit) == 3) {
    if (strcmp(key, "MemTotal:") == 0) {
      mem_total = value;
    } else if (strcmp(key, "MemFree:") == 0) {
      mem_free = value;
    } else if (strcmp(key, "Buffers:") == 0) {
      buffers = value;
    } else if (strcmp(key, "Cached:") == 0) {
      cached = value;
    }
  }
  fclose(f);

  long mem_used = mem_total - mem_free - buffers - cached;\
  char *mem_used_type = "KiB";

  if (mem_total >= 1024) {
    mem_total /= 1024;
    mem_total_type = "MiB";
  }
  if (mem_used >= 1024) {
    mem_used /= 1024;
    mem_used_type = "MiB";
  }

  snprintf(buf, 64, "%ld%s / %ld%s", 
      mem_used, mem_used_type, mem_total, mem_total_type);
  return buf;
}



char *
get_cpus(void) {
  FILE *f = fopen("/proc/cpuinfo", "r");
  if (!f) return NULL;

  typedef struct {
    char model[128];
    int cores;
    int first_logical;
  } cpu_info;

  cpu_info cpus[32] = {0};
  int cpu_count = 0;

  char line[256];
  int logical_index = 0;
  int current_physical = -1;

  while (fgets(line, sizeof(line), f)) {
    if (strncmp(line, "processor", 9) == 0) {
      logical_index++;
    } else if (strncmp(line, "physical id", 11) == 0) {
      char *p = strchr(line,  ':');
      if (p) current_physical = atoi(p + 1);
      if (current_physical + 1 > cpu_count) cpu_count = current_physical + 1;
    } else if (strncmp(line, "model name", 10) == 0 && current_physical >= 0)  {  
      char *p = strchr(line, ':');
      if (!p) {
        continue;
      }

      p++;
      while(*p == ' ' || *p == '\t') p++;
      char *newline = strchr(p, '\n');
      if (newline) *newline = '\0';
      if (cpus[current_physical].cores == 0) {
        char *cpu_at = strstr(p, " CPU @");
        if (cpu_at) *cpu_at = '\0';
        strncpy(cpus[current_physical].model, p, 
            sizeof(cpus[current_physical].model)-1);
        cpus[current_physical].first_logical = logical_index;
      }
      cpus[current_physical].cores++;
    }
  }
  
  fclose(f);
  
  char *buffer = malloc(4096);
  if (!buffer) return NULL;
  buffer[0] =  '\0';

  for (int i = 0; i < cpu_count; i++) {
    if (cpus[i].cores == 0) continue;

    /* read max freq for first cpu thread */
    char path[128];
    snprintf(path, sizeof(path), 
        "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", 
        cpus[i].first_logical);
    unsigned long max_khz = 0;
    FILE *freq_file = fopen(path, "r");
    if (freq_file) {
      fscanf(freq_file, "%lu", &max_khz);
      fclose(freq_file);
    }
    double max_ghz = max_khz / 1000.0 / 1000.0;

    if (i < 0) strcat(buffer, "\n");
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s (%d) @ %.2f GHz", 
        cpus[i].model, cpus[i].cores + 1, max_ghz);
    strcat(buffer, tmp);
  }

  return buffer;
}
