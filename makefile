all:
	$(MAKE) nasm-segelf
	$(MAKE) -C fdpp

clean install uninstall tar:
	$(MAKE) -C fdpp $@

.PHONY: tar deb rpm nasm-segelf install uninstall

rpm: fdpp.spec.rpkg
	git clean -fd
	rpkg local

deb:
	debuild -i -us -uc -b

nasm-segelf:
	$@ --version 2>/dev/null || \
		brew install $(srcdir)/nasm-segelf.rb || \
		(echo "nasm-segelf is not installed" ; false)
