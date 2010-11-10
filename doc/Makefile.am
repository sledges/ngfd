all: doxygen.cfg
	doxygen $<

install-data-local:
	$(INSTALL_DATA) -d html $(datadir)/doc/ngfd-plugin/html
	$(INSTALL_DATA) html/* $(datadir)/doc/ngfd-plugin/html/

clean-local:
	-rm -rf html