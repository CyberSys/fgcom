#!/usr/bin/perl
#
# fg-config-builder.pl
#
# Version 1,.0
# (c) H. Wirtz <wirtz@midips.b.shuttle.de>
# Version 1.0.1 - Geoff R. McLane 20120616
#
use File::Slurp;
use IO::File::flock;
use strict;
use warnings;
use File::Basename;  # split path ($name,$dir,$ext) = fileparse($file [, qr/\.[^.]*/] )
use Time::gmtime;

my $VERS = "1.0.2 2012-06-22";
my ($pgmname,$pgmpath) = fileparse($0);
my $verbosity = 0;

my $allow_no_pre = 0;
my $allow_no_post = 0;  # do not require a POST input file
my $allow_no_fgreg;
### program variables
my @warnings = ();
my %SYSTEM = ();
my %CONF = ();
my %ACCESS = ();

my @protocols = qw(sip iax);
my $in_files = "sip.conf iax.conf";

sub VERB1() { return $verbosity >= 1; }
sub VERB2() { return $verbosity >= 2; }
sub VERB5() { return $verbosity >= 5; }
sub VERB9() { return $verbosity >= 9; }

# debug only 
my $debug_on = 0;
my $def_auth = 'C:\FG\16\build-fgcom\asterisk\auth.conf';
my $def_pre = 'C:\FG\16\fgcom\server\config';
my $def_post = '';
my $def_conf_out = 'C:\FG\16\build-fgcom\asterisk\temp';

sub prt($) { print shift; }

sub prt_header() {
    prt("$pgmname - generate sip.conf and iax.conf for asterisk reload.\n");
    prt(" Original: (c) H. Wirtz <wirtz\@parasitstudio.de>\n");
    prt(" Revision: $VERS Geoff R. McLane\n");
    prt("Script MUST be run with 'correct' asterisk reload priviledges.\n\n");
}

sub lu_get_YYYYMMDD_hhmmss_UTC($) {
    my ($t) = shift;
    # sec, min, hour, mday, mon, year, wday, yday, and isdst.
    my $tm = gmtime($t);
    my $m = sprintf( "%04d/%02d/%02d %02d:%02d:%02d",
        $tm->year() + 1900, $tm->mon() + 1, $tm->mday(), $tm->hour(), $tm->min(), $tm->sec());
    return $m;
}

