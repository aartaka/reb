.PHONY: clean indent all install

all: reb

indent: reb.c
	-indent --k-and-r-style --indent-level8 --tab-size8 --procnames-start-lines $^

clean:
	-rm reb

install: reb
	$(INSTALL) reb $(DESTDIR)$(PREFIX)/bin
