INSTALL_BIN:=/usr/local/fgcom/bin
INSTALL_DIR:=/usr/local/fgcom
SVNDEF:=-D'SVN_REV="$(shell svnversion -n .)"'
CFLAGS:=-O2 $(SVNDEF) -I $(INSTALL_DIR)/include -I/usr/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/gnet-2.0/include -I/usr/include/gnet-2.0
LDFLAGS:=-L $(INSTALL_DIR)/lib
LIBS:=-liaxclient -lsndfile -lspeex -lspeexdsp -lopenal -glib-2.0 -lgnet-2.0 -loggz

all: fgcom

install: all
	install -m 755 -s fgcom $(INSTALL_BIN)/fgcom
	install -d -m 755 $(INSTALL_DIR)

uninstall: 
	rm -f $(INSTALL_BIN)/fgcom
	rm -rf $(INSTALL_DIR)

clean:
	@if [ ! -f ../iaxclient/Makefile ]; then \
                cd ../iaxclient/lib && make clean; \
	fi
	rm -f *.o *~

distclean: clean
	@if [ ! -f ../iaxclient/Makefile ]; then \
                cd ../iaxclient/lib && make distclean; \
	fi
	rm -f fgcom

fgcom: iaxclient config.o  event.o  fgcom.o  mode.o  net.o  oggfile.o
	$(CC) $(LDFLAGS) config.o  event.o  fgcom.o  mode.o  net.o  oggfile.o $(LIBS) -o fgcom

iaxclient:
	@if [ ! -f ../iaxclient/Makefile ]; then \
                cd ../iaxclient/lib && make; \
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

oggfile.o: oggfile.c oggfile.h
	$(CC) $(CFLAGS) -c oggfile.c
