BASEDIR = ../
include $(BASEDIR)Make.env

PREFIX=/usr/local

HEADERS = $(wildcard *.h)

clean:

install:
	install -d $(PREFIX)/include/reef
	install -m 0644 $(HEADERS) $(PREFIX)/include/reef

uninstall:
	cd $(PREFIX)/include/reef && rm -f $(HEADERS) && cd -
