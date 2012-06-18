#!/usr/bin/perl
#
# gen_phonebook.pl
# 20120616 - Add strict and warnings
use strict;
use warnings;
use IO::Zlib;
use IO::File;
use File::Slurp;
use Data::Dumper;
use File::Basename;  # split path ($name,$dir,$ext) = fileparse($file [, qr/\.[^.]*/] )

my ($pgmname,$pgmpath) = fileparse($0);

my $VERS = "2012-06-16 0.0.2";
my $verbosity = 0;
my $FG_AIRPORTS = "/usr/local/tmp/apt.dat.gz";
my $FG_NAVAIDS = "/usr/local/tmp/nav.dat.gz";

my $phonebook_post="ZZZZ                        910.000  0190909090910000  Echo-Box\n".
"ZZZZ                        123.450  0190909090123450  Air2Air\n".
"ZZZZ                        122.750  0190909090122750  Air2Air\n";

my %APT = ();
my %NAV = ();
my %CONF = ();

my $use_ws_location = 0; # add airport of av. windsock location, if no tower.
my $use_ctaf_freq = 0;   # add CTAF freq. 127.6 if no others given.
my $overwrite_flag = 1;
my $ctaf_freq = 126.7;

### program variables
my @warnings = ();
my $out_dir = '';

my $conf_extensions = 'extensions.conf';    # base extensions configuration

my $def_phonebook = 'phonebook.txt';
my $def_positions = 'positions.txt';

my $out_phonebook = $def_phonebook;
my $out_positions = $def_positions;

# DEBUG
my $debug_on = 0;
my $def_file = '';

sub VERB1() { return $verbosity >= 1; }
sub VERB2() { return $verbosity >= 2; }
sub VERB5() { return $verbosity >= 5; }
sub VERB9() { return $verbosity >= 9; }

sub prt($) { print shift; }

sub show_warnings($) {
    my ($val) = @_;
    if (@warnings) {
        prt( "\nGot ".scalar @warnings." WARNINGS...\n" );
        foreach my $itm (@warnings) {
           prt("$itm\n");
        }
        prt("\n");
    } else {
        prt( "\nNo warnings issued.\n\n" ) if (VERB9());
    }
}

sub pgm_exit($$) {
    my ($val,$msg) = @_;
    if (length($msg)) {
        $msg .= "\n" if (!($msg =~ /\n$/));
        prt($msg);
    }
    show_warnings($val);
    exit($val);
}


sub prtw($) {
   my ($tx) = shift;
   $tx =~ s/\n$//;
   prt("$tx\n");
   push(@warnings,$tx);
}

##############################################################################
# Read config
##############################################################################
sub read_config()
{
    if ((exists $ENV{'HOME'}) && (-e $ENV{'HOME'}."/.fgreg/fgreg.conf"))
    {
            $CONF{'CONFIG'} = $ENV{'HOME'}."/.fgreg";
            require $CONF{'CONFIG'}."/fgreg.conf";
    }
    elsif(-e "./fgreg.conf")
    {
            $CONF{'CONFIG'}=".";
            require $CONF{'CONFIG'}."/fgreg.conf";
    }
    else
    {
        prt("WARNING: Found no configuration file...\n");
        ### exit(1);
    }
}

