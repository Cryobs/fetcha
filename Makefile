# Makefile for fetcha


VERSION = 1.0.0

CC = cc
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -DVERSION=\"${VERSION}\"
CFLAGS = -std=c99 -pedantic -Wall -Wno-deprecated-declarations -Os -MMD ${CPPFLAGS}
LDFLAGS = -lX11

SRC = fetcha.c modules.c
OBJ = ${SRC:.c=.o}

PREFIX = /usr/local
DESTDIR = 

MANPREFIX = ${PREFIX}/share/man

all: fetcha

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h

config.h:
	cp config.def.h $@

fetcha: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f fetcha ${OBJ}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	@cp -f fetcha "$(DESTDIR)$(PREFIX)/bin/fetcha"
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

.PHONY: all clean install uninstall
