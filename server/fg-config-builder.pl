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
#use strict;
use warnings;
use File::Basename;  # split path ($name,$dir,$ext) = fileparse($file [, qr/\.[^.]*/] )

my $VERS = "1.0.1 2012-06-16";
my ($pgmname,$pgmpath) = fileparse($0);
my $verbosity = 0;

my $allow_no_pre = 0;
my $allow_no_post = 0;  # do not require a POST input file
my $allow_no_fgreg;
### program variables
my @warnings = ();
my %SYSTEM = ();
my %CONF = ();
my @protocols = qw(sip iax);
my $in_files = "sip.conf iax.conf";

# debug only 
my $debug_on = 0;
my $def_auth = 'C:\FG\16\build-fgcom\asterisk\auth.conf';
my $def_pre = 'C:\FG\16\fgcom\server\config';
my $def_post = '';
my $def_conf_out = 'C:\FG\16\build-fgcom\asterisk\temp';

sub prt($) { print shift; }

##############################################################################
# Read config
##############################################################################
# oops, can NOT be a sub
my ($fgreg);
#sub read_config()
#{
    if((exists $ENV{'HOME'}) && (-e $ENV{'HOME'}."/.fgreg/fgreg.conf"))
    {
        $CONF{'CONFIG'} = $ENV{'HOME'}."/.fgreg";
        if ($allow_no_fgreg) {
            $fgreg = $CONF{'CONFIG'}."/fgreg.conf";
            if (-f $fgreg) {
                require $fgreg;
            }
        } else {
            require $CONF{'CONFIG'}."/fgreg.conf";
        }
        require $CONF{'CONFIG'}."/auth.conf";
    }
    elsif(-e "./fgreg.conf")
    {
        $CONF{'CONFIG'}=".";
        if ($allow_no_fgreg) {
            $fgreg = $CONF{'CONFIG'}."/fgreg.conf";
            if (-f $fgreg) {
                require $fgreg;
            }
        } else {
            require $CONF{'CONFIG'}."/fgreg.conf";
        }
        require $CONF{'CONFIG'}."/auth.conf";
    }
    else
    {
        print "WARNING: Found no configuration file...\n";
        ###exit(10);
        if ($debug_on) {
            # my $cnt = scalar keys(%ACCESS);
            #if (($cnt == 0) && (-f $def_auth)) {
            if (-f $def_auth) {
                require $def_auth;
                prt("[DEBUG] Loaded DEFAULT file [$def_auth]\n");
            }
        } else {
            my %ACCESS = ();
        }
    }
#}

##############################################################################
# Global System Configuration Vars
##############################################################################
$SYSTEM{'ASTERISK'} = "/usr/sbin/asterisk";
$CONF{'FS_SIM_CODE'} = "01";
$CONF{'LOCK_TIMEOUT'} = 10;