sub icao2number($) {
	my ($icao) = @_;
	my ($number,$n,$i);

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

# read airport data in hash
sub read_ap_data() {
    my ($z,$icao,$lat,$lon,$text,$f,$com);
    my %frq = ();
    my ($cnt);
    my $missed = 0;
    my $no_tower = 0;
    my $no_freq = 0;
    my $add_freq = 0;
    my $add_ws_loc = 0;
    my ($ws_cnt,$wslat,$wslon);
    $ws_cnt = 0;
    $wslat = 0;
    $wslon = 0;
    my $had_ap = 0;
    my $ap_cnt = 0;
    my $fh = new IO::Zlib;
    if( $fh->open($FG_AIRPORTS, "r") )
    {
        prt("Reading Airports [$FG_AIRPORTS]\n") if (VERB1());
        while( $z = <$fh> ) {
            chop($z);
            if ( $z =~ /^\s*$/ ) {
                next if (!$had_ap);
                $cnt = scalar(keys(%frq));
                if (($cnt == 0) && $use_ctaf_freq) {
                    $com = sprintf("%3.3f",$ctaf_freq);
                    $frq{$com} = 'CTAF';
                    $cnt = scalar(keys(%frq));
                    $add_freq++;
                }
                if ($use_ws_location && $ws_cnt &&
                    ((! defined $lat)||(! defined $lon))) {
                    # use av. ws location
                    $lat = $wslat / $ws_cnt;
                    $lon = $wslon / $ws_cnt;
                    $add_ws_loc++;
                }

                if($icao && $cnt && $lat && $lon) {
                    $APT{$icao}{'text'}=$text;
                    $APT{$icao}{'lat'}=$lat;
                    $APT{$icao}{'lon'}=$lon;
                    foreach $f (keys(%frq)) {
                        $APT{$icao}{'com'}{$f}=$frq{$f};
                    }
                } else {
                    $missed++;
                    if ($icao) {
                        if (! $cnt) {
                            $no_freq++;
                        }
                        if ((! defined $lat)||( ! defined $lon)) {
                            $no_tower++;
                        }
                    }
                }

                undef($icao);
                undef($text);
                undef($lon);
                undef($lat);
                undef($com);
                $ws_cnt = 0;
                $wslat = 0;
                $wslon = 0;
                $had_ap = 0;
                %frq=();
                next;
            }
            elsif ($z =~ /^1\s+/ )
            {
                if ($z=~/^1\s+-?\d+\s+[01]\s+[01]\s+([A-Z0-9]+)\s+(.+)$/) {
                    # Airport Header
                    $icao=$1;
                    $text=$2;
                    $had_ap = 1;
                    $ap_cnt++;
                } else {
                    prtw("WARNING: a/p REGEX FAILED! [$z] CHECK ME!\n");
                }
            }
            elsif ( $z =~ /^14\s+(-?\d+\.\d+)\s+(-?\d+\.\d+)/ )
            {
                    # TWR Position
                    $lat=$1;
                    $lon=$2;
            }
            elsif ( $z =~ /^5[0-6]\s+(\d{5})\d*\s+(.+)/ )
            {
                # COM data
                $com = sprintf("%3.3f",$1/100);
                $frq{$com} = $2;
            }
            elsif ( $z =~ /^19\s+(-?\d+\.\d+)\s+(-?\d+\.\d+)\s+/ )
            {   
                # 19  08.320709 -073.357287 1 WS
                $wslat += $1;
                $wslon += $2;
                $ws_cnt++;
            }
        }
        $cnt = scalar keys(%APT);
        if (VERB1()) {
            # give some information
            prt("Loaded $cnt of $ap_cnt airport records. ");
            if ($use_ctaf_freq || $use_ws_location) {
                prt("skipped $missed even with ");
                prt("-f ") if ($use_ctaf_freq);
                prt("-w ") if ($use_ws_location);
            } else {
                prt("Skipped $missed ");
            }
            prt("\n");
            if ($no_tower || $no_freq) {
                prt("Skipped ");
                if ($no_tower) {
                    prt("$no_tower with no tower ");
                    if ($use_ws_location) {
                        prt("nor windsock ");
                    } else {
                        prt("- see -w ");
                    }
                }
                if ($no_freq) {
                    prt("$no_freq with no frequency. ");
                    prt("- see -f ") if (!$use_ctaf_freq);
                }
                prt("\n");
            }
        }
    }
    else
    {
        pgm_exit(1,"ERROR: Cannot open $FG_AIRPORTS :$!\n");
    }
    $fh->close;
}

# read nav data in hash
sub read_navaids() {
    my ($z,$lat,$lon,$frq,$code,$text);
    my $nav=new IO::Zlib;
    my ($cnt);
    my $nav_cnt = 0;
    if($nav->open($FG_NAVAIDS, "r")) {
        prt("Reading Navaids [$FG_NAVAIDS]\n") if (VERB1());
        while( $z = <$nav> ) {
            chop($z);
            if( $z =~ /^3\s+(-?\d+\.\d+)\s+(-?\d+\.\d+)\s+\d+\s+(\d+)\s+\d+\s+-?\d+\.\d+\s+([A-Z]+)\s+(.*)\s*$/ )
            {
                # VOR/DME Nav
                $lat = $1;
                $lon = $2;
                $frq = sprintf("%3.3f",$3/100);
                $code = $4;
                $text = $5;

                $NAV{$code}{'lat'}=$lat;
                $NAV{$code}{'lon'}=$lon;
                $NAV{$code}{'frq'}=$frq;
                $NAV{$code}{'text'}=$text;
            } elsif ($z =~ /data\s+cycle/) {
                # this is the Version line
            } elsif ($z =~ /^\d+\s+/) {
                $nav_cnt++;
            }
        }
        $cnt = scalar keys(%NAV);
        prt("Loaded $cnt of $nav_cnt navaid records.\n") if (VERB1());
    }
    else
    {
        pgm_exit(1,"ERROR: Cannot open $FG_NAVAIDS :$!\n");
    }

    close($nav);
}

sub write_files() {
    # open positions file
    my ($positions,$phonebook,$extensions,$extensions_pre);
    my ($airport,$f,$icao_number,$tmp);
    my ($vor,$extensions_post,$i);
    my ($out_conf,$in_conf,$in_post);
    my ($acnt,$ncnt);
    # set file names from %CONF
    $out_conf = $CONF{'ASTERISK_CONFIG_DIR'}."/$conf_extensions";
    $in_conf = $CONF{'CONFIG_DIR'}."/$conf_extensions";
    $in_post = $CONF{'CONFIG'}."/$conf_extensions";

    # read pre data for extensions
    $extensions_pre = read_file($in_conf);
    # read post data for extensions
    $extensions_post = read_file($in_post);

    # open positions phonebook and extensions file
    $positions=new IO::File;
    $positions->open($out_positions, "w")
        || pgm_exit(1, "ERROR: Cannot open [$out_positions for writing: $!\n");
     
    # open phonebook file
    $phonebook=new IO::File;
    $phonebook->open($out_phonebook, "w")
        || pgm_exit(1,"ERROR: Cannot open $out_phonebook for writing: $!\n");

    # open asterisk extensions
    $extensions=new IO::File;
    $extensions->open($out_conf, "w")
        || pgm_exit(1,"ERROR: Cannot open $out_conf for writing: $!\n");

    $acnt = scalar keys(%APT);
    $ncnt = scalar keys(%NAV);
    if (VERB1()) {
        prt("Writing [$out_conf] ");
        prt("[$out_phonebook] and ");
        prt("[$out_positions] ");
        prt(" with $acnt air, $ncnt nav records.\n");
    }
    print $phonebook "ICAO  Decription            FRQ      Phone no.          Name\n";
    print $phonebook "-" x 79,"\n";

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

    # write post data for extensions
    print $extensions $extensions_post;

    # close positions
    $positions->close;

    # close extensions
    $extensions->close;

    # close phonebook
    print $phonebook $phonebook_post;
    $phonebook->close;

    prt("Done files with $acnt air, and $ncnt nav records.\n") if (VERB1());
}

sub check_for_valid_inputs() {
    my $fg_root = (exists $ENV{'FG_ROOT'}) ? $ENV{'FG_ROOT'} : '';
    my $bad = 0;
    my ($nf);
    if (! -f $FG_AIRPORTS) {
        if (length($fg_root)) {
            $nf = "$fg_root/Airports/apt.dat.gz";
            if (-f $nf) {
                $FG_AIRPORTS = $nf;
                prt("Using FG_AIRPORTS file [$FG_AIRPORTS]\n");
            } else {
                $bad++;
                prt("ERROR: Can NOT locate airports file [$FG_AIRPORTS]\n");
                prt("Nor [$nf] based on FG_ROOT! Use -a file\n");
            }
        } else {
            $bad++;
            prt("ERROR: Can NOT locate airports file [$FG_AIRPORTS]! Use -a file\n");
        }
    }
    if (! -f $FG_NAVAIDS) {
        if (length($fg_root)) {
            $nf = "$fg_root/Navaids/nav.dat.gz";
            if (-f $nf) {
                $FG_NAVAIDS = $nf;
                prt("Using FG_NAVAIDS file [$FG_NAVAIDS]\n");
            } else {
                $bad++;
                prt("ERROR: Can NOT locate airports file [$FG_NAVAIDS]\n");
                prt("Nor [$nf] based on FG_ROOT! Use -n file\n");
            }
        } else {
            $bad++;
            prt("ERROR: Can NOT locate navaid file [$FG_NAVAIDS]! Use -n file\n");
        }
    }

    if (! defined $CONF{'CONFIG_DIR'}) {
        $bad++;
        prt("ERROR: NO directory to find PRE $conf_extensions file! Use -e dir\n");
    } else {
        $nf = $CONF{'CONFIG_DIR'};
        if (-d $nf) {
            $nf .= "/$conf_extensions";
            if (! -f $nf) {
                prt("ERROR: PRE file [$nf] does NOT exite! Use -e dir\n");
            }
        } else {
            $bad++;
            prt("ERROR: Directory [$nf], to find PRE $conf_extensions does NOT exite! Use -e dir\n");
        }
    }

    if (! defined $CONF{'CONFIG'}) {
        $bad++;
        prt("ERROR: NO directory to find POST $conf_extensions file! Use -p dir\n");
    } else {
        $nf = $CONF{'CONFIG'};
        if (-d $nf) {
            $nf .= "/$conf_extensions";
            if (! -f $nf) {
                prt("ERROR: POST file [$nf] does NOT exist! Use -p dir\n");
            }
        } else {
            $bad++;
            prt("ERROR: Directory [$nf], to find POST $conf_extensions does NOT exist! Use -p dir\n");
        }
    }

    # $extensions->open($CONF{'ASTERISK_CONFIG_DIR'}."/extensions.conf", "w") || die("Cannot open ".$CONF{'ASTERISK_CONFIG_DIR'}."/extensions.conf for writing: $!\n");
    if (! defined $CONF{'ASTERISK_CONFIG_DIR'}) {
        $bad++;
        prt("ERROR: NO asterisk configuration directory defined, for OUTPUT of $conf_extensions file! Use -c dir\n");
    } elsif (! -d $CONF{'ASTERISK_CONFIG_DIR'}) {
        $nf = $CONF{'ASTERISK_CONFIG_DIR'};
        $bad++;
        prt("ERROR: asterisk configuration directory defined of [$nf],\nfor OUTPUT of $conf_extensions does NOT exist! Use -c dir\n");
    } else {
        $nf = $CONF{'ASTERISK_CONFIG_DIR'}."/$conf_extensions";
        if (-f $nf) {
            if ($overwrite_flag) {
                prt("NOTE: Will NOT overwrite existing [$nf] file!\n") if (VERB2());
            } else {
                $bad++;
                prt("ERROR: Will NOT overwrite existing [$nf] file!\n*** Rename, move or delete first ***\n");
            }
        }
    }

    if (length($out_dir)) {
        $out_positions = "$out_dir/$def_positions";
        $out_phonebook = "$out_dir/$def_phonebook";
    }
    $nf = $out_positions;
    if (-f $nf) {
       if ($overwrite_flag) {
            prt("NOTE: Will overwrite existing [$nf] file!\n") if (VERB2());
        } else {
           $bad++;
            prt("ERROR: Will NOT overwrite existing [$nf] file!\n*** Rename, move or delete first ***\n");
        }
    }
    $nf = $out_phonebook;
    if (-f $nf) {
       if ($overwrite_flag) {
            prt("NOTE: Will overwrite existing [$nf] file!\n") if (VERB2());
        } else {
            $bad++;
            prt("ERROR: Will NOT overwrite existing [$nf] file!\n*** Rename, move or delete first ***\n");
        }
    }

    pgm_exit(1,"ABORTING due to the above problem(s)!\n") if ($bad);;
}

##############################################################################
# Main program
##############################################################################
# read config, if available
read_config();
# parse arguments
parse_args(@ARGV);
# check program can continue
check_for_valid_inputs();
# read apt.dat.gz
read_ap_data();
# read nav.dat.gx
read_navaids();
# write outputs
write_files();
pgm_exit(0,"");

##############################################################################

sub need_arg {
    my ($arg,@av) = @_;
    pgm_exit(1,"ERROR: [$arg] must have a following argument!\n") if (!@av);
}

sub parse_args {
    my (@av) = @_;
    my ($arg,$sarg,$ff);
    while (@av) {
        $arg = $av[0];
        if ($arg =~ /^-/) {
            $sarg = substr($arg,1);
            $sarg = substr($sarg,1) while ($sarg =~ /^-/);
            if (($sarg =~ /^h/i)||($sarg eq '?')) {
                give_help();
                pgm_exit(0,"Help exit(0)");
            } elsif ($sarg =~ /^a/) {
                need_arg(@av);
                shift @av;
                $sarg = $av[0];
                if (-f $sarg) {
                    $FG_AIRPORTS = $sarg;
                    prt("Set input AIRPORTS file to [$FG_AIRPORTS]\n") if (VERB1());
                } else {
                    pgm_exit(1,"ERROR: Can NOT locate AIPRORTS file [$sarg]\n");
                }
            } elsif ($sarg =~ /^n/) {
                need_arg(@av);
                shift @av;
                $sarg = $av[0];
                if (-f $sarg) {
                    $FG_NAVAIDS = $sarg;
                    prt("Set input NAVAID file to [$FG_NAVAIDS]\n") if (VERB1());
                } else {
                    pgm_exit(1,"ERROR: Can NOT locate NAVAID file [$sarg]\n");
                }
            } elsif ($sarg =~ /^e/) {
                need_arg(@av);
                shift @av;
                $sarg = $av[0];
                if (-d $sarg) {
                    $ff = "$sarg/$conf_extensions";
                    if (-f $ff) {
                        $CONF{'CONFIG_DIR'} = $sarg;
                        prt("Set PRE $conf_extensions INPUT dir to [$sarg]\n") if (VERB1());
                    } else {
                        pgm_exit(1,"ERROR: Can NOT locate PRE $conf_extensions file in directory [$sarg]\n");
                    }
                } else {
                    pgm_exit(1,"ERROR: Can NOT locate PRE $conf_extensions directory [$sarg]\n");
                }
            } elsif ($sarg =~ /^p/) {
                need_arg(@av);
                shift @av;
                $sarg = $av[0];
                if (-d $sarg) {
                    $ff = "$sarg/$conf_extensions";
                    if (-f $ff) {
                        $CONF{'CONFIG'} = $sarg;
                        prt("Set POST $conf_extensions INPUT dir to [$sarg]\n") if (VERB1());
                    } else {
                        pgm_exit(1,"ERROR: Can NOT locate POST $conf_extensions file in directory [$sarg]\n");
                    }
                } else {
                    pgm_exit(1,"ERROR: Can NOT locate POST $conf_extensions directory [$sarg]\n");
                }
            } elsif ($sarg =~ /^c/) {
                need_arg(@av);
                shift @av;
                $sarg = $av[0];
                if (-d $sarg) {
                    $CONF{'ASTERISK_CONFIG_DIR'} = $sarg;
                    prt("Set asterisk $conf_extensions output dir to [$sarg]\n") if (VERB1());
                } else {
                    pgm_exit(1,"ERROR: Can NOT locate output $conf_extensions directory [$sarg]\n");
                }
            } elsif ($sarg =~ /^w/) {
                $use_ws_location = 1;
                prt("Enable adding airport of av. windsock location, if no tower.\n") if (VERB1());
            } elsif ($sarg =~ /^f/) {
                $use_ctaf_freq = 1;
                prt("Enable adding airport using CTAF 126.7 if no other frequencies given. ($use_ctaf_freq)\n") if (VERB1());
            } elsif ($sarg =~ /^v/) {
                if ($sarg =~ /^v.*(\d+)$/) {
                    $verbosity = $1;
                } else {
                    while ($sarg =~ /^v/) {
                        $verbosity++;
                        $sarg = substr($sarg,1);
                    }
                }
                prt("Verbosity = $verbosity\n") if (VERB1());
            } elsif ($sarg =~ /^o/) {
                need_arg(@av);
                shift @av;
                $sarg = $av[0];
                if (-d $sarg) {
                    $out_dir = $sarg;
                    prt("Set OUTPUT directory to [$out_dir].\n") if (VERB1());
                } else {
                    pgm_exit(1,"ERROR: OUTPUT directory [$sarg] does NOT exist!.\n") if (VERB1());
                }
            } else {
                pgm_exit(1,"ERROR: Invalid argument [$arg]! Try -?\n");
            }
        } else {
            pgm_exit(1,"ERROR: Invalid argument [$arg]! Try -?\n");
        }
        shift @av;
    }
}

sub give_help {
    prt("$pgmname: version $VERS\n");
    prt("Usage: $pgmname [options]\n");
    prt("Options:\n");
    prt(" --help  (-h or -?) = This help, and exit 0.\n");
    prt(" --air <file>  (-a) = Set AIRPORTS dat.gz INPUT file.\n");
    prt(" --nav <file>  (-n) = Set NAVAID dat.gz INTPUT file.\n");
    prt(" --ext <dir>   (-e) = Set PRE $conf_extensions INPUT directory.\n");
    prt(" --post <dir>  (-p) = Set POST $conf_extensions INPUT directory.\n");
    prt(" --conf <dir>  (-c) = Set $conf_extensions OUTPUT directory for asterisk to read.\n");
    prt(" --out <dir>   (-o) = Set OUTPUT directory for generated text files.\n");
    prt(" --verb[n]     (-v) = Bump [or set] verbosity. def=$verbosity\n");
    prt(" --ws          (-w) = Enable adding an airport on the av. windsock locations if no tower position.\n");
    prt(" --freq        (-f) = Enable adding an airport using CTAF 127.6, if no other frequencies given.\n");
}

# eof - gen_phonebook.pl