##############################################################################
# Read authorisations
##############################################################################
sub read_auth_file($) {
    my $fil = shift;
    if (open INF, "<$fil") {
        my @arr = <INF>;
        close INF;
        my ($line,$name,$val);
        foreach $line (@arr) {
            chomp $line;
            next if ($line =~ /^\s*$/);
            next if ($line =~ /\s*\#/);
            if ($line =~ /^\$ACCESS\{['"]{1}(.+)["']{1}\}\s*=\s*['"]{1}(.+)["']{1}/) {
                $name = $1;
                $val  = $2;
                prt("$name $val\n") if (VERB9());
                $ACCESS{$name} = $val;
            } else {
                prt("WARNING: Unknown line in 'auth' file [$line]!\n");
            }
        }
    }
}

##############################################################################
# Read config
##############################################################################
sub read_config()
{
    my ($infil,@arr,$line,$conf,$val,$bad,$try);
    # user 'strict' must READ config file, and set %CONF accordingly
    # using require like this does NOT work.
    $bad = 1;
    $infil = '';
    $try = 0;
    if ((exists $ENV{'HOME'}) && (-e $ENV{'HOME'}."/.fgreg/fgreg.conf"))
    {
            $CONF{'CONFIG'} = $ENV{'HOME'}."/.fgreg";
            # require $CONF{'CONFIG'}."/fgreg.conf";
            $infil = $CONF{'CONFIG'}."/fgreg.conf";
            $try = 1;
    }
    elsif(-e "./fgreg.conf")
    {
            $CONF{'CONFIG'}=".";
            #require $CONF{'CONFIG'}."/fgreg.conf";
            $infil = $CONF{'CONFIG'}."/fgreg.conf";
            $try = 1;
    }
    if ($try) {
        if (open INF, "<$infil") {
            @arr = <INF>;
            close INF;
            foreach $line (@arr) {
                chomp $line;
                if ($line =~ /^\$CONF\{['"]{1}(.+)["']{1}\}\s*=\s*['"]{1}(.+)["']{1}/) {
                    $conf = $1;
                    $val = $2;
                    $CONF{$conf} = $val;
                    $bad = 0;
                }
            }
        }
    }
    if ($bad) {
        prt("WARNING: Found no configuration file...\n") if (VERB5());
        ### exit(1);
    }
}

##############################################################################
# Global System Configuration Vars
##############################################################################
$SYSTEM{'ASTERISK'} = "/usr/sbin/asterisk";
$CONF{'FS_SIM_CODE'} = "01";
$CONF{'LOCK_TIMEOUT'} = 10;


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


sub _asterisk($) {
	my ($command) = @_;
	my ($ret);
    $ret = "WARING: asterisk runtime NOT found!\n";
	if((defined $SYSTEM{'ASTERISK'}) && (-e $SYSTEM{'ASTERISK'}) && (-x $SYSTEM{'ASTERISK'}))
	{
        $ret = '';
		open(ASTERISK,$SYSTEM{'ASTERISK'}." -rx \"$command\"|")
            ||print "Cannot open pipe ".$SYSTEM{'ASTERISK'}." [$command]\n";
		while(<ASTERISK>)
		{
			next if(/^Verbosity/i);
			$ret .= $_;
		}
		close(ASTERISK);

	}
	return ($ret);
}

sub _create_config($) {
	my ($protocol) = @_;
	my ($conf,$c,$u,$pre,$post,$file);
    my ($out_file,$msg,$cnt);

    $out_file = $CONF{'ASTERISK_CONFIG_DIR'}."/".$protocol.".conf";
    $msg = '';
    $cnt = 0;
	foreach $u (keys(%ACCESS)) {
        $msg .= ' ' if (length($msg));
        $msg .= $u;
        $cnt++;
    }
    prt("Writting [$out_file], adding $cnt users [$msg]\n") if (VERB1());

	# open pre file
    if ($allow_no_pre) {
        $pre = "; due to allow_no_pre, PRE data not required.\n";
        if (defined $CONF{'CONFIG_DIR'}) {
            $file = $CONF{'CONFIG_DIR'};
            $file .= "/".$protocol.".conf";
            if (-f $file) {
                $pre = read_file($file);
            }
        }
    } else {
        # has been validated, so read it
    	$pre = read_file($CONF{'CONFIG_DIR'}."/".$protocol.".conf"); # PRE file
    }

    $c .= "\n; generated by $pgmname, on ";
    $c .= lu_get_YYYYMMDD_hhmmss_UTC(time());
    $c .= " UTC, adding $cnt users.\n\n";

	foreach $u (keys(%ACCESS)) {
		$c .= "[".$u."]\n";
		$c .= "type=friend\n";
		$c .= "username=".$u."\n";
		$c .= "secret=".$ACCESS{$u}."\n";
		$c .= "context=default\n";
		$c .= "host=dynamic\n";
		$c .= "nat=yes\n";
		$c .= "notransfer=yes\n";
		$c .= "regcontext=default\n";
		$c .="\n";
	}

	# open post file
    if ($allow_no_post) {
        $post = "; due to allow_no_post, POST data not required.\n";
        if (defined $CONF{'CONFIG'}) {
            $file = $CONF{'CONFIG'};
            $file .= "/".$protocol.".conf";
            if (-f $file) {
                $post = read_file($file);
            }
        }
    } else {
    	$post = read_file($CONF{'CONFIG'}."/".$protocol.".conf");
    }

	# open and write config file
	$conf = new IO::File::flock($out_file,"w",'lock_ex',$CONF{'LOCK_TIMEOUT'})
        || die "ERROR: Cannot open $out_file! $!\n";

	print $conf $pre;
	print $conf $c;
	print $conf $post;

	# close config file
	undef($conf);

	# reload config
	$protocol="iax2" if($protocol eq "iax");
	print _asterisk($protocol." reload");

	return(0);
}

sub validate_input() {
    # check we can continue, at least partially...
    my $bad = 0;
    my ($dir,$proto,$ff,$cnt,$msg,$have,$errs,@arr);
    $msg = 'Use -p dir, or -d POST';
    $have = '';
    $errs = '';
    if (!$allow_no_post) {
        if (! defined $CONF{'CONFIG'}) {
            $bad++;
            $errs .= "ERROR: No POST configuration directory given, to find INPUT $in_files! $msg\n";
        } else {
            $dir = $CONF{'CONFIG'};
            $have .= "Have POST INPUT directory [$dir]\n";
            if (-d $dir) {
                foreach $proto (@protocols) {
                    $ff = $dir."/$proto.conf";
                    if (-f $ff) {
                        $have .= "Have POST INPUT file [$ff]\n";
                    } else {
                        $bad++;
                        $errs .= "ERROR: POST configuration file [$ff] NOT FOUND! $msg\n";
                    }
                }
            } else {
                $bad++;
                $errs .= "ERROR: POST configuration directory [$dir] NOT VALID! $msg\n";
            }
        }
    }

    $msg = 'Use -e dir, or -d PRE';
    if (!$allow_no_pre) {
        if (! defined $CONF{'CONFIG_DIR'}) {
            $bad++;
            $errs .= "ERROR: No PRE configuration directory given, to find $in_files! $msg\n";
        } else {
            $dir = $CONF{'CONFIG_DIR'};
            $have .= "Have PRE INPUT directory [$dir]\n";
            if (-d $dir) {
                foreach $proto (@protocols) {
                    $ff = $dir."/$proto.conf";
                    if (-f $ff) {
                        $have .= "Found PRE INPUT file [$ff]\n";
                    } else {
                        $bad++;
                        $errs .= "ERROR: PRE configuration file [$ff] NOT FOUND! $msg\n";
                    }
                }
            } else {
                $bad++;
                $errs .= "ERROR: PRE configuration directory [$dir] NOT VALID! $msg\n";
            }
        }
    }

    $msg = "Use -a file to load 'auth' file.";
    $cnt = scalar keys(%ACCESS);
    if ($cnt == 0) {
        $bad++;
        $errs .= "ERROR: No ACCESS keys defined! $msg\n";
    } else {
        $have .= "Have $cnt ACCESS authorization keys.\n";
    }

    $msg = "Use -c dir, to set output directory.";
    if (defined $CONF{'ASTERISK_CONFIG_DIR'}) {
        $dir = $CONF{'ASTERISK_CONFIG_DIR'};
        @arr = glob($dir."/*.conf");
        $cnt = scalar @arr;
        $have .= "Have OUTPUT directory [$dir], for asterisk to reload";
        if ($cnt) {
            $have .= " with $cnt other 'conf' files";
        }
        $have .= "\n";
    } else {
        $bad++;
        $errs .= "ERROR: No ASTERISK configuration install directory given! $msg\n";
    }

    pgm_exit(1,"${have}${errs}ERROR ABORT due to the above $bad error(s)!\n") if ($bad);

}

sub do_test() {
    # what happens on reading a no existant file?
    my $fil = "aix.conf";
    prt("Attempting to read file [$fil] ");
    if (! -f $fil) {
        prt("which does not exists!");
    }
    prt("\n");
    my $txt = read_file($fil);
    pgm_exit(1,"Test exit\n");
    # simple, it ABORTS the script with 
    # read_file 'aix.conf' - sysopen: No such file or directory at fg-config-builder.pl line 224
}

##############################################################################
# Main 
##############################################################################
prt_header();

read_config(); # seems this MUST be at a global scope
if ((defined $CONF{'CONFIG'}) && (-f $CONF{'CONFIG'}."/auth.conf")) {
    read_auth_file($CONF{'CONFIG'}."/auth.conf");
}

parse_args(@ARGV);
### do_test();
validate_input();

# Init configs
_create_config("sip");
_create_config("iax");

##############################################################################

sub need_arg {
    my ($arg,@av) = @_;
    pgm_exit(1,"ERROR: [$arg] must have a following argument!\n") if (!@av);
}

sub parse_args {
    my (@av) = @_;
    my ($arg,$sarg,$ff,$ff2,$ff1,$cnt,$msg);
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
                    read_auth_file($sarg);
                } else {
                    pgm_exit(1,"ERROR: Can NOT locate ACCESS authoration file [$sarg]!\n");
                }
            } elsif ($sarg =~ /^d/) {
                need_arg(@av);
                shift @av;
                $sarg = $av[0];
                $msg = "Set to allow NO [$sarg]";
                if ($sarg eq 'PRE') {
                    $allow_no_pre = 1;

                } elsif ($sarg eq 'POST') {
                    $allow_no_post = 1;  # do not require a POST input file
                } else {
                    pgm_exit(1,"ERROR: Argument [$arg] can only be followed by PRE or POST, not [$sarg]\n");
                }
                prt("$msg\n") if (VERB1());
            } elsif ($sarg =~ /^e/) {
                need_arg(@av);
                shift @av;
                $sarg = $av[0];
                if (-d $sarg) {
                    $ff1 = "$sarg/sip.conf";
                    $ff2 = "$sarg/iax.conf";
                    if ((-f $ff1)&&(-f $ff2)) {
                        $CONF{'CONFIG_DIR'} = $sarg;
                        prt("Set PRE $in_files INPUT dir to [$sarg]\n") if (VERB1());
                    } else {
                        $msg = '';
                        if (! -f $ff1) {
                            $msg .= "[$ff1]";
                        }
                        if (! -f $ff2) {
                            $msg .= " and\n" if (length($msg));
                            $msg .= "[$ff2]";
                        }
                        pgm_exit(1,"ERROR: NOT locate PRE $msg file in directory [$sarg]\n");
                    }
                } else {
                    pgm_exit(1,"ERROR: Can NOT locate PRE $in_files directory [$sarg]\n");
                }
            } elsif ($sarg =~ /^p/) {
                need_arg(@av);
                shift @av;
                $sarg = $av[0];
                if (-d $sarg) {
                    $ff1 = "$sarg/sip.conf";
                    $ff2 = "$sarg/iax.conf";
                    if ((-f $ff1)&&(-f $ff2)) {
                        $CONF{'CONFIG'} = $sarg;
                        prt("Set POST $in_files INPUT dir to [$sarg]\n") if (VERB1());
                    } else {
                        $msg = '';
                        if (! -f $ff1) {
                            $msg .= "[$ff1]";
                        }
                        if (! -f $ff2) {
                            $msg .= " and\n" if (length($msg));
                            $msg .= "[$ff2]";
                        }
                        pgm_exit(1,"ERROR: Can NOT locate POST $msg file in directory [$sarg]\n");
                    }
                } else {

                    pgm_exit(1,"ERROR: Can NOT locate POST $in_files directory [$sarg]\n");
                }
            } elsif ($sarg =~ /^c/) {
                need_arg(@av);
                shift @av;
                $sarg = $av[0];
                if (-d $sarg) {
                    $CONF{'ASTERISK_CONFIG_DIR'} = $sarg;
                    prt("Set asterisk config OUTPUT output dir to [$sarg]\n") if (VERB1());
                } else {
                    pgm_exit(1,"ERROR: Can NOT locate $in_files OUTPUT directory [$sarg]\n");
                }
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
            } else {
                pgm_exit(1,"ERROR: Invalid argument [$arg]! Try -?\n");
            }
        } else {
            pgm_exit(1,"ERROR: Invalid argument [$arg]! Try -?\n");
        }
        shift @av;
    }
    if ($debug_on) {
        # OOPS has to be doen in global scope
        #    $cnt = scalar keys(%ACCESS);
        #    if (($cnt == 0) && (-f $def_auth)) {
        #        require $def_auth;
        #        prt("Loaded DEFAULT file [$def_auth]\n");
        #    }
        if ((! defined $CONF{'CONFIG_DIR'}) && (-d $def_pre)) {
            $CONF{'CONFIG_DIR'} = $def_pre;
            prt("[DEBUG]: Set DEFAULT PRE dir [$def_pre]\n");
        }
        if ((! defined $CONF{'ASTERISK_CONFIG_DIR'}) && (-d $def_conf_out)) {
            $CONF{'ASTERISK_CONFIG_DIR'} = $def_conf_out;
            prt("[DEBUG]: Set DEFAULT config OUTPUT dir [$def_conf_out]\n");
        }
        if (!VERB1()) {
            $verbosity = 1;
        }
    }

}

sub give_help {
    prt("$pgmname: version $VERS\n");
    prt("Usage: $pgmname [options]\n");
    prt("Options:\n");
    prt(" --help   (-h or -?) = This help, and exit 0.\n");
    prt(" --auth <file>  (-a) = Set the 'authorisation' ACCESS file.\n");
    prt(" --conf <dir>   (-c) = Set $in_files OUTPUT directory for asterisk to read.\n");
    prt(" --dis PRE|POST (-d) = Disable the need for PRE or POST files.\n");
    prt(" --ext <dir>    (-e) = Set PRE INPUT directory, for $in_files.\n");
    prt(" --post <dir>   (-p) = Set POST INPUT directory, for $in_files.\n");
    prt(" --verb[n]      (-v) = Bump [or set] verbosity. def=$verbosity\n");
}

# eof - fg-config-builder.pl
