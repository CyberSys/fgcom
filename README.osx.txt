
---------------------------------------------------------------------------------------
                                                        .&HMMHMHMMQMMQgBMMm.
MMMMMMMMF  .MMMMMMa    .MMMMMMp    .MMMMMM.   JMMMb   .MMMFNMMMHMMMNKMNMNfNMMx
MMM!!!!!!  MM#! JMMF   MM#! 4MM,  .MMF .4MM   JMb4M.  JMJMFMMNMMMMHMMMM#J#GH#Mc
MMM       .MMF        .MMF  .""^  JMMF  JMMr  JMMJMF .M@JMFHHMM#NMNMMNHMp?x4dNM
MMMMMMF   .MMF .....  .MMF        JMMF  JMMF  JMM MM JMFJMFM#MMN#TMMMMMMMbJxWMMr
MMM"""5   .MMF JMMMF  .MMF        JMMF  JMMF  JMM JMFMM JMFT""!    ?MM#MMMNh.MMF
MMM       .MMF   MMF  .MMF  JMMF  JMMF  JMMt  JMM .MMMF JMF          ."MMNN KJM
MMM        MMM&.JMMF   MMM..MMM^   MMN..MMM   JMM  HMM' JMF              "WFJ.5
MMM        .WMMMMFMF   .YMMMM#^    .WMMMMY!   JMM  JM#  JMb.               .mJ
                                                .JYuMHWMMNMMMx              717.
                                               .E!.@XNMMNMMMMMN             .,. ?&
FGCOM VoIP Client                              J!Q#KMMMNMMJM#NMF      ..&WM#0.&J` ?&
for the FlightGear Radio                      .#.MXd#M#MMM@MNMMF   ..MdM"5J"!..J#Q.4Mr
                                             .&J MwdNM#MMMNMNMMF  .N5N# JY  `.JNdM57J.r
                                               6\.MddNM#MMMjM#MMF  JFk# JC.  .`.````..M5
(c) 2007-2012 Holger Wirtz                    J!,Mdd#MNMMFdM#MM`  MjMFJE`!  `.......`WJ
Licence: GPLv2                                 S`MKNNMMMmMMMMM$   MJM 4jJ.  .`F,.^E .JJ
                                                J?MeMNMMMHHMM^    JdMJJ.l.  ..;J?.F..^J
Contribution (alphabetically):              ..     ."JBWMT"^      JXMN.J.. ..`..&J.?|.$
Czsaba Halasz, Charles Ingels,             MNHHB..                JNMM.+!,.. .#dMWHeF`
Geoff McLane, Yves Sablonier,              .WHbMNdMa.              ?NMN,+.jY`.W#Mb.F
Martin Spott and others.                      ."TMMNKN.             .J5xMM,xui.#.
                                                   ?"""WMNmN&&&gMMMMHM ."".  ?#F
                                                                    ?=       .M^
---------------------------------------------------------------------------------------

This project can be build using CMake 2.8 or later
See : http://www.cmake.org/ for download and install details.
Alternate you can install cmake with macports (http://www.macports.org/)

Dependencies:
### OpenAL - sound library
is part of Core Audio chez Apple and probably already installed on your Mac, 
otherwise please install Xcode/SDKs from http://developer.apple.com

### ALUT.framework
You will need the ALUT.framework in your Library/Frameworks path. You can download a
precompiled ALUT.framework here (thanks to James Turner): 
http://files.goneabitbursar.com/fg/alut-osx-universal.zip

### PLIB - for network code plib net.lib and ul.lib
comes with macports (port install plib +universal) or download latest source 
at http://plib.sourceforge.net/download.html and patch this source with current 
macports patches before you compile.


Building FGCOM for OSX
======================

1. Create an out of source 'build' directory

$ mkdir build-fgcom
$ cd build-fgcom

$ cmake . ../fgcom \
-DCMAKE_BUILD_TYPE=RELEASE

$ sudo make install

Distributors: In case you want to set the positions/frequencies files 
relative to the fgcom binary add -DDEFAULT_POSITIONS_FILE="fgcom-data/positions.txt".
fgcom will find the data dir in Application.app/Contents/MacOS i.e. this way.
This will work also for default install on osx of course (/usr/local).


Running FGCOM
=============

After succesfull install run an echo test with
$ fgcom -f910 --positions=/usr/local/bin/fgcom-data/positions.txt

For an overview of command line options type:
$ fgcom --help

There are two new command line options for positions and frequency file paths:
-T, -positions, --positions=    #location positions file
-Q, -special, --special=        #location special frequencies file (optional)

Here is a guide about how to use FGCOM with FlightGear:
http://wiki.flightgear.org/Fgcom

2012/07/14/ys