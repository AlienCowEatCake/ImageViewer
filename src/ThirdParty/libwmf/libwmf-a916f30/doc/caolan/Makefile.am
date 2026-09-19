SUBDIRS = . pics

wmfonedocdir = @WMF_DOCDIR@/caolan

install-data-local:
	@echo "Installing Caolan's documentation"; \
	 cd $(srcdir) && \
	 $(mkinstalldirs) $(DESTDIR)$(wmfonedocdir); \
	 (for i in *.html *.gif; do \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(wmfonedocdir)/$$i; \
	  done)

DOCUMENTATION = \
	2.html \
	Arc.html \
	BitBlt.html \
	Bmpfrmat.html \
	Chord.html \
	CreateBrushIndirect.html \
	CreatePenIndirect.html \
	CreateRegion.html \
	DeleteObject.html \
	DibBitBlt.html \
	DibCreatePatternBrush.html \
	DibStretchBlt.html \
	Ellipse.html \
	ExcludeClipRect.html \
	ExtFloodFill.html \
	ExtTextOut.html \
	FillRgn.html \
	FloodFill.html \
	FrameRgn.html \
	IntersectClipRect.html \
	InvertRgn.html \
	LineTo.html \
	MoveTo.html \
	OffsetClipRgn.html \
	OffsetViewportOrg.html \
	OffsetWindowOrg.html \
	PaintRgn.html \
	PatBlt.html \
	Pie.html \
	PolyPolygon.html \
	Polygon.html \
	Polyline.html \
	Rectangle.html \
	RestoreDC.html \
	RoundRect.html \
	SaveDC.html \
	ScaleViewportExt.html \
	ScaleWindowExt.html \
	SelectClipRgn.html \
	SelectObject.html \
	SetBkColor.html \
	SetBkMode.html \
	SetDIBitsToDevice.html \
	SetMapMode.html \
	SetMapperFlags.html \
	SetPixel.html \
	SetPolyFillMode.html \
	SetROP2.html \
	SetStretchBltMode.html \
	SetTextAlign.html \
	SetTextCharacterExtra.html \
	SetTextColor.html \
	SetTextJustification.html \
	SetViewportExt.html \
	SetViewportOrg.html \
	SetWindowExt.html \
	SetWindowOrg.html \
	StretchDib.html \
	TextOut.html \
	dibs.html \
	esc.html \
	index.html \
	mapmode.html \
	ora-clp.html \
	ora-wmf.html \
	rop.html \
	support.html \
	support2.html \
	template.html \
	unknown.html \
	msmeta01.gif \
	msmeta02.gif \
	msmeta03.gif \
	msmeta04.gif

EXTRA_DIST = $(DOCUMENTATION)
