TOPDIR     := $(PWD)
DESTDIR    :=
PREFIX     := /usr/local
LIBDIR     := $(PREFIX)/lib

CROSS_COMPILE :=

CC         := $(CROSS_COMPILE)gcc
LIB_CFLAGS := -fPIC -O2
DEB_CFLAGS := -Wall -W -g
USR_CFLAGS :=
INC_CFLAGS := -I$(TOPDIR)/include
CFLAGS     := $(LIB_CFLAGS) $(DEB_CFLAGS) $(USR_CFLAGS) $(INC_CFLAGS)

LD         := $(CC)
DEB_LFLAGS := -g
USR_LFLAGS :=
LIB_LFLAGS := -ldl
LDFLAGS    := $(DEB_LFLAGS) $(USR_LFLAGS) $(LIB_LFLAGS)

MAJOR      := 0
MINOR      := 1
VERSION    := $(MAJOR).$(MINOR)
SHARED     := libetcept.so
SO_MAJ     := $(SHARED).$(MAJOR)
SONAME     := $(SHARED).$(VERSION)

all: $(SHARED)

$(SHARED): $(SO_MAJ)
	ln -sf $(SO_MAJ) $(SHARED)

$(SO_MAJ): $(SONAME)
	ln -sf $(SONAME) $(SO_MAJ)

$(SONAME): etcept.o
	$(LD) -shared $(LDFLAGS) -Wl,-soname,$(SONAME) -o $(SONAME) etcept.o

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

install:
	[ -d "$(DESTDIR)$(LIBDIR)/." ] || mkdir -p -m 0755 $(DESTDIR)$(LIBDIR)
	cp    $(SONAME) $(DESTDIR)$(LIBDIR)/$(SONAME)
	cp -P $(SO_MAJ) $(DESTDIR)$(LIBDIR)/$(SO_MAJ)
	cp -P $(SHARED) $(DESTDIR)$(LIBDIR)/$(SHARED)

clean:
	-rm -f *.so* *.o
