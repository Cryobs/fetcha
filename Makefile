# Makefile for fetcha

CC = gcc
CFLAGS = -Wall -Wextra -O2 -MMD
SRCS = fetcha.c modules.c
OBJDIR = bin/obj
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)
BIN = bin/fetcha

PREFIX = /usr/local
DESTDIR = 

.PHONY: all clean install uninstall

config.h:
	cp config.def.h config.h

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


install: all
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	cp -f "$(BIN)" "$(DESTDIR)$(PREFIX)/bin/fetcha"
	chmod 755 "$(DESTDIR)$(PREFIX)/bin/fetcha"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/fetcha"
