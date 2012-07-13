Building FGCOM using MSVC

Read the README file for general details, but 
the first step for MSVC is the copy the file
config.h.msvc to your build directory, naming 
it config.h

And as that README advises fgcom building has 
dependencies on -
1. OpenAL - either built from the OPenAL-soft 
   source, or from the Creative OpenAL SDK.

2. PLIB - for network code plib net.lib and ul.lib

3. SimGear - for SG_LOG (sgdebug;sgmisc)


This is a cmake build project, which will 
write a version.h file, and generate build 
files for which ever version of MSVC you have 
installed, or other generators if desired.

There are also quite a number of *.sln and 
*.vcproj files still in the source for MSVC 7.1,
vs2003, and vs2005. And some Makefile files.
These have NOT been used or updated, and may 
fail.

The current cmake build builds two items -
1. iaxclient_lib.lib - a STATIC library which is linked into
2. fgcom.exe

That is using the Release configuration. There is 
also a Debug configuration where a 'd' will be appended 
to the names
 iaxclient_libd.lib and fgcomd.exe

Like most cmake built projects, there are also 
MinSizeRel and RelWithDebInfo configurations. None of 
these have been built or used by the maintainers.

While the cmake files contains as options to 
build a shared library, a DLL this is disabled 
by default, and no check has been done on such 
DLL building.

The running of fgcom.exe 