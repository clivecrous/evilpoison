# If you don't use CC 
CC       = gcc

CFLAGS  += -std=gnu99 

# Edit this line if you don't want evilpoison to install under /usr.
# Note that $(DESTDIR) is used by the Debian build process.
prefix = $(DESTDIR)/usr

XROOT    = /usr/X11R6
INCLUDES = -I$(XROOT)/include
LDPATH   = -L$(XROOT)/lib
LIBS     = -lX11 -lpthread -lXext -lXinerama

DEFINES  = $(EXTRA_DEFINES)
# Configure evilpoison by editing the following DEFINES lines.  You can also
# add options by setting EXTRA_DEFINES on the make(1) command line,
# e.g., make EXTRA_DEFINES="-DDEBUG".

# Uncomment to compile in certain text messages like help.  You want this too
# unless you *really* want to trim the bytes.
# Note that snprintf(3) is always part of the build.
DEFINES += -DSTDIO

# Print whatever debugging messages I've left in this release.
#DEFINES += -DDEBUG	# miscellaneous debugging

# ----- You shouldn't need to change anything under this line ------ #

version = 0.9.$(shell git rev-list --all|wc -l)

distname = evilpoison-$(version)

#DEFINES += -DXDEBUG	# show some X calls

DEFINES += -DVERSION=\"$(version)\" $(DEBIAN)

# Optimize for size
#CFLAGS  += $(INCLUDES) $(DEFINES) -Os -Wall
# Optimize for speed
#CFLAGS  += $(INCLUDES) $(DEFINES) -O3 -Wall
# Debug build
CFLAGS  += $(INCLUDES) $(DEFINES) -g -Wall

CFLAGS  += -W -Wstrict-prototypes -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow -Waggregate-return -Wnested-externs -Winline -Wwrite-strings -Wundef -Wsign-compare -Wmissing-prototypes -Wredundant-decls
LDFLAGS += $(LDPATH) $(LIBS)
