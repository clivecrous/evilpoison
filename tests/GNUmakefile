include ../settings.make

HEADERS  = check_settings.h
SRCS     = tests.c check_dictionary.c check_settings.c
OBJS     = $(SRCS:.c=.o) \
	../src/dictionary.o ../src/settings.o

.PHONY: all install dist debuild clean

all: tests

tests: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS) -lcheck

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -I../src -L../src

clean:
	rm -f tests tests.exe $(OBJS)