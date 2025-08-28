#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>



#define ASCII_DIR     "config/ascii.txt"
#define ASCII_COLOR   "32"

#define FIRST_COLOR   "37"
#define SECOND_COLOR  "32"

#define USER_HOST_SEP "@"

#define USER          "brace"
#define HOST          "arch"


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


void
print_color(char *s, char *color)
{
  printf("\x1b[%sm%s\x1b[0m", color, s);
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
  FILE* fp = fopen(ASCII_DIR, "r") ;
  if (!fp) {
    perror("ASCII");
    exit(EXIT_FAILURE);
  }
  
  struct stat st;
  if(fstat(fileno(fp), &st) == 0 && S_ISREG(st.st_mode)) {
    size_t sz = (size_t)st.st_size;
    res.art = malloc(sz + 1);
    if (!res.art) 
      fclose(fp);
    
    size_t read = fread(res.art, 1, sz, fp);
    res.art[read] = '\0';
    fclose(fp);
  }
  get_ascii_size(&res);
  return res;
}

int
print_fetch(struct ascii *res)
{
  char *p = res->art;
  int  curw = 0;
  while(*p) {
    if(*p != '\n'){
      putchar(*p);
      p++;
      curw++;
      continue;
    }
    while(curw != res->width) {
      putchar(' ');
      curw++;
    }

    /*  print info */

    printf("INFO"); 

    p++;
    curw = 0;
    printf("\n");
  }
  return 0;
}

int
main(int argc, char *argv[])
{
  struct ascii art = get_ascii();
  //puts(art.art); 
  
  print_fetch(&art);

  free_ascii(&art);
  return 0;


}
