#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <unistd.h>

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
  if (!f) {
    return strdup("unknown");
  }
  char buf[256];
  if (!fgets(buf, sizeof buf, f)) {
    fclose(f);
    return strdup("unknown");
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
    return strdup("unknown");
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
    return strdup("unknown");
  }
  if (!version) version = strdup("");
  size_t len = strlen(product ? product : "") + 1 + strlen(version) + 1 + 8;
  char *out = malloc(len);
  if (!out) {
    free(product);
    free(version);
    return strdup("unknown");
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
    return strdup("unknown");
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
 * - if years/months/weeks/days/hours/mins == 0: dont add
 *
 * return malloc string
 */
char *
format_uptime(int years, int months, int weeks, int days, int hours, int mins)
{
	char *buf = malloc(128);
	int first = 1;

	if (!buf)
		return strdup("unknown");

	buf[0] = '\0';
	append_part(buf, 128, years, "year", "years", &first);
	append_part(buf, 128, months, "month", "months", &first);
	append_part(buf, 128, weeks, "week", "weeks", &first);
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
	double seconds;
	long long total_seconds;
	long long total_days;
	int years, months, weeks, days, hours, mins;

	if (!f)
		return strdup("unknown");

	if (fscanf(f, "%lf", &seconds) != 1) {
		fclose(f);
		return strdup("unknown");
	}
	fclose(f);

	total_seconds = (long long)seconds;
	mins = (int)((total_seconds / 60) % 60);
	hours = (int)((total_seconds / 3600) % 24);
	total_days = total_seconds / 86400;

	years = (int)(total_days / 365);
	total_days %= 365;
	months = (int)(total_days / 30);
	total_days %= 30;
	weeks = (int)(total_days / 7);
	days = (int)(total_days % 7);

	return format_uptime(years, months, weeks, days, hours, mins);
}


char *
get_memory(void) {
  char *buf = malloc(64);
  FILE *f = fopen("/proc/meminfo", "r");
  if (!f) {
    return strdup("unknown");
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
  if (!f) {
    return strdup("unknown");
  }

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
  if (!buffer) {
    return strdup("unknown");
  }

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

    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s (%d) @ %.2f GHz", 
        cpus[i].model, cpus[i].cores + 1, max_ghz);

    if(buffer[0] != '\0')  strcat(buffer, "\n");
    strcat(buffer, tmp);
  }

  return buffer;
}


char *
get_gpus(void)
{
  FILE *f = popen("lspci | grep -i VGA", "r");
  typedef struct {
    char brand[64];
    char model[128];
  } gpu_info;

  gpu_info gpus[64] = {0};
  char line[256];
  int gpu_num = 0;

  if (!f) {
    return strdup("unknown");
  }

  char *buffer = malloc(4096);
  if (!buffer) return NULL;
  buffer[0] = '\0';

  while (fgets(line, sizeof(line),  f)) {
    /* search for a brand */
    char *brand = strstr(line, "VGA compatible controller: ");
    if (!brand) continue;
    brand += strlen("VGA compatible controller: ");

    /* search brand type */
    if (strstr(brand, "NVIDIA")) {
      strcpy(gpus[gpu_num].brand, "NVIDIA");
    } else if (strstr(brand, "Intel")) {
      strcpy(gpus[gpu_num].brand, "Intel");
    } else if (strstr(brand, "AMD") || strstr(brand, "ATI")) {
      strcpy(gpus[gpu_num].brand, "AMD");
    } else { 
      strcpy(brand, "Unknown");
    }

    char *model = strchr(brand, '[');
    char *model_end = strchr(brand, ']');
    if (model && model_end && model_end > model) {
      int len = model_end - model - 1;
      if (len > 127) len = 127;
      strncpy(gpus[gpu_num].model, model + 1, len);
      model[len] =  '\0';
    } else {
      strncpy(gpus[gpu_num].model, brand, sizeof(gpus[0].model) - 1);
    }

    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s %s\n", 
        gpus[gpu_num].brand, gpus[gpu_num].model);
    strcat(buffer, tmp);
    gpu_num++;
  }

  pclose(f);
  return buffer;
}




char *
get_wm(void)
{
  Display *dpy = XOpenDisplay(NULL);
  if (!dpy) {
    return strdup("unknown");
  }

  Atom wm_check = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", True);
  Atom wm_name = XInternAtom(dpy, "_NET_WM_NAME", True);
  if (wm_check == None || wm_name == None) {
    return strdup("unknown");
  }

  Window root = DefaultRootWindow(dpy);
  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_after;
  unsigned char *prop = NULL;

  /* get window that have "supporting WM check" property */
  if (XGetWindowProperty(dpy, root, wm_check, 0, 1, False, XA_WINDOW, 
                          &actual_type, &actual_format, &nitems, &bytes_after,
                          &prop) != Success || !prop) {
    return strdup("unknown");
  }

  Window wm_window = *(Window*)prop;
  XFree(prop);

  /* read WM name */
  if (XGetWindowProperty(dpy, wm_window, wm_name, 0, (~0L), False,
                       AnyPropertyType, &actual_type, &actual_format,
                       &nitems, &bytes_after, &prop) != Success || !prop) {
    return strdup("unknown");
  }

  char *name = strndup((char *)prop, nitems);
  XFree(prop);

  return name;
}

char *
get_shell(void)
{
  char *shell = getenv("SHELL");
  if (!shell) {
    return strdup("unknown");
  }

  char cmd[256];
  snprintf(cmd, sizeof(cmd), "%s --version 2>/dev/null", shell);

  FILE *f =  popen(cmd, "r");
  if (!f) {
    return strdup(shell);
  }

  char buf[256];
  if (!fgets(buf, sizeof(buf), f)) {
    pclose(f);
    return strdup(shell);
  }
  pclose(f);

  buf[strcspn(buf, "\n")] = 0; /* without \n */

  /* parse: name version */
  char *name = strtok(buf, " ,");
  char *ver  = strtok(NULL, " ,");

  char out[128];
  if (name && ver) {
    snprintf(out, sizeof(out), "%s %s", name, ver);
  } else {
    snprintf(out, sizeof(out), "%s", shell);
  }

  return strdup(out);

}

pid_t
get_parent_pid(pid_t pid)
{
  char path[64];
  snprintf(path, sizeof(path), "/proc/%d/stat", pid);

  FILE *f = fopen(path, "r");
  if (!f) {
    return 0;
  }

  pid_t ppid = 0;
  fscanf(f, "%*d %*s %*c %d", &ppid);
  fclose(f);
  return ppid;

}

char *
get_terminal(void)
{
    char *term = getenv("TERMINAL");
    if (term && *term) { 
      return strdup(term);
    }

    term = getenv("TERM_PROGRAM");
    if (term && *term) {
      return strdup(term);
    }

    term = getenv("TERM");
    if (term && *term) {
      return strdup(term);
    }

    return strdup("unknown");
}

char *
get_editor(void)
{
  char *editor = getenv("EDITOR");
  if (editor && *editor) {
    return strdup(editor);
  }

  return strdup("unknown");
}
