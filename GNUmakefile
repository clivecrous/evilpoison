.PHONY: default doc src tests clean all

default: src

all: src doc tests

src:
	$(MAKE) -C src

tests: src
	$(MAKE) -C tests

doc:
	doxygen doxygen.config

clean:
	$(MAKE) -C src clean
	$(MAKE) -C tests clean
	rm -rf doc/html doc/latex
