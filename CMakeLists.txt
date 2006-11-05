project(evilpoison,c)

set(VERSION 0.1)
#set(CMAKE_VERBOSE_MAKEFILE true)
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

set(CMAKE_C_FLAGS_DEBUG "-g -Wall -W -Wstrict-prototypes -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow -Waggregate-return -Wnested-externs -Winline -Wwrite-strings -Wundef -Wsign-compare -Wmissing-prototypes -Wredundant-decls")
set(CMAKE_C_FLAGS_RELEASE "-O3")

subdirs(src tests)
