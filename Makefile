# Guess the operating system
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
PLIB_PREFIX:=/usr
OPENAL_PREFIX:=/usr
SVNDEF:=-D'SVN_REV="$(shell svnversion -n .)"'
CXXFLAGS:=-O2 $(SVNDEF) -I $(OPENAL_PREFIX) -I iaxclient/lib -I $(PLIB_PREFIX)/include
LDFLAGS:=-L $(PLIB_PREFIX)/lib
STATIC_LIBS:=iaxclient/lib/libiaxclient.a -lopenal -lasound -lpthread
INDENT:=/usr/bin/indent
IFLAGS:=
INSTALL_BIN:=/usr/local/bin
INSTALL_DIR:=/usr/local/fgcom
endif

ifeq ($(OSTYPE),MACOSX)
PLIB_PREFIX:=../../PLIB/build/Release/PLIB.framework
SVNDEF:=-D'SVN_REV="$(shell svnversion -n .)"'
CXXFLAGS:=-O2 $(SVNDEF) -I ../iaxclient/lib -I $(PLIB_PREFIX)/Headers
LDFLAGS:= -F$(PLIB_PREFIX)/.. -framework OpenAL -framework PLIB
STATIC_LIBS:=../iaxclient/lib/libiaxclient.a -lpthread -lm
INDENT:=/usr/bin/indent
IFLAGS:=
INSTALL_BIN:=../FlightGearOSX/build/Deployment/FlightGear.app/Contents/Resources
INSTALL_DIR:=$(INSTALL_BIN)/data/fgcom
endif

all: fgcom3

iaxclient:
	 cd iaxclient/lib && make

fgcom3: fgcom3.o iaxclient
	$(CXX) $(LDFLAGS) fgcom3.o $(STATIC_LIBS) -o fgcom3

indent: fgcom.cpp fgcom.h
	$(INDENT) $(IFLAGS) fgcom3.c

install: all
	install -m 755 -s fgcom3 $(INSTALL_BIN)/fgcom

clean:
	cd iaxclient/lib && make clean
	rm -f *.o fgcom3 *~

fgcom3.o: Makefile fgcom3.c
	$(CXX) $(CXXFLAGS) -c fgcom3.c

