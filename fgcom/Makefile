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
INSTALL_BIN:=/usr/local/fgcom/bin
INSTALL_DIR:=/usr/local/fgcom
PKG_CONFIG_LIBDIR:=$(PKG_CONFIG_LIBDIR):$(INSTALL_PATH)
SVNDEF:=-D'SVN_REV="$(shell svnversion -n .)"'
CFLAGS:=-O2 $(SVNDEF) -I $(INSTALL_DIR)/include -I/usr/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/gnet-2.0/include -I/usr/include/gnet-2.0
LDFLAGS:=-L $(INSTALL_DIR)/lib
LIBS:=-liaxclient -lspeex -lspeexdsp -lopenal -glib-2.0 -lgnet-2.0 -loggz -lvidcap
endif

#
# The make code for MACOSX must be fixed... sorry
#
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

install: all
	install -m 755 -s fgcom $(INSTALL_BIN)/fgcom
	install -d -m 755 $(INSTALL_DIR)

uninstall: 
	rm -f $(INSTALL_BIN)/fgcom
	rm -rf $(INSTALL_DIR)

clean:
	make -C ../iaxclient clean
	make -C ../speex clean
	rm -f *.o *~

distclean: clean
	make -C ../iaxclient distclean
	make -C ../speex distclean
	rm -f fgcom

fgcom: iaxclient speex config.o  event.o  fgcom.o  mode.o  net.o  oggfile.o
	$(CC) $(LDFLAGS) config.o  event.o  fgcom.o  mode.o  net.o  oggfile.o $(LIBS) -o fgcom

iaxclient: speex
	@if [ ! -f ../iaxclient/configure ]; then \
		cd ../iaxclient; \
		sh autogen.sh; \
	fi
	@if [ ! -f ../iaxclient/Makefile ]; then \
		cd ../iaxclient; \
		export PKG_CONFIG_PATH="$(INSTALL_DIR)/lib/pkgconfig:$(PKG_CONFIG_PATH)";\
		./configure --prefix=$(INSTALL_DIR) --disable-video -without-theora --without-vidcap --disable-gsmtest --disable-iax2test --enable-local-gsm --disable-clients --with-openal --with-echo-can=speex;\
	fi
	make CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C ../iaxclient
	make -C ../iaxclient install

speex:
	@if [ ! -f ../speex/Makefile ]; then \
		cd ../speex; \
		./configure --prefix=$(INSTALL_DIR) --enable-sse;\
	fi
	make -C ../speex
	make -C ../speex install
	
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
