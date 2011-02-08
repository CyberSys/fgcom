##
## Makefile for the FGCom project.
##





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
PLIB_PREFIX := /usr/local/src/fgfs-builder/install
OPENAL_PREFIX := /usr/local/src/fgfs-builder/install
SVNDEF := -D'SVN_REV="$(shell svnversion -n .)"'
INSTALL_BIN := /usr/local/bin
INSTALL_DIR := /usr/local/fgcom
SIMGEAR_INC := ~/tmp/simgear
DEBUG := -O0 -Wall -ggdb
CXXFLAGS := $(DEBUG) $(SVNDEF) -I $(OPENAL_PREFIX) -I ../iaxclient/lib -I $(PLIB_PREFIX)/include
CXXFLAGS += -I $(SIMGEAR_INC) -DDEFAULT_POSITIONS_FILE=\"$(INSTALL_DIR)/positions.txt\" -DSPECIAL_FREQUENCIES_FILE=\"$(INSTALL_DIR)/special_frequencies.txt\"
LDFLAGS := -L $(PLIB_PREFIX)/lib
STATIC_LIBS := ../iaxclient/lib/libiaxclient.a -lopenal -lasound -lplibul -lplibnet -lpthread -lsgmisc -lsgdebug -lz -lm
INDENT := /usr/bin/indent
ECHO := /bin/echo -e
IFLAGS :=
endif

ifeq ($(OSTYPE),MACOSX)
PLIB_PREFIX:=../../PLIB/build/Release/PLIB.framework
SVNDEF:=-D'SVN_REV="$(shell svnversion -n .)"'
CXXFLAGS:=-O2 $(SVNDEF) -I ../iaxclient/lib -I $(PLIB_PREFIX)/Headers
LDFLAGS:= -F$(PLIB_PREFIX)/.. -framework OpenAL -framework PLIB
STATIC_LIBS:=../iaxclient/lib/libiaxclient.a -lpthread -lm
INDENT:=/usr/bin/indent
IFLAGS:=-DDEFAULT_POSITIONS_FILE=\"$(INSTALL_DIR)/positions.txt\"
INSTALL_BIN:=../FlightGearOSX/build/Deployment/FlightGear.app/Contents/Resources
INSTALL_DIR:=$(INSTALL_BIN)/data/fgcom
endif

##
## Look for all C source files (.c & .cpp)
##
CPPFILES := $(wildcard *.cpp)
HFILES := $(patsubst %.cpp,%.h,$(CPPFILES))
OFILES := $(patsubst %.cpp,%.o,$(CPPFILES))



.PHONY:	all iaxclient fgcomstatic indent install clean list


all: fgcomstatic iaxclient

iaxclient:
	 cd ../iaxclient/lib && make

fgcomstatic: $(OFILES)
	cd ../iaxclient/lib && make
	$(CXX) $(LDFLAGS) $(OFILES) $(STATIC_LIBS) -o fgcom

indent:	$(CPPFILES) $(HFILES)
	$(INDENT) $(IFLAGS) $(CPPFILES) $(HFILES)

install: all
	install -m 755 -s fgcom $(INSTALL_BIN)/fgcom
	install -d -m 755 $(INSTALL_DIR)
	install -m 644 ../data/positions.txt $(INSTALL_DIR)
	install -m 644 ../data/phonebook.txt $(INSTALL_DIR)
	install -m 644 ../data/special_frequencies.txt $(INSTALL_DIR)

clean:
	cd ../iaxclient/lib && make clean
	rm -f *.o fgcom *~

%o:	%cpp $(HFILES)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

list:	
	@$(ECHO) "Source files: \t" $(CPPFILES)
	@$(ECHO) "Header files: \t" $(HFILES)
	@$(ECHO) "Object files: \t" $(OFILES)

#
# End of file.
#

