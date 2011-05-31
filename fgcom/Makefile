INSTALL_BIN:=/usr/local/bin
SVNDEF:=-D'SVN_REV="$(shell svnversion -n .)"'
#CFLAGS=-D__DEBUG__ -g -L/usr/lib/debug -I/usr/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/gnet-2.0/include -I/usr/include/gnet-2.0
CFLAGS=-I/usr/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/gnet-2.0/include -I/usr/include/gnet-2.0
LIBS:=-lgnet-2.0 -lglib-2.0 -lopenal

all: fgcom Makefile

install: all
	install -m 755 -s fgcom $(INSTALL_BIN)/fgcom

uninstall: 
	rm -f $(INSTALL_BIN)/fgcom

clean:
	rm -f *.o *~

distclean: clean
	rm -f fgcom

fgcom: config.o fgcom.o net.o
	$(CC) $(LDFLAGS) config.o  fgcom.o  net.o  $(LIBS) -o fgcom

config.o: config.c config.h
	$(CC) $(CFLAGS) -c config.c

fgcom.o: fgcom.c fgcom.h
	$(CC) $(CFLAGS) -c fgcom.c

net.o: net.c net.h
	$(CC) $(CFLAGS) -c net.c

indent: *.c *.h
	indent \
		--use-tabs \
		--tab-size5 \
		--indent-level5 \
		--blank-lines-after-declarations \
		--blank-lines-after-procedures \
		--blank-lines-before-block-comments \
		--braces-after-if-line \
		--brace-indent0 \
	$?

print: *.[hc]
	trueprint \
		--ps-level-two \
		--no-page-break-after-function \
		--intelligent-line-wrap \
		--tabsize=5 \
		--double-sided \
		--output=source-code.ps \
		$?
	ps2pdf14 source-code.ps

