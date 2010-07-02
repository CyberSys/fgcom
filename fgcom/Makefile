INSTALL_BIN:=/usr/local/bin
SVNDEF:=-D'SVN_REV="$(shell svnversion -n .)"'
CFLAGS:=-O2 $(SVNDEF) -I ../iaxclient/lib -I/usr/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/gnet-2.0/include -I/usr/include/gnet-2.0
LDFLAGS:=-L../iaxclient/lib/.libs
LIBS:=-liaxclient -lspeex -lspeexdsp -lgnet-2.0 -lglib-2.0 -loggz -lvidcap

all: fgcom Makefile

testclient: testclient.c Makefile iaxclient
	$(CC) $(CFLAGS) -c testclient.c
	$(CC) $(LDFLAGS) testclient.o $(LIBS) -o testclient

lighthouse: lighthouse.c oggfile.c oggfile.h Makefile iaxclient
	$(CC) $(CFLAGS) -c lighthouse.c
	$(CC) $(CFLAGS) -c oggfile.c
	$(CC) $(LDFLAGS) lighthouse.o oggfile.o $(LIBS) -o lighthouse

install: all
	install -m 755 -s fgcom $(INSTALL_BIN)/fgcom

uninstall: 
	rm -f $(INSTALL_BIN)/fgcom

clean:
	@if [ -f ../iaxclient/Makefile ]; then \
                cd ../iaxclient && make clean; \
	fi
	rm -f *.o *~ testclient lighthouse

distclean: clean
	@if [ ! -f ../iaxclient/Makefile ]; then \
                cd ../iaxclient && make distclean; \
	fi
	rm -f fgcom

fgcom: iaxclient config.o event.o fgcom.o mode.o net.o oggfile.o
	$(CC) $(LDFLAGS) config.o event.o fgcom.o mode.o net.o oggfile.o $(LIBS) -o fgcom

iaxclient: ../iaxclient/Makefile
	@cd ../iaxclient && make

../iaxclient/Makefile:
	@cd ../iaxclient; \
	./configure --prefix=/usr --without-ogg --without-theora --without-vidcap --disable-clients 
	#./configure --prefix=/usr --with-gnu-ld --with-echo-can --without-ogg --without-theora --without-vidcap --disable-clients 

config.o: config.c config.h
	$(CC) $(CFLAGS) -c config.c

event.o: event.c event.h
	$(CC) $(CFLAGS) -c event.c

fgcom.o: fgcom.c fgcom.h
	$(CC) $(CFLAGS) -c fgcom.c

mode.o: mode.c mode.h fgcom.h
	$(CC) $(CFLAGS) -c mode.c

net.o: net.c net.h
	$(CC) $(CFLAGS) -c net.c
