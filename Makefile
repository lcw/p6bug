CC = mpicc
CXX = mpicxx
F77 = mpif77
FC = mpif90
CFLAGS = --std=gnu11 -g -O2

CFLAGS +=-fno-common -fomit-frame-pointer

CFLAGS += -Wconversion -Wno-sign-conversion \
          -Wchar-subscripts -Wall -W \
          -Wpointer-arith -Wwrite-strings -Wformat-security -pedantic \
          -Wextra -Wno-unused-parameter

# list of libraries to build
TPLS ?= p4est

UNAME_S := $(shell uname -s)

# p4est flags
CPPFLAGS += -Ip4est/local/include
LDFLAGS += -Lp4est/local/lib
LDLIBS += -lp4est -lsc
ifeq ($(UNAME_S),Linux)
 LDFLAGS += -Wl,-rpath=$(CURDIR)/p4est/local/lib,--enable-new-dtags
endif

all: p6bug

p4est:
	tar xzf tpl/p4est-*.tar.gz && mv p4est-* p4est
	cd p4est && ./configure CC=$(CC) CXX=$(CXX) F77=$(F77) FC=$(FC) --enable-mpi --enable-debug \
           && $(MAKE) install

# Dependencies
p6bug: p6bug.c $(DEPS_SOURCE) $(DEPS_HEADERS)  | $(TPLS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) \
        $< $(DEPS_SOURCE) $(LOADLIBES) $(LDLIBS) -o $@

# Rules
.PHONY: clean realclean
clean:
	rm -rf p6bug *.o

realclean:
	rm -rf $(TPLS)
	git clean -X -d -f
