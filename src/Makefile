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
PLIB_PREFIX:=/usr/local/src/fgfs-builder/install
OPENAL_PREFIX:=/usr/local/src/fgfs-builder/install
SVNDEF:=-D'SVN_REV="$(shell svnversion -n .)"'
#CXXFLAGS:=-O2 -DDEBUG $(SVNDEF)
CXXFLAGS:=-O2 $(SVNDEF) -I $(OPENAL_PREFIX) -I ../iaxclient/lib -I $(PLIB_PREFIX)/include
LDFLAGS:=-L $(PLIB_PREFIX)/lib
STATIC_LIBS:=../iaxclient/lib/libiaxclient.a -lopenal -lasound -lplibul -lplibnet -lpthread -lm
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

all: fgcomstatic iaxclient

iaxclient:
	 cd ../iaxclient/lib && make

test: test.o
	cd ../iaxclient/lib && make
	$(CC) test.o $(STATIC_LIBS) -o test

fgcomstatic: fgcom.o usage.o position.o
	cd ../iaxclient/lib && make
	$(CXX) $(LDFLAGS) fgcom.o usage.o position.o $(STATIC_LIBS) -o fgcom

indent: fgcom.cpp fgcom.h
	$(INDENT) $(IFLAGS) fgcom.cpp
	$(INDENT) $(IFLAGS) fgcom.h
	$(INDENT) $(IFLAGS) usage.cpp
	$(INDENT) $(IFLAGS) position.cpp

install: all
	install -m 755 -s fgcom $(INSTALL_BIN)/fgcom
	install -d -m 755 $(INSTALL_DIR)
	install -m 644 ../data/positions.txt $(INSTALL_DIR)
	install -m 644 ../data/phonebook.txt $(INSTALL_DIR)

clean:
	cd ../iaxclient/lib && make clean
	rm -f *.o fgcom *~

fgcom.o: Makefile fgcom.cpp fgcom.h
	$(CXX) $(CXXFLAGS) -DDEFAULT_POSITIONS_FILE=\"$(INSTALL_DIR)/positions.txt\" -c fgcom.cpp

usage.o: Makefile usage.cpp
	$(CXX) $(CXXFLAGS) -c usage.cpp

position.o: Makefile position.cpp
	$(CXX) $(CXXFLAGS) -c position.cpp
