Building FGCOM using MSVC

Read the README file for general details...

And as that README advises fgcom building has 
dependencies on -
1. OpenAL - either built from the OpenAL-soft 
   source, or from the Creative OpenAL SDK.
2. PLIB - for network code plib net.lib and ul.lib

This is a cmake build project, which will 
write a version.h file, and generate build 
files for which ever version of MSVC you have 
installed, or other generators if desired.

There are also quite a number of *.sln and 
*.vcproj files still in the source for MSVC 7.1,
vs2003, and vs2005. And some Makefile files.
These have NOT been used or updated, and WILL 
probaly fail. Use CMake instead...

The current cmake build builds two items -
1. iaxclient_lib.lib - a STATIC library which is linked into
2. fgcom.exe

That is using the Release configuration. There is 
also a Debug configuration where a 'd' will be appended 
to the names
 iaxclient_libd.lib and fgcomd.exe

Like most cmake built projects, there are also 
MinSizeRel and RelWithDebInfo configurations. These are not usually 
built or used by the maintainers.

The running of fgcom.exe 
========================

After succesfull build run an echo test with
>Release\fgcom -f910 --positions=..\fgcom\data\positions.txt

You MUST of course have a working microphone, and speakers attached 
to your system. If all successful you should see -
Call 0 accepted
Call 0 answered
output to the console, and you may need to allow fgcom access to 
the net, then words spoken into the mic should be echoed to the 
speaker(s). Use Ctrl+C to abort the application.

For an overview of command line options type:
>Release\fgcom --help

There are two new command line options for positions and frequency file paths:
-T, -positions, --positions=    #location positions file
-Q, -special, --special=        #location special frequencies file (optional)

Here is a guide about how to use FGCOM with FlightGear:
http://wiki.flightgear.org/Fgcom

Enjoy,
Geoff.
20120905 - 20130825 - 2013/01/30

# eof


