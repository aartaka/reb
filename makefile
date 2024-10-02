.PHONY: clean indent all install

all: reb

indent: reb.c
	-indent -linux $^

clean:
	-rm reb

install: reb
	$(INSTALL) reb $(DESTDIR)$(PREFIX)/bin/
