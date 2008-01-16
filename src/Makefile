PLIB_PREFIX:=/usr/local
SVNDEF:=-D'SVN_REV="$(shell svnversion -n .)"'
#CXXFLAGS:=-O2 -DDEBUG $(SVNDEF)
CXXFLAGS:=-O2 $(SVNDEF) -I ../iaxclient/lib -I $(PLIB_PREFIX)/include
LDFLAGS:=-L $(PLIB_PREFIX)/lib
STATIC_LIBS:=../iaxclient/lib/libiaxclient.a -lopenal -lasound -lplibul -lplibnet -lpthread -lm
INDENT:=/usr/bin/indent
IFLAGS:=
INSTALL_BIN:=/usr/local/bin
INSTALL_DIR:=/usr/local/fgcom

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
