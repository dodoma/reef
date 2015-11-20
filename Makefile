BASEDIR = ./
include $(BASEDIR)Make.env

SUBDIR = include src test

all: $(SUBDIR)
	@$(MULTIMAKE) $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

install:
	@$(MULTIMAKE) -m install $(SUBDIR)

uninstall:
	@$(MULTIMAKE) -m uninstall $(SUBDIR)
