#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>





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

} config;

/* cfg init */
config cfg = {
  .ascii_dir = NULL,
  .ascii_pad = 2,
};

cfg_entry cfgmap[] = {
  { "ascii_dir", CFG_STRING, &cfg.ascii_dir },
  { "ascii_pad", CFG_INT,    &cfg.ascii_pad },
};
int cfgmap_len = sizeof(cfgmap) / sizeof(cfgmap[0]);

void
free_config(config *cfg)
{
  if (!cfg) return;
  free(cfg->ascii_dir);
  cfg->ascii_dir = NULL;
}

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


int
read_file(const char *file_path, char **buf)
{
  if (!file_path || !buf) return -EINVAL; *buf = NULL;
  
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


struct ascii
get_ascii()
{
  struct ascii res;
  read_file(cfg.ascii_dir, &res.art);

  get_ascii_size(&res);
  return res;
}

int
print_fetch(struct ascii *res)
{
  char *p = res->art;
  int  curw = 0;
  char  cur_color = '7';
  while(*p) {
    /*  check color  */
    if(*p == '$' && isdigit(*(p + 1))){
      cur_color = *++p;
      printf("\x1b[3%cm",  cur_color);
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

    /* add padding */
    while(curw != res->width) {
      putchar(' ');
      curw++;
    }

    /*  print info */
    
    printf("\x1b[0m"); /*  reset color */
    for (int i = 0; i < cfg.ascii_pad; i++) {
      putchar(' ');
    }
    printf("cur_color: %c", cur_color); 

    p++;
    curw = 0;
    printf("\n");
    printf("\x1b[3%cm",  cur_color);
  }
  printf("\x1b[0m"); /*  reset color */
  return 0;
}

int
main(int argc, char *argv[])
{
  if(load_config("config/config.conf", &cfg) == 0) {
    printf("ASCII_DIR: %s\n", cfg.ascii_dir);
  }

  struct ascii art = get_ascii();
  //puts(art.art); 
  
  print_fetch(&art);

  

  //free_ascii(&art);
  return 0;


}
