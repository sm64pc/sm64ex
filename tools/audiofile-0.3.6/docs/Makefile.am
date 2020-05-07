if ENABLE_DOCUMENTATION

DOCS_TXT_MAN1 = \
	sfconvert.1.txt \
	sfinfo.1.txt

DOCS_TXT_MAN3 = \
	afCloseFile.3.txt \
	afGetFrameCount.3.txt \
	afGetFrameSize.3.txt \
	afInitAESChannelDataTo.3.txt \
	afInitCompression.3.txt \
	afInitFileFormat.3.txt \
	afInitSampleFormat.3.txt \
	afNewFileSetup.3.txt \
	afOpenFile.3.txt \
	afQuery.3.txt \
	afReadFrames.3.txt \
	afReadMisc.3.txt \
	afSeekFrame.3.txt \
	afSetErrorHandler.3.txt \
	afSetVirtualSampleFormat.3.txt \
	afWriteFrames.3.txt

DOCS_TXT = $(DOCS_TXT_MAN1) $(DOCS_TXT_MAN3)

EXTRA_DIST = \
	$(DOCS_TXT) \
	asciidoc.conf

DOCS_MAN1 = $(DOCS_TXT_MAN1:.txt=)
DOCS_MAN3 = $(DOCS_TXT_MAN3:.txt=)
DOCS_MAN3_EXTRA = \
	afInitAESChannelData.3 \
	afInitByteOrder.3 \
	afInitChannels.3 \
	afInitRate.3 \
	afGetDataOffset.3 \
	afGetTrackBytes.3 \
	afQueryLong.3 \
	afQueryDouble.3 \
	afQueryPointer.3 \
	afSeekMisc.3 \
	afSetVirtualByteOrder.3 \
	afSetVirtualChannels.3 \
	afSetVirtualPCMMapping.3 \
	afTellFrame.3 \
	afWriteMisc.3

DOCS_HTML = $(DOCS_TXT:.txt=.html)

dist_man1_MANS = $(DOCS_MAN1)
dist_man3_MANS = $(DOCS_MAN3) $(DOCS_MAN3_EXTRA)

ASCIIDOCFLAGS = -arevnumber=$(VERSION) -amansource="Audio File Library"
A2XFLAGS = $(ASCIIDOCFLAGS) -d manpage -f manpage

%.html: %.txt
	$(ASCIIDOC) $(ASCIIDOCFLAGS) -b html4 -d manpage -f asciidoc.conf $<

%.1: %.1.txt
	$(A2X) $(A2XFLAGS) --asciidoc-opts="-f asciidoc.conf" $<

%.3: %.3.txt
	$(A2X) $(A2XFLAGS) --asciidoc-opts="-f asciidoc.conf" $<

html: $(DOCS_HTML)

CLEANFILES = *.1 *.3 *.html

endif
