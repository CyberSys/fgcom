/*
 * fgcom - VoIP-Client for the FlightGear-Radio-Infrastructure
 *
 * This program realizes the usage of the VoIP infractructure based
 * on flight data which is send from FlightGear with an external
 * protocol to this application.
 *
 * For more information read: http://squonk.abacab.org/dokuwiki/fgcom
 *
 * (c) H. Wirtz <wirtz@dfn.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 *
 */

#include "fgcom.h"
#include <plib/netSocket.h>

/* global vars */
int exitcode = 0;
static char debug = 0;
int initialized = 0;
int connected = 0;
int reg_id;
static int port;
const char* voipserver;
const char* fgserver;
const char* dialstring;
char airport[5];
static double frequency = -1.0;
static double selected_frequency = 0.0;
double level_in = 0.7;
double level_out = 0.7;
int codec=DEFAULT_IAX_CODEC;
static const char* username;
static const char* password;
static char mode = 0;
static char tmp[1024];		/* report output buffer */
static int last_state = 0;	/* previous state of the channel */
static char states[256];	/* buffer to hold ascii states */
static char delim = '\t';	/* output field delimiter */
static const char *map[] = {
  "unknown", "active", "outgoing", "ringing", "complete", "selected",
  "busy", "transfer", NULL
};
static const char *mode_map[] = {
  "ATC", "Flightgear"
};
static const char *radio_map[] = {
  "COM1", "NAV1", "COM2", "NAV2"
};
					/*const */ double earthradius = 6370.0;
					/* radius of  earth */
						/*const */ double uf = 0.01745329251994329509;
						/* conversion factor pi/180 degree->rad */
struct airport *airportlist;
struct fgdata data;
char icao[5];
double special_frq[] = { 999.999, 910.0, 123.45, 122.75, -1.0 };

double previous_com_frequency = 0.0;
int previous_ptt = 0;
int com_select = 0;
int max_com_instruments = 2;

void
process_packet (char *buf)
{

  /* cut off ending \n */
  buf[strlen (buf) - 1] = '\0';

  /* parse the data into a struct */
  parse_fgdata (&data, buf);

  /* get the selected frequency */
  if (com_select == 0 && data.COM1_SRV == 1)
    selected_frequency = data.COM1_FRQ;
  else if (com_select == 1 && data.NAV1_SRV == 1)
    selected_frequency = data.NAV1_FRQ;
  else if (com_select == 2 && data.COM2_SRV == 1)
    selected_frequency = data.COM2_FRQ;
  else if (com_select == 3 && data.NAV2_SRV == 1)
    selected_frequency = data.NAV2_FRQ;

  /* Check for com frequency changes */
  if (previous_com_frequency != selected_frequency)
    {
      printf ("Selected frequency: %3.3f\n", selected_frequency);

      /* remark the new frequency */
      previous_com_frequency = selected_frequency;

      if (connected == 1)
	{
	  /* hangup call, if connected */
	  iaxc_dump_call ();
	  iaxc_millisleep (5 * DEFAULT_MILLISLEEP);
	  connected = 0;
	}

      strcpy (icao,
	      icaobypos (airportlist, selected_frequency, data.LAT,
			 data.LON, DEFAULT_RANGE));
      icao2number (icao, selected_frequency, tmp);
#ifdef DEBUG
      printf ("dialing %s %3.3f MHz: %s\n", icao, selected_frequency, tmp);
#endif
      do_iaxc_call (username, password, voipserver, tmp);
      /* iaxc_select_call (0); */

      connected = 1;
    }
  /* Check for pressed PTT key */
  if (previous_ptt != data.PTT)
    {
      if (data.PTT == 2)
	{
	  /* select the next com equipment */
	  com_select = (com_select + 1) % 4;
	  printf ("Radio-Select: %s\n", radio_map[com_select]);
	}
      else if (connected == 1)
	{
	  ptt (data.PTT);
	}
      previous_ptt = data.PTT;
    }
}

