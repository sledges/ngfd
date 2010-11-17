EXTRA_DIST = $(dox_DATA)
doxdir = $(datadir)/doc/ngfd-plugin/html
dox_DATA = html/*

html/*: doxygen.cfg
	doxygen $<

clean-local:
	-rm -rf html
