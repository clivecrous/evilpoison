include ../settings.make

HEADERS  = evilpoison.h log.h settings.h
SRCS     = client.c dictionary.c events.c main.c misc.c \
					 new.c screen.c ewmh.c settings.c
OBJS     = $(SRCS:.c=.o)

.PHONY: all install dist debuild clean

all: evilpoison

evilpoison: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

install: evilpoison
	if [ -f evilpoison.exe ]; then mv evilpoison.exe evilpoison; fi
	mkdir -p $(prefix)/bin $(prefix)/share/man/man1
	install -s evilpoison $(prefix)/bin
	install evilpoison.1 $(prefix)/share/man/man1
	#gzip -9 $(prefix)/share/man/man1/evilpoison.1

dist:
	darcs dist --dist-name $(distname)
	mv $(distname).tar.gz ..

debuild: dist
	-cd ..; rm -rf $(distname)/ $(distname).orig/
	cd ..; mv $(distname).tar.gz evilpoison$(version).orig.tar.gz
	cd ..; tar xfz evilpoison$(version).orig.tar.gz
	cp -a debian ../$(distname)/
	rm -rf ../$(distname)/debian/_darcs/
	cd ../$(distname); debuild

clean:
	rm -f evilpoison evilpoison.exe $(OBJS)
