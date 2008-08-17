include ../settings.make

HEADERS  = commandline.h evilpoison.h log.h settings.h xinerama.h
SRCS     = bind.c client.c command.c commandline.c dictionary.c events.c \
					 evilpoison_commands.c main.c misc.c xinerama.c \
					 new.c screen.c ewmh.c settings.c
OBJS     = $(SRCS:.c=.o)

.PHONY: all install dist debuild clean

all: evilpoison

evilpoison: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c $(HEADERS) revision
	$(CC) $(CFLAGS) -c $<

revision:
ifneq "$(git_revision)" "$(file_revision)"
	echo "$(git_revision)" > revision
endif

install: evilpoison
	if [ -f evilpoison.exe ]; then mv evilpoison.exe evilpoison; fi
	mkdir -p $(prefix)/bin $(prefix)/share/man/man1
	install -s evilpoison $(prefix)/bin
	install evilpoison.1 $(prefix)/share/man/man1
	#gzip -9 $(prefix)/share/man/man1/evilpoison.1

clean:
	rm -f evilpoison evilpoison.exe $(OBJS) revision