int
main (int argc, char *argv[])
{
  int numbytes;
  static char buf[MAXBUFLEN];
  int c;
  int ret = 0;

  /* program header */
  printf ("%s - a communication radio based on VoIP with IAX/Asterisk\n",
	  argv[0]);
  printf ("(c)2007 by H. Wirtz <wirtz@dfn.de>\n");
  printf ("Version %s build %s\n", VERSION, SVN_REV);
  printf ("Using iaxclient library Version %s\n", iaxc_version (tmp));
  printf ("\n");

  /* init values */
  voipserver = DEFAULT_VOIP_SERVER;
  fgserver = DEFAULT_FG_SERVER;
  port = DEFAULT_FG_PORT;
  username = DEFAULT_USER;
  password = DEFAULT_PASSWORD;
  mode = 0;			/* 0=ATC mode, 1=FG mode */

#ifndef _WIN32
  /* catch signals */
  signal (SIGINT, quit);
  signal (SIGQUIT, quit);
  signal (SIGTERM, quit);
#endif

  /* setup iax */
#ifdef HAVE_IAX12
  if (iaxc_initialize (DEFAULT_MAX_CALLS))
#else
  if (iaxc_initialize (DEFAULT_IAX_AUDIO, DEFAULT_MAX_CALLS))
#endif
    fatal_error ("cannot initialize iaxclient!");
  initialized = 1;

  /* get options */
  while (1)
    {
      static struct option long_options[] = {
	{"debug", no_argument, 0, 'd'},
	{"voipserver", required_argument, 0, 'S'},
	{"fgserver", required_argument, 0, 's'},
	{"port", required_argument, 0, 'p'},
	{"airport", required_argument, 0, 'a'},
	{"frequency", required_argument, 0, 'f'},
	{"user", required_argument, 0, 'U'},
	{"password", required_argument, 0, 'P'},
	{"mic", required_argument, 0, 'i'},
	{"speaker", required_argument, 0, 'o'},
	{"mic-boost", no_argument, 0, 'b'},
	{"list-audio", no_argument, 0, 'l'},
	{"set-audio-in", required_argument, 0, 'r'},
	{"set-audio-out", required_argument, 0, 'k'},
	{"codec", required_argument, 0, 'C'},
	{0, 0, 0, 0}
      };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c =
	getopt_long (argc, argv, "dbls:p:a:f:U:P:i:o:r:k:S:C:", long_options,
		     &option_index);

      /* Detect the end of the options. */
      if (c == -1)
	break;

      switch (c)
	{
	case 'd':
#ifdef DEBUG
	  printf ("option -d\n");
#endif
	  debug = 1;
	  break;
	case 'S':
#ifdef DEBUG
	  printf ("option -S with value `%s'\n", optarg);
#endif
	  voipserver = optarg;
	  break;

	case 'C':
#ifdef DEBUG
	  printf ("option -C with value `%s'\n", optarg);
#endif
	  switch(optarg[0])
		{
		case 'u':
			codec=IAXC_FORMAT_ULAW;
			break;
		case 'a':
			codec=IAXC_FORMAT_ALAW;
			break;
		case 'g':
			codec=IAXC_FORMAT_GSM;
			break;
		case '7':
			codec=IAXC_FORMAT_G726;
			break;
		case 's':
			codec=IAXC_FORMAT_SPEEX;
			break;
		}
	  break;

	case 'p':
#ifdef DEBUG
	  printf ("option -p with value `%s'\n", optarg);
#endif
	  port = atoi (optarg);
	  break;

	case 's':
#ifdef DEBUG
	  printf ("option -s with value `%s'\n", optarg);
#endif
	  fgserver = optarg;
	  break;

	case 'a':
#ifdef DEBUG
	  printf ("option -a with value `%s'\n", optarg);
#endif
	  strtoupper (optarg, airport, sizeof(airport));
	  break;

	case 'f':
#ifdef DEBUG
	  printf ("option -f with value `%s'\n", optarg);
#endif
	  frequency = atof (optarg);
	  break;

	case 'D':
#ifdef DEBUG
	  printf ("option -D with value `%s'\n", optarg);
#endif
	  dialstring = optarg;
	  break;

	case 'i':
#ifdef DEBUG
	  printf ("option -i with value `%s'\n", optarg);
#endif
	  level_in = atof (optarg);
	  if (level_in > 1.0)
	    level_in = 1.0;
	  if (level_in < 0.0)
	    level_in = 0.0;
	  break;

	case 'o':
#ifdef DEBUG
	  printf ("option -o with value `%s'\n", optarg);
#endif
	  level_out = atof (optarg);
	  if (level_out > 1.0)
	    level_out = 1.0;
	  if (level_out < 0.0)
	    level_out = 0.0;
	  break;

	case 'U':
#ifdef DEBUG
	  printf ("option -U with value `%s'\n", optarg);
#endif
	  username = optarg;
	  break;

	case 'P':
#ifdef DEBUG
	  printf ("option -P with value `%s'\n", optarg);
#endif
	  password = optarg;
	  break;

	case 'b':
#ifdef DEBUG
	  printf ("option -b\n");
#endif
	  iaxc_mic_boost_set (1);
	  break;

	case 'l':
#ifdef DEBUG
	  printf ("option -l\n");
#endif

	  printf ("Input audio devices:\n");
	  printf ("%s", report_devices (IAXC_AD_INPUT));
	  printf ("\n");

	  printf ("Output audio devices:\n");
	  printf ("%s", report_devices (IAXC_AD_OUTPUT));
	  printf ("\n");

	  iaxc_shutdown ();
	  exit (1);
	  break;

	case 'r':
#ifdef DEBUG
	  printf ("option -r with value `%s'\n", optarg);
#endif
	  set_device (optarg, 0);
	  break;

	case 'k':
#ifdef DEBUG
	  printf ("option -k with value `%s'\n", optarg);
#endif
	  set_device (optarg, 1);
	  break;

	case '?':
	case 'h':
	  usage (argv[0]);
	  exit (10);
	  break;

	default:
	  abort ();
	}
    }

#ifdef DEBUG
  /* Print any remaining command line arguments (not options). */
  if (optind < argc)
    {
      printf ("non-option ARGV-elements: ");
      while (optind < argc)
	printf ("%s ", argv[optind++]);
      putchar ('\n');
    }
#endif

  /* checking consistency of arguments */
  if (frequency > 0.0 && frequency < 1000.0)
    {
      if (strlen (airport) == 0)
	{
	  strcpy (airport, "ZZZZ");
	}
      /* airport and frequency are given => ATC mode */
      mode = 0;
    }
  else
    {
      /* no airport => FG mode */
      mode = 1;
    }

  /* read airport frequencies and positions */
  airportlist = read_airports (DEFAULT_POSITIONS_FILE);

  /* preconfigure iax */
  printf ("Initializing IAX client as %s:%s@%s\n", username, "xxxxxxxxxxx",
	  voipserver);

  iaxc_set_callerid (const_cast<char*>(username), const_cast <char*>("0125252525122750"));
  iaxc_set_formats (codec,
		    IAXC_FORMAT_ULAW | IAXC_FORMAT_GSM | IAXC_FORMAT_SPEEX);
  iaxc_set_event_callback (iaxc_callback);

  iaxc_start_processing_thread ();

  if (username && password && voipserver)
    {
      reg_id = iaxc_register (const_cast<char*>(username), const_cast<char*>(password), const_cast<char*>(voipserver));
#ifdef DEBUG
      printf ("Registered as '%s' at '%s'\n", username, voipserver);
#endif
    }
  else
    {
      exitcode = 130;
      quit (0);
    }

  iaxc_millisleep (DEFAULT_MILLISLEEP);

  /* main loop */
#ifdef DEBUG
  printf ("Entering main loop in mode %s\n", mode_map[mode]);
#endif

  if (mode == 1)
    {
      /* only in FG mode */
      netInit ();
      netSocket fgsocket;
      fgsocket.open (false);
      fgsocket.bind (fgserver, port);

      /* mute mic, speaker on */
      iaxc_input_level_set (0);
      iaxc_output_level_set (level_out);

      ulClock clock;
      clock.update ();
      double next_update = clock.getAbsTime () + DEFAULT_ALARM_TIMER;
      /* get data from flightgear */
      while (1)
	{
	  clock.update ();
	  double wait = next_update - clock.getAbsTime ();
	  if (wait > 0.001)
	    {
	      netSocket *readsockets[2] = { &fgsocket, 0 };
	      if (fgsocket.
		  select (readsockets, readsockets + 1,
			  (int) (wait * 1000)) == 1)
		{
		  netAddress their_addr;
		  if ((numbytes =
		       fgsocket.recvfrom (buf, MAXBUFLEN - 1, 0,
					  &their_addr)) == -1)
		    {
		      perror ("recvfrom");
		      exit (1);
		    }
		  buf[numbytes] = '\0';
#ifdef DEBUG
		  printf ("got packet from %s:%d\n", their_addr.getHost (),
			  their_addr.getPort ());
		  printf ("packet is %d bytes long\n", numbytes);
		  printf ("packet contains \"%s\"\n", buf);
#endif
		  process_packet (buf);
		}
	    }
	  else
	    {
	      alarm_handler (0);
	      clock.update ();
	      next_update = clock.getAbsTime () + DEFAULT_ALARM_TIMER;
	    }
	}
    }

  else
    {
      /* only in ATC mode */
      struct pos p;

      /* mic on, speaker on */
      iaxc_input_level_set (level_in);
      iaxc_output_level_set (level_out);

      /* get geo positions of the airport */
      p = posbyicao (airportlist, airport);

      icao2number (airport, frequency, tmp);
#ifdef DEBUG
      printf ("dialing %s %3.3f MHz: %s\n", airport, frequency, tmp);
#endif
      do_iaxc_call (username, password, voipserver, tmp);
      /* iaxc_select_call (0); */

      while (1)
	{
	  /* sleep endless */
	  ulSleep (3600);
	}
    }

  /* should never be reached */
  exitcode = 999;
  quit (0);
}