sub VERB1() { return $verbosity >= 1; }
sub VERB2() { return $verbosity >= 2; }
sub VERB5() { return $verbosity >= 5; }
sub VERB9() { return $verbosity >= 9; }


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
    my ($out_file,$msg);

    $out_file = $CONF{'ASTERISK_CONFIG_DIR'}."/".$protocol.".conf";
    $msg = '';
	foreach $u (keys(%ACCESS)) {
        $msg .= ' ' if (length($msg));
        $msg .= $u;
    }
    prt("Writting [$out_file], adding users [$msg]\n") if (VERB1());

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

	foreach $u (keys(%ACCESS))
	{
		$c.="[".$u."]\n";
		$c.="type=friend\n";
		$c.="username=".$u."\n";
		$c.="secret=".$ACCESS{$u}."\n";
		$c.="context=default\n";
		$c.="host=dynamic\n";
		$c.="nat=yes\n";
		$c.="notransfer=yes\n";
		$c.="regcontext=default\n";
		$c.="\n";
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
	$conf=new IO::File::flock($out_file,"w",'lock_ex',$CONF{'LOCK_TIMEOUT'})||print "Cannot open ".$CONF{'ASTERISK_CONFIG_DIR'}."/".$protocol.".conf: $!\n";
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
    my ($dir,$proto,$ff,$cnt,$msg);
    $msg = 'Use -p dir, or -d POST';
    if (!$allow_no_post) {
        if (! defined $CONF{'CONFIG'}) {
            $bad++;
            prt("ERROR: No POST configuration directory given, to find INPUT $in_files! $msg\n");
        } else {
            $dir = $CONF{'CONFIG'};
            if (-d $dir) {
                foreach $proto (@protocols) {
                    $ff = $dir."/$proto.conf";
                    if (! -f $ff) {
                        $bad++;
                        prt("ERROR: POST configuration file [$ff] NOT FOUND! $msg\n");
                    }
                }
            } else {
                $bad++;
                prt("ERROR: POST configuration directory [$dir] NOT VALID! $msg\n");
            }
        }
    }

    $msg = 'Use -e dir, or -d PRE';
    if (!$allow_no_pre) {
        if (! defined $CONF{'CONFIG_DIR'}) {
            $bad++;
            prt("ERROR: No PRE configuration directory given, to find $in_files! $msg\n");
        } else {
            $dir = $CONF{'CONFIG_DIR'};
            if (-d $dir) {
                foreach $proto (@protocols) {
                    $ff = $dir."/$proto.conf";
                    if (! -f $ff) {
                        $bad++;
                        prt("ERROR: PRE configuration file [$ff] NOT FOUND! $msg\n");
                    }
                }
            } else {
                $bad++;
                prt("ERROR: PRE configuration directory [$dir] NOT VALID! $msg\n");
            }
        }
    }

    $cnt = scalar keys(%ACCESS);
    if ($cnt == 0) {
        $bad++;
        prt("ERROR: No ACCESS keys defined! Run in folder where auth.conf can be found.\n");
    }

    if (! defined $CONF{'ASTERISK_CONFIG_DIR'}) {
        $bad++;
        prt("ERROR: No ASTERISK configuration install directory given! Use -c dir\n");
    }

    pgm_exit(1,"Aborting due to the above $bad errors!\n") if ($bad);

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
prt("$pgmname\n");
prt("Original: (c) H. Wirtz <wirtz\@parasitstudio.de>\n");
prt("Revision: $VERS Geoff R. McLane\n\n");

### read_config(); # seems this MUST be at a global scope
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
    my ($arg,$sarg,$ff,$ff2,$cnt);
    while (@av) {
        $arg = $av[0];
        if ($arg =~ /^-/) {
            $sarg = substr($arg,1);
            $sarg = substr($sarg,1) while ($sarg =~ /^-/);
            if (($sarg =~ /^h/i)||($sarg eq '?')) {
                give_help();
                pgm_exit(0,"Help exit(0)");
            } elsif ($sarg =~ /^e/) {
                need_arg(@av);
                shift @av;
                $sarg = $av[0];
                if (-d $sarg) {
                    $ff = "$sarg/sip.conf";
                    $ff2 = "$sarg/aix.conf";
                    if ((-f $ff)&&(-f $ff2)) {
                        $CONF{'CONFIG_DIR'} = $sarg;
                        prt("Set PRE $in_files INPUT dir to [$sarg]\n") if (VERB1());
                    } else {
                        pgm_exit(1,"ERROR: NOT locate PRE $ff or $ff2 file in directory [$sarg]\n");
                    }
                } else {
                    pgm_exit(1,"ERROR: Can NOT locate PRE $in_files directory [$sarg]\n");
                }
            } elsif ($sarg =~ /^p/) {
                need_arg(@av);
                shift @av;
                $sarg = $av[0];
                if (-d $sarg) {
                    $ff = "$sarg/sip.conf";
                    $ff2 = "$sarg/aix.conf";
                    if ((-f $ff)&&(-f $ff2)) {
                        $CONF{'CONFIG'} = $sarg;
                        prt("Set POST $in_files INPUT dir to [$sarg]\n") if (VERB1());
                    } else {
                        pgm_exit(1,"ERROR: Can NOT locate POST $ff or $ff2 file in directory [$sarg]\n");
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
    prt(" --conf <dir>   (-c) = Set $in_files OUTPUT directory for asterisk to read.\n");
    prt(" --dis PRE|POST (-d) = Disable the need for PRE or POST files.\n");
    prt(" --ext <dir>    (-e) = Set PRE INPUT directory, for $in_files.\n");
    prt(" --post <dir>   (-p) = Set POST INPUT directory, for $in_files.\n");
    prt(" --verb[n]      (-v) = Bump [or set] verbosity. def=$verbosity\n");
}

# eof - fg-config-builder.pl
