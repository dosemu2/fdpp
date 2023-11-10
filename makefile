-include config.mak
srcdir ?= $(CURDIR)

define mdir
	if [ ! -f $(1)/makefile ]; then \
	    mkdir $(1) 2>/dev/null ; \
	    ln -s $(srcdir)/$(1)/makefile $(1)/makefile ; \
	fi
	cd $(1) && $(MAKE) srcdir=$(srcdir)/$(1) $(2)
endef

all:
	$(MAKE) nasm-segelf
	+$(call mdir,fdpp,all)

clean:
	+$(call mdir,fdpp,clean)

rpm: fdpp.spec.rpkg
	git clean -fd
	rpkg local

install uninstall deb tar:
	cd fdpp && $(MAKE) srcdir=$(srcdir)/fdpp $@

nasm-segelf:
	$@ --version 2>/dev/null || \
		brew install $(srcdir)/nasm-segelf.rb