void
quit (int signal)
{
  printf ("Stopping service\n");

  if (initialized)
    iaxc_shutdown ();
  if (reg_id)
    iaxc_unregister (reg_id);

  exit (exitcode);
}

void
alarm_handler (int signal)
{
  /* Check every DEFAULT_ALARM_TIMER seconds if position related things should happen */

  /* Send our coords to the server */
#ifdef SENDTEXT
  if (initialized == 1)
    {
      //iaxc_select_call (1);
      do_iaxc_call (username, password, voipserver, "0190909090999999");
      strcpy (tmp, "HIER DIE KOORDINATEN USW.\n");
      iaxc_send_text (tmp);
      iaxc_dump_call ();
    }
#endif

  if (check_special_frq (selected_frequency))
    {
      strcpy (icao, "ZZZZ");
    }
  else
    {
      strcpy (icao,
	      icaobypos (airportlist, selected_frequency, data.LAT, data.LON,
			 DEFAULT_RANGE));
    }

  /* Check if we are out of range */
  if (strlen (icao) == 0 && connected == 1)
    {
      /* Yes, we are out of range so hangup */
      iaxc_dump_call ();
      iaxc_millisleep (5 * DEFAULT_MILLISLEEP);
      connected = 0;
    }

  /* Check if we are now in range */
  else if (strlen (icao) != 0 && connected == 0)
    {
      icao2number (icao, selected_frequency, tmp);
#ifdef DEBUG
      printf ("dialing %s %3.3f MHz: %s\n", icao, selected_frequency, tmp);
#endif
      do_iaxc_call (username, password, voipserver, tmp);

      connected = 1;
    }
}

