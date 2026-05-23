SUBDIRS = . caolan html

wmfdocdir = @WMF_DOCDIR@

install-data-local:
	@echo "Installing libwmf2 documentation"; \
	 cd $(srcdir) && \
	 $(mkinstalldirs) $(DESTDIR)$(wmfdocdir); \
	 (for i in smlogo2.gif wmfpic.png *.html; do \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(wmfdocdir)/$$i; \
	  done)

DOCUMENTATION = \
	gd.html \
	index.html \
	tutapi.html \
	tutipa.html \
	dlayer.html \
	wmfpic.png \
	smlogo2.gif

EXTRA_DIST = $(DOCUMENTATION)
