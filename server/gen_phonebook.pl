#!/usr/bin/perl
#
# gen_phonebook.pl
#

$FG_AIRPORTS="/usr/local/tmp/apt.dat.gz";
$FG_NAVAIDS="/usr/local/tmp/nav.dat.gz";

$phonebook_post="ZZZZ                        910.000  0190909090910000  Echo-Box
ZZZZ                        123.450  0190909090123450  Air2Air
ZZZZ                        122.750  0190909090122750  Air2Air
";

use IO::Zlib;
use IO::File;
use File::Slurp;
use Data::Dumper;

##############################################################################
# Read config
##############################################################################
if(-e $ENV{'HOME'}."/.fgreg/fgreg.conf")
{
        $CONF{'CONFIG'}=$ENV{'HOME'}."/.fgreg";
        require $CONF{'CONFIG'}."/fgreg.conf";
}
elsif(-e "./fgreg.conf")
{
        $CONF{'CONFIG'}=".";
        require $CONF{'CONFIG'}."/fgreg.conf";
}
else
{
	print "Found no configuration file...\n";
	exit(10);
}

##############################################################################
# Main program
##############################################################################

# read airport data in hash
$fh=new IO::Zlib;
if($fh->open($FG_AIRPORTS, "r"))
{
	while($z=<$fh>)
	{
		chop($z);

                if($z=~/^\s*$/)
		{
                	if($icao && scalar(keys(%frq))>0 && $lat && $lon)
			{
				$APT{$icao}{'text'}=$text;
				$APT{$icao}{'lat'}=$lat;
				$APT{$icao}{'lon'}=$lon;
				foreach $f (keys(%frq))
				{
					$APT{$icao}{'com'}{$f}=$frq{$f};
				}
			}

			undef($icao);
			undef($text);
			undef($lon);
			undef($lat);
			undef($com);
			%frq=();
			next;
		}
		elsif($z=~/^1\s+\d+\s+[01]\s+[01]\s+([A-Z0-9]+)\s+(.+)$/)
		{
			# Airport Header
			$icao=$1;
			$text=$2;
		}
                elsif($z=~/^14\s+(-?\d+\.\d+)\s+(-?\d+\.\d+)/)
                {
                        # TWR Position
                        $lat=$1;
                        $lon=$2;
                }
		elsif($z=~/^5[0-6]\s+(\d{5})\d*\s+(.+)/)
		{
			# COM data
			$com=sprintf("%3.3f",$1/100);

			$frq{$com}=$2;
		}
	}
}
else
{
	die("Cannot open $FG_AIRPORTS :$!\n");
}
$fh->close;

# read nav data in hash
$nav=new IO::Zlib;
if($nav->open($FG_NAVAIDS, "r"))
{
	while($z=<$nav>)
	{
		chop($z);

                if($z=~/^3\s+(-?\d+\.\d+)\s+(-?\d+\.\d+)\s+\d+\s+(\d+)\s+\d+\s+-?\d+\.\d+\s+([A-Z]+)\s+(.*)\s*$/)
		{
			# VOR/DME Nav
			$lat=$1;
			$lon=$2;
			$frq=sprintf("%3.3f",$3/100);
			$code=$4;
			$text=$5;

			$NAV{$code}{'lat'}=$lat;
			$NAV{$code}{'lon'}=$lon;
			$NAV{$code}{'frq'}=$frq;
			$NAV{$code}{'text'}=$text;
		}
	}
}
else
{
	die("Cannot open $FG_NAVAIDS :$!\n");
}

close($nav);

# open positions file
$positions=new IO::File;
$positions->open("positions.txt", "w") || die("Cannot open positions.txt for writing: $!\n");
 
# open phonebook file
$phonebook=new IO::File;
$phonebook->open("phonebook.txt", "w") || die("Cannot open phonebook.txt for writing: $!\n");
print $phonebook "ICAO  Decription            FRQ      Phone no.          Name\n";
print $phonebook "-"x79,"\n";

# open asterisk extensions
$extensions=new IO::File;
$extensions->open($CONF{'ASTERISK_CONFIG_DIR'}."/extensions.conf", "w") || die("Cannot open ".$CONF{'ASTERISK_CONFIG_DIR'}."/extensions.conf for writing: $!\n");

