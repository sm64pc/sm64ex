## Process this file with automake to produce Makefile.in

SUBDIRS = gtest libaudiofile sfcommands test examples docs

EXTRA_DIST = \
	ACKNOWLEDGEMENTS \
	NOTES \
	README \
	TODO \
	COPYING.GPL \
	configure configure.ac \
	audiofile.spec.in \
	audiofile.pc.in \
	audiofile-uninstalled.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = audiofile.pc

dist-hook: audiofile.spec
	cp audiofile.spec $(distdir)

if ENABLE_COVERAGE
coverage:
	$(MAKE) coverage-reset
	$(MAKE) check
	$(MAKE) coverage-report

coverage-reset:
	$(LCOV) --base-directory=@top_srcdir@ --directory @top_srcdir@/libaudiofile --zerocounters

coverage-report:
	$(LCOV) --directory @top_srcdir@/libaudiofile \
		--capture \
		--output-file @top_builddir@/lcov.info

	$(LCOV) --directory @top_srcdir@/libaudiofile \
		--output-file @top_builddir@/lcov.info \
		--remove @top_builddir@/lcov.info \
		"/usr/include/*" "gtest/*" "*/UT_*"

	$(mkdir_p) @top_builddir@/coverage
	git_commit=`GIT_DIR=@top_srcdir@/.git git log -1 --pretty=format:%h 2>/dev/null`; \
	$(GENHTML) --title "@PACKAGE@ @VERSION@ $$git_commit" \
		--output-directory @top_builddir@/coverage @top_builddir@/lcov.info
	@echo
	@echo 'lcov report can be found here:'
	@echo 'file://@abs_top_builddir@/coverage/index.html'
	@echo

clean-local:
	-rm -rf coverage

.PHONY: coverage-reset coverage coverage-report
else
coverage:
	@echo "Code coverage is not enabled."
	@exit 1
endif
