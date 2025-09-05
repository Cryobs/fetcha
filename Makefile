# Makefile for fetcha

CC = gcc
CFLAGS = -Wall -Wextra -O2 -MMD
SRCS = fetcha.c modules.c
OBJDIR = bin/obj
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)
BIN = bin/fetcha

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJDIR) $(BIN)