void
strtoupper (const char *str, char *buf, size_t len)
{
  int i;
  for (i = 0; str[i] && i < len - 1; i++)
    {
      buf[i] = toupper (str[i]);
    }

  buf[i++] = '\0';
}

void
fatal_error (const char *err)
{
  fprintf (stderr, "FATAL ERROR: %s\n", err);
  if (initialized)
    iaxc_shutdown ();
  exit (1);
}

int
iaxc_callback (iaxc_event e)
{
  switch (e.type)
    {
    case IAXC_EVENT_LEVELS:
      event_level (e.ev.levels.input, e.ev.levels.output);
      break;
    case IAXC_EVENT_TEXT:
      event_text (e.ev.text.type, e.ev.text.message);
      break;
    case IAXC_EVENT_STATE:
      event_state (e.ev.call.state, e.ev.call.remote, e.ev.call.remote_name,
		   e.ev.call.local, e.ev.call.local_context);
      break;
    case IAXC_EVENT_NETSTAT:
      event_netstats (e.ev.netstats);
    case IAXC_EVENT_REGISTRATION:
      event_register (e.ev.reg.id, e.ev.reg.reply, e.ev.reg.msgcount);
      break;
    default:
      event_unknown (e.type);
      break;
    }
  return 1;
}

void
event_state (int state, char *remote, char *remote_name,
	     char *local, char *local_context)
{
  last_state = state;
  /* This is needed for auto-reconnect */
  if (state == 0)
    {
      connected = 0;
      /* FIXME: we should wake up the main thread somehow */
      /* in fg mode the next incoming packet will do that anyway */
    }

  snprintf (tmp, sizeof (tmp),
	    "S%c0x%x%c%s%c%.50s%c%.50s%c%.50s%c%.50s", delim, state,
	    delim, map_state (state), delim, remote, delim, remote_name,
	    delim, local, delim, local_context);
  report (tmp);
}

