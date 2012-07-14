FGCOM
=====

This project can be build using CMake 2.8 or later
See : http://www.cmake.org/ for download and install details.
Alternate you can install cmake with macports (http://www.macports.org/), where you can get also fgcom plib dependency.

Dependencies:

### SimGear - for SG_LOG (sgdebug;sgmisc)
http://www.gitorious.org/fg/simgear

You will find a guide how to install and compile simgear/flightgear for osx here: http://wiki.flightgear.org/FlightGear_Git_on_Mac_OS_X
IMPORTANT: When you follow this guide you will have all fgcom deps installed and you can skip next sections OpenAL and PLIB

### OpenAL - sound library

is part of Core Audio chez Apple, install Xcode/SDKs
http://developer.apple.com

### PLIB - for network code plib net.lib and ul.lib

comes with macports (port install plib +universal) or download latest source at http://plib.sourceforge.net/download.html and patch this source with current macports patches before you try to compile.


Building FGCOM for OSX
----------------------

1. Create an out of source 'build' directory

$ mkdir build-fgcom
$ cd build-fgcom

$ cmake . ../fgcom -DCMAKE_BUILD_TYPE=RELEASE
$ sudo make install



That's it folks ;=))
Geoff
20120714 (edited by Yves for OSX)

; eof

