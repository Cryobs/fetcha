#ifndef MODULES_H
#define MODULES_H


typedef  char*(*info_func_t)(void);

typedef struct {
  const char *label;
  info_func_t func;
} info_item;

char *get_os(void);

#endif
