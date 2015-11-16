BASEDIR = ../
include $(BASEDIR)Make.env

CFLAGS += -fPIC -shared

SOURCES = $(wildcard *.c)
OBJS	= $(patsubst %.c, %.o, $(SOURCES))
DEPEND	= .depend

all: libreef.so

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ > $@

libreef.so:$(OBJS) $(DEPEND)
	@echo "CFLAGS="$(CFLAGS);
	@$(CC) $(CFLAGS) -o $@ $(OBJS)

include $(DEPEND)
%.o:%.c
	@echo "$(CC) -c $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) *.so

install:
	install -m 0644 libreef.so /usr/local/lib
#TODO headers?

uninstall:
	rm -f /usr/local/lib/libreef.so
