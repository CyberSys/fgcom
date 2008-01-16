#include "fgcom.h"

void
usage (char *prog)
{
  printf
    ("Usage: %s [-u user] -w [password] [[-s voipserver] [-p port]] | [[-a airport] [-f frequency]] [-d]\n",
     prog);
  printf ("\n");
  printf
    ("--user          | -U     username for VoIP account (default: '%s')\n",
     DEFAULT_USER);
  printf
    ("--password      | -P     password for VoIP account (default: '%s')\n",
     DEFAULT_PASSWORD);
  printf
    ("--voipserver    | -s     voip server to ceonnect to (default '%s')\n",
     DEFAULT_FG_SERVER);
  printf
    ("--port          | -p     where we should listen to FG(default '%d')\n",
     DEFAULT_FG_PORT);
  printf ("--airport       | -a     airport-id (ICAO) for ATC-mode\n");
  printf ("--frequency     | -f     frequency for ATC-mode\n");
  printf ("--mic           | -i     mic input level (0.0 - 1.0)\n");
  printf ("--speaker       | -o     speaker output level (0.0 - 1.0)\n");
  printf ("--debug         | -d     show debugging information\n");
  printf ("--mic-boost     | -b     enable mic boost\n");
  printf ("--list-audio    | -l     list audio devices\n");
  printf ("--set-audio-in  | -r     use <devicename> as audio input\n");
  printf ("--set-audio-out | -k     use <devicename> as audio output\n");
  printf ("\n");
  printf ("Mode 1: client for COM1 of flightgear:\n");
  printf ("$ %s\n", prog);
  printf ("- connects %s to fgfs at localhost:%d\n", prog, DEFAULT_FG_PORT);
  printf ("$ %s -sother.host.tld -p23456\n", prog);
  printf ("- connects %s to fgfs at other.host.tld:23456\n", prog);
  printf ("\n");
  printf ("Mode 2: client for an ATC at <airport> on <frequency>:\n");
  printf ("$ %s -aKSFO -d120.500\n", prog);
  printf ("- sets up %s for an ATC radio at KSFO 120.500 MHz\n", prog);
  printf ("\n");
  printf
    ("Note that %s starts with a guest account unless you use -U and -P!\n",
     prog);
  printf ("\n");
}
