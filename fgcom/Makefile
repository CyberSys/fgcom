ifneq (,$(findstring Linux,$(shell uname)))
OSTYPE=LINUX
else
ifneq (,$(findstring Darwin,$(shell uname)))
OSTYPE=MACOSX
else
# CYGWIN reports CYGWIN_NT-5.0 under Win2K
ifneq (,$(findstring WIN,$(shell uname)))
OSTYPE=WIN32
else
ifneq (,$(findstring MINGW,$(shell uname)))
OSTYPE=WIN32
else
ifneq (,$(findstring SunOS,$(shell uname)))
OSTYPE=SOLARIS
else
$(warning OSTYPE cannot be detected, assuming Linux)
OSTYPE=LINUX
endif
endif
endif
endif
endif

ifeq ($(OSTYPE),LINUX)
INSTALL_PATH:=/usr/local/fgcom
CC:=gcc
SVNDEF:=-D'SVN_REV="$(shell svnversion -n .)"'
CFLAGS:=-O2 $(SVNDEF)
LDFLAGS:=-L $(INSTALL_PATH)/lib
LIBS:=-lglib-2.0 -lgnet-2.0 -loggz -liaxclient -lspeex -lspeexdsp
IFLAGS:=-I $(INSTALL_PATH)/include -I/usr/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/include/gnet-2.0/ -I/usr/lib/gnet-2.0/include/
INSTALL_BIN:=$(INSTALL_PATH)/bin
endif

ifeq ($(OSTYPE),MACOSX)
PLIB_PREFIX:=../../PLIB/build/Release/PLIB.framework
SVNDEF:=-D'SVN_REV="$(shell svnversion -n .)"'
CXXFLAGS:=-O2 $(SVNDEF) -I ../iaxclient/lib -I $(PLIB_PREFIX)/Headers 
LDFLAGS:= -F$(PLIB_PREFIX)/.. -framework OpenAL -framework PLIB
STATIC_LIBS:=../iaxclient/lib/libiaxclient.a ../speex/libspeex/.libs/libspeex.a
LIBS:=-lpthread -lm
INDENT:=/usr/bin/indent
IFLAGS:=
INSTALL_BIN:=../FlightGearOSX/build/Deployment/FlightGear.app/Contents/Resources
INSTALL_DIR:=$(INSTALL_BIN)/data/fgcom
endif

all: fgcom

fgcom: config.o event.o fgcom.o mode.o net.o oggfile.o
	$(CC) $(LDFLAGS) config.o event.o fgcom.o mode.o net.o oggfile.o $(LIBS) -o fgcom

install: all
	install -m 755 -s fgcom $(INSTALL_PATH)/bin/fgcom
	install -d -m 755 $(INSTALL_PATH)

uninstall: 
	rm -f $(INSTALL_PATH)

clean:
	make -C ../iaxclient distclean
	make -C ../speex distclean
	rm -f *.o fgcom *~

config.o: config.c config.h
	$(CC) $(CFLAGS) $(IFLAGS) -c config.c

event.o: event.c event.h
	$(CC) $(CFLAGS) $(IFLAGS) -c event.c

fgcom.o: fgcom.c fgcom.h
	$(CC) $(CFLAGS) $(IFLAGS) -c fgcom.c

mode.o: mode.c mode.h
	$(CC) $(CFLAGS) $(IFLAGS) -c mode.c

net.o: net.c net.h
	$(CC) $(CFLAGS) $(IFLAGS) -c net.c

oggfile.o: oggfile.c oggfile.h
	$(CC) $(CFLAGS) $(IFLAGS) -c oggfile.c