void
event_text (int type, char *message)
{
  snprintf (tmp, sizeof (tmp), "T%c%d%c%.200s", delim, type, delim, message);
  printf ("%s\n", message);
  report (tmp);
}

void
event_register (int id, int reply, int count)
{
  const char *reason;
  switch (reply)
    {
    case IAXC_REGISTRATION_REPLY_ACK:
      reason = "accepted";
      break;
    case IAXC_REGISTRATION_REPLY_REJ:
      reason = "denied";
      if (strcmp (username, "guest") != 0)
	{
	  printf ("Registering denied\n");
	  /* exitcode = 110;
	     quit (SIGTERM); */
	}
      break;
    case IAXC_REGISTRATION_REPLY_TIMEOUT:
      reason = "timeout";
      break;
    default:
      reason = "unknown";
    }
  snprintf (tmp, sizeof (tmp), "R%c%d%c%s%c%d", delim, id, delim,
	    reason, delim, count);
  report (tmp);
}

void
event_netstats (struct iaxc_ev_netstats stat)
{
  struct iaxc_netstat local = stat.local;
  struct iaxc_netstat remote = stat.remote;
  snprintf (tmp, sizeof (tmp),
	    "N%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d",
	    delim, stat.callNo, delim, stat.rtt,
	    delim, local.jitter, delim, local.losspct, delim,
	    local.losscnt, delim, local.packets, delim, local.delay,
	    delim, local.dropped, delim, local.ooo, delim,
	    remote.jitter, delim, remote.losspct, delim, remote.losscnt,
	    delim, remote.packets, delim, remote.delay, delim,
	    remote.dropped, delim, remote.ooo);
  report (tmp);
}

void
event_level (double in, double out)
{
  snprintf (tmp, sizeof (tmp), "L%c%.1f%c%.1f", delim, in, delim, out);
  report (tmp);
}

const char *
map_state (int state)
{
  int i, j;
  int next = 0;
  *states = '\0';
  if (state == 0)
    {
      return "free";
    }
  for (i = 0, j = 1; map[i] != NULL; i++, j <<= 1)
    {
      if (state & j)
	{
	  if (next)
	    strcat (states, ",");
	  strcat (states, map[i]);
	  next = 1;
	}
    }
  return states;
}

void
event_unknown (int type)
{
  snprintf (tmp, sizeof (tmp), "U%c%d", delim, type);
  report (tmp);
}

void
report (char *text)
{
  if (debug > 0)
    {
      printf ("%s\n", text);
      fflush (stdout);
    }
}

