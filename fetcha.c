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
#include <unistd.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <stdbool.h>

#include "modules.h"
#include "config.h"

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

/* header */
typedef struct 
{
  char *name;     
  char *sep;      /* separator name[]hostname */
  char hostname[256];

} header;


typedef struct
{
  char *label;
  char *value;

} rendered_info;

typedef struct
{
  rendered_info *entries;
  size_t count;

} info_list;

void 
free_info_list(info_list *infos) {
  if (!infos) return;

  for (size_t i = 0; i < infos->count; i++) {
    free(infos->entries[i].label);
    free(infos->entries[i].value);
  }

  free(infos->entries);
  infos->count = 0;
}

/*
 * prints string with line break (if setted up "line_break")
 * return 0: if no line_break
 * return 1: if line_break
 */
int
puts_limited(const char *s, int term_width, int *curw, bool show_break)
{
  while (*s) {
    if (term_width > 0 && *curw >= term_width - 2 && line_break) {
      if (show_break) 
        printf(" \x1b[%dm%c", colors[9], line_break_char);
      *curw = term_width;
      return 1;
    }
    putchar(*s++);
    (*curw)++;
  }
  return 0;
}

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



/*
 * function that:
 * 1. if (align_info) add padding to label
 * 2. return info_list, with malloc!
 */
