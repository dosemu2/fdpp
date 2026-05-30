all:
	$(MAKE) -C fdpp

clean install uninstall tar:
	$(MAKE) -C fdpp $@

.PHONY: tar deb rpm install uninstall

rpm: fdpp.spec.rpkg
	git clean -fd
	rpkg local

deb:
	debuild -i -us -uc -b
