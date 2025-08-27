#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>

#define ASCII_DIR "config/ascii.txt"

int
print_ascii()
{
  FILE* fp = fopen(ASCII_DIR, "r") ;
  if (!fp) 
    return -1;
  
  struct stat st;
  if(fstat(fileno(fp), &st) == 0 && S_ISREG(st.st_mode)) {
    size_t sz = (size_t)st.st_size;
    char *buf = malloc(sz + 1);
    if (!buf) 
      fclose(fp);
    
    size_t read = fread(buf, 1, sz, fp);
    buf[read] = '\0';
    fclose(fp);
    puts(buf);
    free(buf);
    return 0;
  }
  return 1;



}


int
main(int argc, char *argv[])
{
  print_ascii(); 

  return 0;


}