void
ptt (int mode)
{
  if (mode == 1)
    {
      /* mic is muted so unmute and mute speaker */
      iaxc_input_level_set (level_in);
      if (!check_special_frq (selected_frequency))
	{
	  iaxc_output_level_set (0.0);
	  printf ("[SPEAK] unmute mic, mute speaker\n");
	}
      else
	{
	  printf ("[SPEAK] unmute mic\n");
	}
    }

  else
    {
      /* mic is unmuted so mute and unmute speaker */
      iaxc_input_level_set (0.0);
      if (!check_special_frq (selected_frequency))
	{
	  iaxc_output_level_set (level_out);
	  printf ("[LISTEN] mute mic, unmute speaker\n");
	}
      else
	{
	  printf ("[LISTEN] mute mic\n");
	}
    }
}

int
split (char *string, char *fields[], int nfields, const char *sep)
{
  register char *p = string;
  register char c;		/* latest character */
  register char sepc = sep[0];
  register char sepc2;
  register int fn;
  register char **fp = fields;
  register const char *sepp;
  register int trimtrail;
  /* white space */
  if (sepc == '\0')
    {
      while ((c = *p++) == ' ' || c == '\t')
	continue;
      p--;
      trimtrail = 1;
      sep = " \t";		/* note, code below knows this is 2 long */
      sepc = ' ';
    }
  else
    trimtrail = 0;
  sepc2 = sep[1];		/* now we can safely pick this up */
  /* catch empties */
  if (*p == '\0')
    return (0);
  /* single separator */
  if (sepc2 == '\0')
    {
      fn = nfields;
      for (;;)
	{
	  *fp++ = p;
	  fn--;
	  if (fn == 0)
	    break;
	  while ((c = *p++) != sepc)
	    if (c == '\0')
	      return (nfields - fn);
	  *(p - 1) = '\0';
	}
      /* we have overflowed the fields vector -- just count them */
      fn = nfields;
      for (;;)
	{
	  while ((c = *p++) != sepc)
	    if (c == '\0')
	      return (fn);
	  fn++;
	}
      /* not reached */
    }

  /* two separators */
  if (sep[2] == '\0')
    {
      fn = nfields;
      for (;;)
	{
	  *fp++ = p;
	  fn--;
	  while ((c = *p++) != sepc && c != sepc2)
	    if (c == '\0')
	      {
		if (trimtrail && **(fp - 1) == '\0')
		  fn++;
		return (nfields - fn);
	      }
	  if (fn == 0)
	    break;
	  *(p - 1) = '\0';
	  while ((c = *p++) == sepc || c == sepc2)
	    continue;
	  p--;
	}
      /* we have overflowed the fields vector -- just count them */
      fn = nfields;
      while (c != '\0')
	{
	  while ((c = *p++) == sepc || c == sepc2)
	    continue;
	  p--;
	  fn++;
	  while ((c = *p++) != '\0' && c != sepc && c != sepc2)
	    continue;
	}
      /* might have to trim trailing white space */
      if (trimtrail)
	{
	  p--;
	  while ((c = *--p) == sepc || c == sepc2)
	    continue;
	  p++;
	  if (*p != '\0')
	    {
	      if (fn == nfields + 1)
		*p = '\0';
	      fn--;
	    }
	}
      return (fn);
    }

  /* n separators */
  fn = 0;
  for (;;)
    {
      if (fn < nfields)
	*fp++ = p;
      fn++;
      for (;;)
	{
	  c = *p++;
	  if (c == '\0')
	    return (fn);
	  sepp = sep;
	  while ((sepc = *sepp++) != '\0' && sepc != c)
	    continue;
	  if (sepc != '\0')	/* it was a separator */
	    break;
	}
      if (fn < nfields)
	*(p - 1) = '\0';
      for (;;)
	{
	  c = *p++;
	  sepp = sep;
	  while ((sepc = *sepp++) != '\0' && sepc != c)
	    continue;
	  if (sepc == '\0')	/* it wasn't a separator */
	    break;
	}
      p--;
    }

  /* not reached */
}

