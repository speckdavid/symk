
# $Id$

all: docs

docs: Doxyfile
	doxygen Doxyfile

clean:
	rm -rf html/ latex/ Doxyfile

install: docs
uninstall: clean

Doxyfile: ../src/revision.h ../configure.ac Doxyfile.skel
	./BuildDoxyFile

distdir:
	cp BuildDoxyFile Doxyfile.skel Makefile $(distdir)/.
