/* 
 * Copyright (C) 2025 cryobs
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define  COLORS 10



struct ascii 
{
  char *art; 
  int width;
  int height;
};

void
free_ascii(struct ascii *s)
{
  if (!s) return;
  free(s->art);
  s->art = NULL;
}

/* info */
typedef struct 
{
  const char *label;
  const char *value;
} info;



/* config */
typedef enum { CFG_STRING, CFG_INT } cfg_type;

typedef struct 
{
  const char *key;
  cfg_type type;
  void *ptr;
} cfg_entry;
  

typedef struct 
{
  char *ascii_dir; /* ascii directory */
  int   ascii_pad; /* ascii/info padding in spaces */ 
  int   info_align; /* info aligment 1 yes, 0 no */

  char *colors[COLORS];

} config;

/*
 * color to ansi code
 * function that takes color [0-15] and convert it to:
 * - [0-7]  - [30-37] default colors
 * - [8-15] - [90-97] light colors
 * and writes to out buffer
 */
void
c_to_ansi(const char *color, char *out, size_t out_size)
{

  int n = atoi(color);
  int ansi_code = 39;

  if (n >= 0 && n <= 7) {
    ansi_code = n + 30;
  } else if (n > 7 && n <= 15) {
    ansi_code = n + 90 - 8;
  }

  snprintf(out, out_size, "%d", ansi_code);
}

/* cfg init */
config cfg;

void
config_init()
{
  cfg.ascii_dir = strdup("config/ascii.txt"); 
  char buf[8];
  char *colors[COLORS] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};

  for (int i = 0; i < COLORS; i++) {
    c_to_ansi(colors[i], buf, sizeof(buf));
    cfg.colors[i] = strdup(buf);
  }
  
  cfg.ascii_pad = 2;
  cfg.info_align = 1;
}

cfg_entry cfgmap[] = {
  { "ascii_dir",  CFG_STRING,&cfg.ascii_dir },
  { "ascii_pad",  CFG_INT,   &cfg.ascii_pad },
  { "info_align", CFG_INT,   &cfg.info_align},
  { "color0",     CFG_STRING,&cfg.colors[0] },
  { "color1",     CFG_STRING,&cfg.colors[1] },
  { "color2",     CFG_STRING,&cfg.colors[2] },
  { "color3",     CFG_STRING,&cfg.colors[3] },
  { "color4",     CFG_STRING,&cfg.colors[4] },
  { "color5",     CFG_STRING,&cfg.colors[5] },
  { "color6",     CFG_STRING,&cfg.colors[6] },
  { "color7",     CFG_STRING,&cfg.colors[7] },
  { "color8",     CFG_STRING,&cfg.colors[8] },
  { "color9",     CFG_STRING,&cfg.colors[9] },
};
int cfgmap_len = sizeof(cfgmap) / sizeof(cfgmap[0]);

void
free_config(config *cfg)
{
  if (!cfg) return;
  free(cfg->ascii_dir);
  cfg->ascii_dir = NULL;
  if (cfg->colors) {
    for (int i = 0; i < COLORS; i++) {
      free(cfg->colors[i]);  
    }
  }
}

/*
 * function that prints information by template
 * label: value
 * label with color 1
 */
void 
print_info(info inf, int maxlen)
{
  if (cfg.info_align == 1) {
    printf("\x1b[%sm%-*s\x1b[0m: %s", cfg.colors[1], maxlen, inf.label, inf.value); 
    return;
  }
  printf("\x1b[%sm%s\x1b[0m: %s", cfg.colors[1], inf.label, inf.value); 
}


/* 
 * function that returns dynamic sebstring from *start to *end
 */
char *
substring(const char *start, const char *end)
{
  if (!start || !end || end < start) return NULL;

  size_t len = end - start; 
  char *out = malloc(len + 1); /* +1 for \0 */
  if (!out) return NULL;

  memcpy(out, start, len);
  out[len] = '\0';

  return out;
}


/* function that:
 * - deletes all spaces from start to first char
 * - deletes all spaces from end to first char
 */
char *
trim(char *str) 
{
    char *end;

    /* skip start spaces */
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  /* blank str */
        return str;

    /* skip end spaces */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    /* ending string */
    end[1] = '\0';

    return str;
}


/*
 * simple config processor, that process configs like that:
 * - key = value // comments 
 * checks for keywords from cfgmap, and changing values in cfg
 */
int 
load_config(const char *filename, config *cfg)
{
  FILE *fd = fopen(filename, "r");
  if (!fd) {
    perror("Config open error");
    return -1;
  }

  char line[128];
  while (fgets(line, sizeof(line), fd)) {
      /* skip comments and blank lines */
      if ((line[0] == '/' && line[1] == '/') || line[0] == '\n') {
        continue;
      }

      char *cmt = strstr(line, "//");
      if (cmt) 
        *cmt = '\0';


      char *eq = strchr(line, '=');
      if (!eq) continue;

      *eq = '\0';  /* split line */
      char *key = trim(line);
      char *value = trim(eq + 1);

      /* remove \n */
      value[strcspn(value, "\r\n")] = 0;

      /* check for keywords */
      for (int i = 0; i < cfgmap_len; i++) {
        if (strcmp(key, cfgmap[i].key) == 0) {
          if (cfgmap[i].type == CFG_STRING) {
            free(*(char **)cfgmap[i].ptr);
            if (strncmp(key, "color", 5) == 0) {
                char buf[8];
                c_to_ansi(value, buf, sizeof(buf));
                *(char **)cfgmap[i].ptr = strdup(buf);
                continue;
            }
            *(char **)cfgmap[i].ptr = strdup(value);
          } else if (cfgmap[i].type == CFG_INT) {
            *(int *)cfgmap[i].ptr = atoi(value);
          }
        }
      }
          
  }
  
  fclose(fd);
  return 0;
}