struct airport *
read_airports (const char *file)
{
  FILE *fp;
  int ret;
  struct airport airport_tmp;
  struct airport *first = NULL;
  struct airport *my_airport = NULL;
  struct airport *previous_airport = NULL;
  printf ("Reading list of airports...");
  if ((fp = fopen (file, "rt")) == NULL)
    {
      printf ("Cannot open %s\n", DEFAULT_POSITIONS_FILE);
      perror ("fopen");
      exitcode = 120;
      quit (0);
    }

  airport_tmp.next = NULL;
  while ((ret = fscanf (fp, " %4[^,],%f,%lf,%lf,%32[^,],%128[^\r\n]",
			airport_tmp.icao, &airport_tmp.frequency,
			&airport_tmp.lat, &airport_tmp.lon,
			airport_tmp.type, airport_tmp.text)) == 6)
    {
      if ((my_airport =
	   (struct airport *) malloc (sizeof (struct airport))) == NULL)
	{
	  printf ("Error allocating memory for airport data\n");
	  exitcode = 900;
	  quit (0);
	}

      if (first == NULL)
	first = my_airport;
      memcpy (my_airport, &airport_tmp, sizeof (airport_tmp));
      if (previous_airport != NULL)
	{
	  previous_airport->next = my_airport;
	}
      previous_airport = my_airport;
    }

  fclose (fp);
  if (ret != EOF)
    {
      printf ("error during reading airports!\n");
      exitcode = 900;
      quit (0);
    }

  printf ("done.\n");
  return (first);
}

char *
report_devices (int in)
{
  struct iaxc_audio_device *devs;	/* audio devices */
  int ndevs;			/* audio dedvice count */
  int input, output, ring;	/* audio device id's */
  int current, i;
  int flag = in ? IAXC_AD_INPUT : IAXC_AD_OUTPUT;
  iaxc_audio_devices_get (&devs, &ndevs, &input, &output, &ring);
  current = in ? input : output;
  snprintf (tmp, sizeof (tmp), "%s\n", devs[current].name);
  for (i = 0; i < ndevs; i++)
    {
      if (devs[i].capabilities & flag && i != current)
	{
	  snprintf (tmp + strlen (tmp), sizeof (tmp) - strlen (tmp), "%s\n",
		    devs[i].name);
	}
    }
  return tmp;
}

int
set_device (char *name, int out)
{
  struct iaxc_audio_device *devs;	/* audio devices */
  int ndevs;			/* audio dedvice count */
  int input, output, ring;	/* audio device id's */
  int i;
  iaxc_audio_devices_get (&devs, &ndevs, &input, &output, &ring);
  for (i = 0; i < ndevs; i++)
    {
      if (devs[i].capabilities & (out ? IAXC_AD_OUTPUT : IAXC_AD_INPUT) &&
	  strcmp (name, devs[i].name) == 0)
	{
	  if (out)
	    {
	      output = devs[i].devID;
	    }
	  else
	    {
	      input = devs[i].devID;
	    }
	  fprintf (stderr, "device %s = %s (%d)\n", out ? "out" : "in", name,
		   devs[i].devID);
	  iaxc_audio_devices_set (input, output, ring);
	  return 1;
	}
    }
  return 0;
}

