# tomato-lock
# See LICENSE file for copyright and license details.

include config.mk

SRC = tomato-lock.c
OBJ = ${SRC:.c=.o}

all: options tomato-lock

options:
	@echo tomato-lock build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

tomato-lock: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f tomato-lock ${OBJ} tomato-lock-${VERSION}.tar.gz config.h

dist: clean
	@echo creating dist tarball
	@mkdir -p tomato-lock-${VERSION}
	@cp -R LICENSE Makefile README config.def.h config.mk ${SRC} tomato-lock-${VERSION}
	@tar -cf tomato-lock-${VERSION}.tar tomato-lock-${VERSION}
	@gzip tomato-lock-${VERSION}.tar
	@rm -rf tomato-lock-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f tomato-lock ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/tomato-lock
	@chmod u+s ${DESTDIR}${PREFIX}/bin/tomato-lock

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/tomato-lock

.PHONY: all options clean dist install uninstall