# read pre data for extensions
$extensions_pre=read_file($CONF{'CONFIG_DIR'}."/extensions.conf");
print $extensions $extensions_pre;

# Print all known airports
foreach $airport (sort(keys(%APT)))
{
	foreach $f (keys(%{$APT{$airport}{'com'}}))
	{
		$icao_number=icao2number($airport);

                # write positions APT
                print $positions $airport.",".$f.",".$APT{$airport}{'lat'}.",".$APT{$airport}{'lon'}.",".$APT{$airport}{'com'}{$f}.",".$APT{$airport}{'text'}."\n";
                
		# write phonebook
		$tmp=sprintf("%4s  %-20s  %3.3f  %-.16s  %-20s\n",$airport,$APT{$airport}{'com'}{$f},$f,"01".$icao_number.$f*1000,$APT{$airport}{'text'});
		print $phonebook $tmp;

		# write extensions.conf
		print $extensions "; $airport $APT{$airport}{'com'}{$f} $f - $APT{$airport}{'text'}\n";
		if($APT{$airport}{'com'}{$f}=~/ATIS$/)
		{
			# ATIS playback extension
                	print $extensions "exten => 01$icao_number".($f*1000).",1,SendText($airport ".$APT{$airport}{'text'}." $f ".$APT{$airport}{'com'}{$f}.")\n";
                	print $extensions "exten => 01$icao_number".($f*1000).",n,SendURL(http://www.the-airport-guide.com/search.php?by=icao&search=$airport)\n";
                	print $extensions "exten => 01$icao_number".($f*1000).",n,Macro(com)\n";

			# ATIS record extension
                	print $extensions "exten => 99$icao_number".($f*1000).",1,SendText($airport ".$APT{$airport}{'text'}." $f Record-".$APT{$airport}{'com'}{$f}.")\n";
                	print $extensions "exten => 99$icao_number".($f*1000).",n,Macro(com)\n";
		}
		else
		{
                	print $extensions "exten => 01$icao_number".($f*1000).",1,SendText($airport ".$APT{$airport}{'text'}." $f ".$APT{$airport}{'com'}{$f}.")\n";
                	print $extensions "exten => 01$icao_number".($f*1000).",n,SendURL(http://www.the-airport-guide.com/search.php?by=icao&search=$airport)\n";
                	print $extensions "exten => 01$icao_number".($f*1000).",n,Macro(com)\n";

		}

                print $extensions ";\n";

	}
}

# write VORs to extensions.conf
foreach $vor (sort(keys(%NAV)))
{
	$icao_number=icao2number($vor);

        # write positions NAV
        print $positions $vor.",".$NAV{$vor}{'frq'}.",".$NAV{$vor}{'lat'}.",".$NAV{$vor}{'lon'}.",VOR,".$NAV{$vor}{'text'}."\n";
                
	# write phonebook
	$tmp=sprintf("%4s  %-20s  %3.3f  %-.16s  %-20s\n",$vor,"",$NAV{$vor}{'frq'},"01".$icao_number.$NAV{$vor}{'frq'}*1000,$NAV{$vor}{'text'});
	print $phonebook $tmp;

	# write extensions.conf
	print $extensions "; VOR $vor $NAV{$vor}{'frq'} - $NAV{$vor}{'text'}\n";
       	print $extensions "exten => 01$icao_number".($NAV{$vor}{'frq'}*1000).",1,SendText($vor ".$NAV{$vor}{'text'}." ".$NAV{$vor}{'frq'}.")\n";
       	print $extensions "exten => 01$icao_number".($NAV{$vor}{'frq'}*1000).",n,Macro(vor,$vor)\n";
}

# read post data for extensions
$extensions_post=read_file($CONF{'CONFIG'}."/extensions.conf");
print $extensions $extensions_post;

# close positions
$positions->close;

# close extensions
$extensions->close;

# close phonebook
print $phonebook $phonebook_post;
$phonebook->close;

sub icao2number
{
	my($icao)=@_;
	my($number,$n);

	$icao=" ".$icao if(length($icao)==3);
	$icao="  ".$icao if(length($icao)==2);
	$icao="   ".$icao if(length($icao)==1);

	for($i=0;$i<length($icao);$i++)
	{
		$n=ord(substr($icao,$i,1));
                $number.=sprintf("%02d",$n);
	}
	return($number);
}
