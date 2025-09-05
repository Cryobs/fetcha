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
#include <pwd.h>
#include <sys/utsname.h>

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
 * function that prints information by template
 * label: value
 * label with color 1
 */
void 
print_info(info_item *item, int maxlen)
{
  char *value;
  value = item->func();
  if (!value) value = "(null)";
  if (info_align == 1) {
    printf("\x1b[%dm%-*s\x1b[0m%s%s", 
        colors[1], maxlen, item->label, info_sep, value); 
    return;
  }
  printf("\x1b[%dm%s\x1b[0m%s%s", colors[1], item->label, info_sep, value); 
  free(value);
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
print_header()
{
  char *name = getenv("USER");
  char hostname[256];
  if (gethostname(hostname, sizeof(hostname)) != 0) {
    perror("gethostname error");
    return -1; 
  } 

  printf("\x1b[0m"); /* reset color */
  printf("\x1b[%dm%s", colors[1], name);
  printf("\x1b[%dm%s", colors[5], header_sep);
  printf("\x1b[%dm%s", colors[2], hostname);


  return strlen(name) + strlen(header_sep) + strlen(hostname);
}

/*
 * function that prints boundary
 * returns:
 *  boundary  length
 */
int
print_boundary(int len)
{
  printf("\x1b[%dm", colors[5]);
  for(int i = 0; i < len; i++) 
  {
    printf("%s", boundary_char);
  }
  printf("\x1b[0m"); /* reset color */

  return len * strlen(boundary_char);
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
  if(read_file(ascii_dir, &res.art) != 0) 
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
print_fetch(struct ascii *res)
{
  char *p = res->art;
  int   curw = 0;
  char  cur_color = '7';
  int   cur_info = 0;
  int   header_len = 0;
  while(*p || (size_t)cur_info < config_items_len) {
    if (*p) {
      /*  check color  */
      if(*p == '$' && isdigit(*(p + 1))){
        cur_color = *++p;
        printf("\x1b[%dm",  colors[cur_color - '0']);
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
      while(curw != res->width + ascii_pad) {
        putchar(' ');
        curw++;
      }
    }

    /* print header */
    if (header_len == 0 && header_show != 0){
      header_len = print_header();
      if (header_len < 0) {
        fprintf(stderr, "Header Error: %d", header_len);
        exit(EXIT_FAILURE);
      } 
      goto print_fetch_end;
    } else if (header_len > 0) {
      print_boundary(header_len);  
      header_len = -1;
      goto print_fetch_end;
    }


    /*  print info */
    
    printf("\x1b[0m"); /*  reset color */

    if ((size_t)cur_info < config_items_len) {
      int maxlen = strlen(config_items[0].label);
      for (int i = 1; (size_t)i < config_items_len; i++) {
        if (strlen(config_items[i].label) > (size_t)maxlen) 
          maxlen = strlen(config_items[i].label);
      }
      print_info(&config_items[cur_info++], maxlen); 
    }

    /* chill I know what I'm doing */
    print_fetch_end:
      curw = 0;
      printf("\n");
      printf("\x1b[%dm",  colors[cur_color - '0']);
  }
  printf("\x1b[0m"); /*  reset color */
  return 0;
}

int
main(int argc, char *argv[])
{
  struct ascii art = get_ascii();

  print_fetch(&art);

  free_ascii(&art);
  return 0;


}