void
parse_fgdata (struct fgdata *data, char *buf)
{
  char *data_pair = NULL;
  char *fields[2];
  fields[0] = '\0';
  fields[1] = '\0';
#ifdef DEBUG
  printf ("Parsing data: [%s]\n", buf);
#endif
  /* Parse data from FG */
  data_pair = strtok (buf, ",");
  while (data_pair != NULL)
    {
      split (data_pair, fields, 2, "=");
      if (strcmp (fields[0], "COM1_FRQ") == 0)
	{
	  data->COM1_FRQ = atof (fields[1]);
#ifdef DEBUG
	  printf ("COM1_FRQ=%3.3f\n", data->COM1_FRQ);
#endif
	}
      else if (strcmp (fields[0], "COM2_FRQ") == 0)
	{
	  data->COM2_FRQ = atof (fields[1]);
#ifdef DEBUG
	  printf ("COM2_FRQ=%3.3f\n", data->COM2_FRQ);
#endif
	}
      else if (strcmp (fields[0], "NAV1_FRQ") == 0)
	{
	  data->NAV1_FRQ = atof (fields[1]);
#ifdef DEBUG
	  printf ("NAV1_FRQ=%3.3f\n", data->NAV1_FRQ);
#endif
	}
      else if (strcmp (fields[0], "NAV2_FRQ") == 0)
	{
	  data->NAV2_FRQ = atof (fields[1]);
#ifdef DEBUG
	  printf ("NAV2_FRQ=%3.3f\n", data->NAV2_FRQ);
#endif
	}
      else if (strcmp (fields[0], "PTT") == 0)
	{
	  data->PTT = atoi (fields[1]);
#ifdef DEBUG
	  printf ("PTT=%d\n", data->PTT);
#endif
	}
      else if (strcmp (fields[0], "TRANSPONDER") == 0)
	{
	  data->TRANSPONDER = atoi (fields[1]);
#ifdef DEBUG
	  printf ("TRANSPONDER=%d\n", data->TRANSPONDER);
#endif
	}
      else if (strcmp (fields[0], "IAS") == 0)
	{
	  data->IAS = atof (fields[1]);
#ifdef DEBUG
	  printf ("IAS=%f\n", data->IAS);
#endif
	}
      else if (strcmp (fields[0], "GS") == 0)
	{
	  data->GS = atof (fields[1]);
#ifdef DEBUG
	  printf ("GS=%f\n", data->GS);
#endif
	}
      else if (strcmp (fields[0], "LON") == 0)
	{
	  data->LON = atof (fields[1]);
#ifdef DEBUG
	  printf ("LON=%f\n", data->LON);
#endif
	}
      else if (strcmp (fields[0], "LAT") == 0)
	{
	  data->LAT = atof (fields[1]);
#ifdef DEBUG
	  printf ("LAT=%f\n", data->LAT);
#endif
	}
      else if (strcmp (fields[0], "ALT") == 0)
	{
	  data->ALT = atoi (fields[1]);
#ifdef DEBUG
	  printf ("ALT=%d\n", data->ALT);
#endif
	}
      else if (strcmp (fields[0], "HEAD") == 0)
	{
	  data->HEAD = atof (fields[1]);
#ifdef DEBUG
	  printf ("HEAD=%f\n", data->HEAD);
#endif
	}
      else if (strcmp (fields[0], "COM1_SRV") == 0)
	{
	  data->COM1_SRV = atoi (fields[1]);
#ifdef DEBUG
	  printf ("COM1_SRV=%d\n", data->COM1_SRV);
#endif
	}
      else if (strcmp (fields[0], "COM2_SRV") == 0)
	{
	  data->COM2_SRV = atoi (fields[1]);
#ifdef DEBUG
	  printf ("COM2_SRV=%d\n", data->COM2_SRV);
#endif
	}
      else if (strcmp (fields[0], "NAV1_SRV") == 0)
	{
	  data->NAV1_SRV = atoi (fields[1]);
#ifdef DEBUG
	  printf ("NAV1_SRV=%d\n", data->NAV1_SRV);
#endif
	}
      else if (strcmp (fields[0], "NAV2_SRV") == 0)
	{
	  data->NAV2_SRV = atoi (fields[1]);
#ifdef DEBUG
	  printf ("NAV2_SRV=%d\n", data->NAV2_SRV);
#endif
	}
#ifdef DEBUG
      else
	{
	  printf ("Unknown val: %s (%s)\n", fields[0], fields[1]);
	}
#endif

      data_pair = strtok (NULL, ",");
    }
#ifdef DEBUG
  printf ("\n");
#endif
}

int
check_special_frq (double frq)
{
  int i = 0;
  while (special_frq[i] >= 0.0)
    {
      if (frq == special_frq[i])
	return (1);
      i++;
    }

  return (0);
}

void
do_iaxc_call(const char* username, const char* password, const char* voipserver, const char* number)
{
  char dest[256];
  
  snprintf (dest, sizeof(dest), "%s:%s@%s/%s", username, password, voipserver, number);
  iaxc_call (dest);
  iaxc_millisleep (DEFAULT_MILLISLEEP);
}
