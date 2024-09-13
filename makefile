.PHONY: clean all install

all: reb

clean:
	rm reb

install: reb
	$(INSTALL) reb $(DESTDIR)/
