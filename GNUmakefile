include settings.make

.PHONY: default doc src test tests clean all

default: src

all: src doc tests

src:
	$(MAKE) -C src

tests: src
	$(MAKE) -C tests

test: tests
	tests/tests

doc:
	doxygen doxygen.config

dist:
	-mkdir dist
	rm -rf dist/$(distname)
	git clone ./ dist/$(distname)
	rm -rf dist/$(distname)/.git
	cd dist; tar zcvf $(distname).tar.gz $(distname)

debuild: dist
	cp -a dist/debian $(distname)/
	cd dist/$(distname); debuild

clean:
	$(MAKE) -C src clean
	$(MAKE) -C tests clean
	rm -rf doc/html doc/latex
	rm -rf dist/evilpoison*
