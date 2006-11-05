.PHONY: default src tests clean all

default: src

all: src tests

src:
	$(MAKE) -C src

tests: src
	$(MAKE) -C tests

clean:
	$(MAKE) -C src clean
	$(MAKE) -C tests clean
