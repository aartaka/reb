.PHONY: clean all install

all: reb

clean:
	rm reb

install: sade
	$(INSTALL) sade $(DESTDIR)/