/*
 * function that reads file to buf dynamicly
 * if returns < 0 here errors:
 * - (-1) file doesn't exist
 * - (-2) can't get size of the file
 * - (-3) problem with allocating memory for file content
 * - (-4) problem with reading file content
 * "adds '\0' to and"
 */
int
read_file(const char *file_path, char **buf)
{
  if (!file_path || !buf) {
    *buf = NULL;
    return -EINVAL;
  }
  
  FILE* fp = fopen(file_path, "r") ;
  if (!fp) {
    perror(file_path);
    return -1;
  } 

  struct stat st;
  if (fstat(fileno(fp), &st) != 0 || !S_ISREG(st.st_mode)) {
    fclose(fp);
    return -2;
  }

  size_t sz = (size_t)st.st_size;
  *buf = malloc(sz + 1);
  if(!*buf) {
    fclose(fp);
    perror("malloc");
    return -3;
  }

  size_t read = fread(*buf, 1, sz, fp);
  if (read < sz && ferror(fp)) {
    perror("fread");
    free(*buf);
    fclose(fp);
    return -4;
  }

  (*buf)[read] = '\0';
  fclose(fp);



  return 0;
}

/*
 * function that fill ascii.width and ascii.height fields,
 * by counting the most long line (width),
 * and counting lines (height);
 * "writes to res"
 */
void
get_ascii_size(struct ascii *res)
{
  if (!res || !res->art) {
    res->width = res->height = 0;
    return;
  }

  int maxw = 0;
  int curw = 0;
  int lines = 0;
  char *p = res->art;
  if  (*p == '\0') {
    res->width = res->height = 0;
    return;
  }

  while (*p) {
    if (*p == '\n') {
      lines++;
      if (curw > maxw) maxw = curw;
      curw = 0;
    } else {
      curw++;
    }
    p++;
  }

  /* if last char is not '\n' */
  if (curw == 0 || (res->art[0] != '\0' && res->art[0] == '\n')) {
    lines++;
    if (curw > maxw) maxw = curw;
  }

  res->width = maxw;
  res->height = lines;
}


/*
 * fill all struct ascii fields and return final structure
 */
struct ascii
get_ascii()
{
  struct ascii res = {0};
  if(read_file(cfg.ascii_dir, &res.art) != 0) 
    res.art = strdup("");

  get_ascii_size(&res);
  return res;
}

/*
 * function that print:
 * - ascii art with colors from config, with processing like:
 *     $1 - cfg.colors[1]
 * - information with print_info(), takes size of infos (size_t infos_size)
 *
 * if return < 0:
 * - 
 */
int
print_fetch(struct ascii *res, info *infos, size_t infos_size)
{
  char *p = res->art;
  int   curw = 0;
  char  cur_color = '7';
  int   cur_info = 0;
  while(*p || (size_t)cur_info < infos_size) {
    if (*p) {
      /*  check color  */
      if(*p == '$' && isdigit(*(p + 1))){
        cur_color = *++p;
        printf("\x1b[%sm",  cfg.colors[cur_color - '0']);
        p++;
        continue;
      }
      /* print char */
      if(*p != '\n'){
        putchar(*p);
        p++;
        curw++;
        continue;
      }
      p++;
    }

    /* add padding */
    if (res->width > 0) {
      while(curw != res->width + cfg.ascii_pad) {
        putchar(' ');
        curw++;
      }
    }

    /*  print info */
    
    printf("\x1b[0m"); /*  reset color */

    if ((size_t)cur_info < infos_size) {
      int maxlen = strlen(infos[0].label);
      for (int i = 1; (size_t)i < infos_size; i++) {
        if (strlen(infos[i].label) > (size_t)maxlen) 
          maxlen = strlen(infos[i].label);
      }
      print_info(infos[cur_info++], maxlen); 
    }

    curw = 0;
    printf("\n");
    printf("\x1b[%sm",  cfg.colors[cur_color - '0']);
  }
  printf("\x1b[0m"); /*  reset color */
  return 0;
}

int
main(int argc, char *argv[])
{
  config_init();

  if(load_config("config/config.conf", &cfg) != 0) {
    fprintf(stderr, "ASCII_DIR: %s not exist\n", cfg.ascii_dir);
    exit(EXIT_FAILURE);
  }

  struct ascii art = get_ascii();
  //puts(art.art); 
  info infos[] = {
    {"OS",  "Arch Linux"},
    {"Kernel", "6.10.5-arch1"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
    {"Shell", "zsh"},
  };

  print_fetch(&art, infos, sizeof(infos) / sizeof(infos[0]));

  
  free_config(&cfg);
  free_ascii(&art);
  return 0;


}
