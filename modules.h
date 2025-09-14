#ifndef MODULES_H
#define MODULES_H


typedef  char*(*info_func_t)(void);

typedef struct {
  const char *label;
  info_func_t func;
} info_item;



char *get_os(void);
char *get_host(void);
char *get_kernel(void);
char *get_uptime(void);
char *get_memory(void);
char *get_cpus(void);
char *get_gpus(void);
char *get_wm(void);
char *get_shell(void);
char *get_terminal(void);


#endif