info_list
render_info(info_item infos[], size_t info_size)
{
  info_list res = {NULL, 0};
  size_t maxlen = 0;

  if (info_align) {
    for(size_t i = 0; i < info_size; i++) {
      size_t len = strlen(infos[i].label);
      if (len > maxlen) {
        maxlen = len;
      }
    }
  }

  for (size_t i = 0; i < info_size; i++) {
    char *value = infos[i].func();
    if (!value) value = "(null)";

    /* count strings */
    int split_count = 0;
    {
      char *tmp = strdup(value);
      char  *p = strtok(tmp, "\n");
      while (p) {
        split_count++;
        p = strtok(NULL, "\n");
      }
      free(tmp);
    }
    
    /* split strings by \n */
    char *line = strtok(value, "\n");
    int number = 1;
    while (line) {
      res.entries = realloc(res.entries, 
                            sizeof(rendered_info) * (res.count + 1));
      
      char tmp_label[128];
      if (split_count > 1 && numerate_same) {
        snprintf(tmp_label, sizeof(tmp_label), "%s%d", infos[i].label, number);
      } else {
        snprintf(tmp_label, sizeof(tmp_label), "%s", infos[i].label);
      }


      if (info_align) {
        char padded[128];
        snprintf(padded, sizeof(padded), "%-*s", (int)maxlen, tmp_label);
        res.entries[res.count].label = strdup(padded);
      } else {
        res.entries[res.count].label = strdup(tmp_label);
      }

      res.entries[res.count].value = strdup(line);
      res.count++;

      line = strtok(NULL, "\n");
      number++;
    }
    free(value);
  }
  return res; 
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
 * function that prints header
 * returns:
 *  > 0: header length
 *  < 0: error
 */
int
print_header(int term_width, int *curw)
{
  char *name = getenv("USER");
  char hostname[256];
  if (gethostname(hostname, sizeof(hostname)) != 0) {
    perror("gethostname error");
    return -1; 
  } 

  printf("\x1b[0m"); /* reset color */
  printf("\x1b[%dm", colors[1]);
  if(!puts_limited(name, term_width, curw, true)) {
    printf("\x1b[%dm", colors[7]);
    if(!puts_limited(header_sep, term_width, curw, true)) {
      printf("\x1b[%dm", colors[2]);
      puts_limited(hostname, term_width, curw, true);
    }
  }


  return strlen(name) + strlen(header_sep) + strlen(hostname);
}

/*
 * function that prints boundary
 */
void
print_boundary(const char c, int len, int term_width, int *curw)
{
  printf("\x1b[%dm", colors[8]);
  char s[len];
  for(int i = 0; i < len; i++) 
  {
    s[i] = c;
  }
  puts_limited(s, term_width, curw, true);
  printf("\x1b[0m"); /* reset color */
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
  if (*p == '\0') {
    res->width = res->height = 0;
    return;
  }

  while (*p) {
    if (*p == '\n') {
      lines++;
      if (curw > maxw) maxw = curw;
      curw = 0;
    } else if (*p == '$' && isdigit(*(p + 1))) {
      p += 2;
      continue;
    } else {
      curw++;
    }
    p++;
  }

  if (curw > 0) {
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
  res.art = strdup(ascii_art);

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
print_fetch(struct ascii *res)
{
  char        *p = res->art;
  int       curw = 0;
  char cur_color = '7';
  int   cur_info = 0;
  int header_len = 0;
  int term_width = 0;

  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
    term_width = ws.ws_col;

  info_list infos = render_info(config_items, config_items_len);

  while (*p || (size_t)cur_info < infos.count +
        ((color_palette_show == 1) ? 3 : 0)) {
    if (*p) {
      /* check color */
      if (*p == '$' && isdigit(*(p + 1))) {
        cur_color = *++p;
        printf("\x1b[%dm", colors[cur_color - '0']);
        p++;
        continue;
      }
      /* print char */
      if (*p != '\n') {
        if (term_width > 0 && curw >= term_width - 2 && line_break) {
          printf(" \x1b[%dm%c", colors[9], line_break_char);
          while (*p && *p != '\n') p++;
          curw = res->width + ascii_pad;
          continue;
        }
        putchar(*p);
        p++;
        curw++;
        continue;
      }
      p++;
    }

    /* add padding */
    if (res->width > 0) {
      while (curw != res->width + ascii_pad) {
        if (term_width > 0 && curw >= term_width) {
          curw = res->width + ascii_pad;
          break;
        }
        putchar(' ');
        curw++;
      }
    }

    /* skip info/header if no space */
    if (term_width > 0 && curw >= term_width - 2) {
      if (header_len == 0 && header_show != 0)
        header_len = -1;
      else if (header_len > 0)
        header_len = -1;
      else if ((size_t)cur_info < infos.count +
               ((color_palette_show == 1) ? 3 : 0))
        cur_info++;
      goto print_fetch_end;
    }

    /* print header */
    if (header_len == 0 && header_show != 0) {
      header_len = print_header(term_width, &curw);
      if (header_len < 0) {
        fprintf(stderr, "Header Error: %d", header_len);
        exit(EXIT_FAILURE);
      }
      goto print_fetch_end;
    } else if (header_len > 0) {
      print_boundary(boundary_char, header_len, term_width, &curw);
      header_len = -1;
      goto print_fetch_end;
    }

    /* print boundary for palette */
    if ((size_t)cur_info == infos.count && color_palette_show) {
      cur_info++;
      goto print_fetch_end;
    }

    /* print normal palette */
    if ((size_t)cur_info == infos.count + 1 && color_palette_show) {
      for (int i = 0; i < 8; i++) {
        printf("\x1b[4%dm", i);
        if (puts_limited("   ", term_width, &curw, false))
          break;
      }
      cur_info++;
      goto print_fetch_end;
    }

    /* print bright palette */
    if ((size_t)cur_info == infos.count + 2 && color_palette_show) {
      for (int i = 0; i < 8; i++) {
        printf("\x1b[10%dm", i);
        if (puts_limited("   ", term_width, &curw, false))
          break;
      }
      cur_info++;
      goto print_fetch_end;
    }

    /* print info */
    if ((size_t)cur_info < infos.count) {
      printf("\x1b[0m");
      printf("\x1b[%dm", colors[1]);
      if (!puts_limited(infos.entries[cur_info].label, term_width, &curw, true)) {
        printf("\x1b[%dm", colors[6]);
        if (!puts_limited(info_sep, term_width, &curw, true)) {
          printf("\x1b[%dm", colors[5]);
          puts_limited(infos.entries[cur_info].value, term_width, &curw, true);
        }
      }
      cur_info++;
    }

    print_fetch_end:
      curw = 0;
      printf("\x1b[0m");
      printf("\n");
      printf("\x1b[%dm", colors[cur_color - '0']);
  }
  printf("\x1b[0m");

  free_info_list(&infos);
  return 0;
}

int
main(void)
{
  struct ascii art = get_ascii();

  print_fetch(&art);

  free_ascii(&art);
  return 0;


}
