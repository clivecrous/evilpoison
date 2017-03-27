include ../settings.make

HEADERS  = check_settings.h
SRCS     = tests.c \
					 check_bind.c check_command.c \
					 check_dictionary.c check_settings.c
OBJS     = $(SRCS:.c=.o)
EPOBJS   = ../src/bind.o ../src/command.o \
					 ../src/commandline.o ../src/dictionary.o \
					 ../src/settings.o

.PHONY: all install dist debuild clean epobjs

all: tests

epobjs:
	make -C ../src $(EPOBJS)

tests: epobjs $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(EPOBJS) -o $@ $(LDFLAGS) -lcheck -lm -lrt -lsubunit

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -I../src -L../src

clean:
	rm -f tests tests.exe $(OBJS)
