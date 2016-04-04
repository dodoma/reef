BASEDIR = ../
include $(BASEDIR)Make.env

CFLAGS += -fPIC
LDFLAGS += -shared

SOURCES = $(wildcard *.c)
OBJS	= $(patsubst %.c, %.o, $(SOURCES))
DEPEND	= .depend

INCS	= $(INCBASE)

#all: libreef.so
all: libreef.a

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) $(INCS) -MM $^ > $@

libreef.so:$(OBJS) $(DEPEND)
	@echo "CFLAGS="$(CFLAGS);
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

libreef.a:$(OBJS) $(DEPEND)
	@echo "CFLAGS="$(CFLAGS);
	@rm -rf $@
	$(AR) rcs $@ $(OBJS)

include $(DEPEND)
%.o:%.c
	@echo "$(CC) -c $<"
	@$(CC) $(CFLAGS) -c $< -o $@ $(INCS)

clean:
	rm -f $(OBJS) *.so *.a

install:
	install -m 0644 libreef.so /usr/local/lib

uninstall:
	rm -f /usr/local/lib/libreef.so
