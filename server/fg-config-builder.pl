#!/usr/bin/perl
#
# fg-config-builder.pl
#
# Version 1,.0
#
# (c) H. Wirtz <wirtz@midips.b.shuttle.de>
#

use File::Slurp;
use IO::File::flock;

##############################################################################
# Global System Configuration Vars
##############################################################################
$SYSTEM{'ASTERISK'}="/usr/sbin/asterisk";
$CONF{'FS_SIM_CODE'}="01";

##############################################################################
# Read config
##############################################################################
if(-e $ENV{'HOME'}."/.fgreg/fgreg.conf")
{
	$CONF{'CONFIG'}=$ENV{'HOME'}."/.fgreg";
        require $CONF{'CONFIG'}."/fgreg.conf";
        require $CONF{'CONFIG'}."/auth.conf";
}
elsif(-e "./fgreg.conf")
{
	$CONF{'CONFIG'}=".";
        require $CONF{'CONFIG'}."/fgreg.conf";
        require $CONF{'CONFIG'}."/auth.conf";
}
else
{
        print "Found no configuration file...\n";
        exit(10);
}

##############################################################################
# Main 
##############################################################################
print "fg-config-builder.pl\n";
print "(c) H. Wirtz <wirtz\@parasitstudio.de>\n\n";

# Init configs
_create_config("sip");
_create_config("iax");

sub _asterisk
{
	my($command)=@_;
	my($ret);

	if(-e $SYSTEM{'ASTERISK'} && -x $SYSTEM{'ASTERISK'})
	{
		open(ASTERISK,$SYSTEM{'ASTERISK'}." -rx \"$command\"|")||print "Cannot open pipe ".$SYSTEM{'ASTERISK'}." [$command]\n";
		while(<ASTERISK>)
		{
			next if(/^Verbosity/i);
			$ret.=$_;
		}
		close(ASTERISK);

		return($ret);
	}
	return(-1);
}

sub _create_config
{
	my($protocol)=@_;
	my($conf,$c,$u);

	# open pre file
	$pre=read_file($CONF{'CONFIG_DIR'}."/".$protocol.".conf");

	foreach $u (keys(%ACCESS))
	{
		$c.="[".$u."]\n";
		$c.="type=friend\n";
		$c.="username=".$u."\n";
		$c.="secret=".$main::ACCESS{$u}."\n";
		$c.="context=default\n";
		$c.="host=dynamic\n";
		$c.="nat=yes\n";
		$c.="notransfer=yes\n";
		$c.="regcontext=default\n";
		$c.="\n";
	}

	# open post file
	$post=read_file($CONF{'CONFIG'}."/".$protocol.".conf");

	# open and write config file
	$conf=new IO::File::flock($CONF{'ASTERISK_CONFIG_DIR'}."/".$protocol.".conf","w",'lock_ex',$CONF{'LOCK_TIMEOUT'})||print "Cannot open ".$CONF{'ASTERISK_CONFIG_DIR'}."/".$protocol.".conf: $!\n";
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

