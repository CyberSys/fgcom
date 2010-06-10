INSTALL_BIN:=/usr/local/fgcom/bin
INSTALL_DIR:=/usr/local/fgcom
SVNDEF:=-D'SVN_REV="$(shell svnversion -n .)"'
CFLAGS:=-O2 $(SVNDEF) -I ../iaxclient -I/usr/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/gnet-2.0/include -I/usr/include/gnet-2.0
LDFLAGS:=-L ../iaxclient
LIBS:=-liaxclient -lspeex -lspeexdsp -lsndfile -lopenal -lglib-2.0 -lgnet-2.0

all: fgcom

install: all
	install -m 755 -s fgcom $(INSTALL_BIN)/fgcom
	install -d -m 755 $(INSTALL_DIR)

uninstall: 
	rm -f $(INSTALL_BIN)/fgcom
	rm -rf $(INSTALL_DIR)

clean:
	@if [ -f ../iaxclient/Makefile ]; then \
                cd ../iaxclient && make clean; \
	fi
	rm -f *.o *~

distclean: clean
	@if [ ! -f ../iaxclient/Makefile ]; then \
                cd ../iaxclient && make distclean; \
	fi
	rm -f fgcom

fgcom: iaxclient config.o event.o fgcom.o mode.o net.o
	$(CC) $(LDFLAGS) config.o event.o fgcom.o mode.o net.o $(LIBS) -o fgcom

iaxclient:
	@if [ -f ../iaxclient/Makefile ]; then \
                cd ../iaxclient && make; \
	fi

config.o: config.c config.h
	$(CC) $(CFLAGS) -c config.c

event.o: event.c event.h
	$(CC) $(CFLAGS) -c event.c

fgcom.o: fgcom.c fgcom.h
	$(CC) $(CFLAGS) -c fgcom.c

mode.o: mode.c mode.h
	$(CC) $(CFLAGS) -c mode.c

net.o: net.c net.h
	$(CC) $(CFLAGS) -c net.c
