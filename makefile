.PHONY: clean indent all install

all: reb
CFLAGS ?= -Wall

indent: reb.c
	-indent -linux --procnames-start-lines $^

clean:
	-rm reb

install: reb
	$(INSTALL) reb $(DESTDIR)$(PREFIX)/bin/
