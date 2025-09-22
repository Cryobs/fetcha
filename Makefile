# Makefile for fetcha


VERSION = 1.0.0

CC = cc
CCFLAGS = -Wall -Wextra -O2 -MMD
LDLIBS = -lX11
SRCS = fetcha.c modules.c
OBJDIR = bin/obj
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)
BIN = bin/fetcha

PREFIX = /usr/local
DESTDIR = 

MANPREFIX = ${PREFIX}/share/man

.PHONY: all clean install uninstall

config.h:
	cp config.def.h config.h

all: $(BIN)

$(BIN): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJDIR) $(BIN)


install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	@cp -f "$(BIN)" "$(DESTDIR)$(PREFIX)/bin/fetcha"
	@chmod 755 "$(DESTDIR)$(PREFIX)/bin/fetcha"
	@echo installing manual page to ${DESTDIR}${MANPREFIX}
	# docs/fetcha.1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < docs/fetcha.1 > ${DESTDIR}${MANPREFIX}/man1/fetcha.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/fetcha.1

	# docs/*.5
	@mkdir -p ${DESTDIR}${MANPREFIX}/man5
	@for file in docs/*.5; do \
		destfile=${DESTDIR}${MANPREFIX}/man5/`basename $$file`; \
		sed "s/VERSION/${VERSION}/g" < $$file > $$destfile; \
		chmod 644 $$destfile; \
	done

uninstall:
	@echo uninstalling executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f "$(DESTDIR)$(PREFIX)/bin/fetcha"

	@echo uninstalling manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f "${DESTDIR}${MANPREFIX}/man1/fetcha.1"

	@echo uninstalling manual pages from ${DESTDIR}${MANPREFIX}/man5
	@for file in docs/*.5; do \
		rm -f "${DESTDIR}${MANPREFIX}/man5/`basename $$file`"; \
	done
