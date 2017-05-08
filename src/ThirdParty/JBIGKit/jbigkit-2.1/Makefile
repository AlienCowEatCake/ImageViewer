# Unix makefile for JBIG-KIT

# Select an ANSI/ISO C compiler here, GNU gcc is recommended
CC = gcc

# Options for the compiler: A high optimization level is suggested
CFLAGS = -O2 -W -Wno-unused-result
# CFLAGS = -O -g -W -Wall -Wno-unused-result -ansi -pedantic # -DDEBUG

export CC CFLAGS

VERSION=2.1

all: lib pbm
	@echo "Enter 'make test' in order to start some automatic tests."

lib:
	cd libjbig && $(MAKE) -e

pbm: lib
	cd pbmtools && $(MAKE) -e

test: lib pbm
	cd libjbig  && $(MAKE) test
	cd pbmtools && $(MAKE) test

analyze:
	cd libjbig  && $(MAKE) analyze
	cd pbmtools && $(MAKE) analyze

clean:
	rm -f *~ core
	cd libjbig  && $(MAKE) clean
	cd pbmtools && $(MAKE) clean

distribution:
	rm -rf jbigkit-$(VERSION)
	git archive v$(VERSION) --prefix jbigkit-$(VERSION)/ | tar xvf -
	make -C jbigkit-$(VERSION)/pbmtools txt
	tar cvaf jbigkit-$(VERSION).tar.gz jbigkit-$(VERSION)

release:
	rsync -t jbigkit-$(VERSION).tar.gz $(HOME)/public_html/download/
	rsync -t jbigkit-$(VERSION)/CHANGES $(HOME)/public_html/jbigkit/
