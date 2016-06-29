BASEDIR = ../
include $(BASEDIR)Make.env

SOURCES = $(wildcard *.c)
BINARY = $(patsubst %.c, %, $(SOURCES))
DEPEND = .depend

INCS	= $(INCBASE)
LIBS	= $(LIBBASE) -lm -ljpeg

all: preload $(BINARY)

preload:
	@$(MAKE) -C $(BASEDIR)/src

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(INCS) > $@

-include $(DEPEND)
%:%.c
	@if [ "$<" = `ls *.c|awk '{print $1}'|sed -n '1p'` ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(INCS); \
		echo "=== "$(LIBS); fi
	@echo "$(CC) -o $@"
	$(CC) $(CFLAGS) $^ -o $@ $(INCS) $(LIBS)

clean:
	@rm -rf $(BINARY)

install:

uninstall:
