BASEDIR = ../
include $(BASEDIR)Make.env

#CFLAGS += -fPIC
#LDFLAGS += -shared

SOURCES = $(wildcard *.c)
SOURCES := $(filter-out _mregexp_util.c, $(SOURCES))
SOURCES := $(filter-out _mregexp_tok.c, $(SOURCES))
SOURCES := $(filter-out _mregexp_instruct.c, $(SOURCES))
SOURCES := $(filter-out _http_tls.c, $(SOURCES))
SOURCES := $(filter-out _http_parse.c, $(SOURCES))
SOURCES := $(filter-out _cgi_parse.c, $(SOURCES))
SOURCES := $(filter-out _rfc2388.c, $(SOURCES))
OBJS	= $(patsubst %.c, %.o, $(SOURCES))
DEPEND	= .depend

INCS	= $(INCBASE)

#all: libreef.so
all: libreef.a

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) $(INCS) -MM $^ > $@

include $(DEPEND)
%.o:%.c
	@echo "$(CC) -c $<"
	@$(CC) $(CFLAGS) $(INCS) -c $< -o $@

libreef.a:$(OBJS) $(DEPEND)
	@echo "CFLAGS="$(CFLAGS);
	@rm -rf $@
	$(AR) rcs $@ $(OBJS)

libreef.so:$(OBJS) $(DEPEND)
	@echo "CFLAGS="$(CFLAGS);
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(OBJS) *.so *.a

install:
	install -m 0644 libreef.so /usr/local/lib

uninstall:
	rm -f /usr/local/lib/libreef.so
