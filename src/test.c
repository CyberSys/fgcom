#include "fgcom.h"

/* global vars */
int initialized;
int connected;
int reg_id;
static char delim = '\t';       /* output field delimiter */
static char tmp[1024];          /* report output buffer */
static int last_state = 0;      /* previous state of the channel */
static char states[256];        /* buffer to hold ascii states */
static char *map[] = {
  "unknown", "active", "outgoing", "ringing", "complete", "selected",
  "busy", "transfer", NULL
};


int
main (int argc, char *argv[])
{
  iaxc_initialize (DEFAULT_IAX_AUDIO, DEFAULT_MAX_CALLS);
  iaxc_set_callerid ("fgcom", "0125252525122750");
  iaxc_set_formats (DEFAULT_IAX_CODEC,
		    IAXC_FORMAT_ULAW | IAXC_FORMAT_GSM | IAXC_FORMAT_SPEEX);
  iaxc_set_event_callback (iaxc_callback);

  iaxc_start_processing_thread ();

  iaxc_register ("fgcom", "sagichnicht", "fgcom1.parasitstudio.de");

  iaxc_millisleep (DEFAULT_MILLISLEEP);

  iaxc_input_level_set (0);
  iaxc_output_level_set (1.0);

  while(1)
  {
      iaxc_call ("fgcom:sagichnicht@fgcom1.parasitstudio.de/0132909086111400");
  iaxc_millisleep (DEFAULT_MILLISLEEP);
	iaxc_dump_call ();
  iaxc_millisleep (5*DEFAULT_MILLISLEEP);
 }
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
      event_state (e.ev.call.state, e.ev.call.remote,
		   e.ev.call.remote_name, e.ev.call.local,
		   e.ev.call.local_context);
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
  char *reason;
  switch (reply)
    {
    case IAXC_REGISTRATION_REPLY_ACK:
      reason = "accepted";
      break;
    case IAXC_REGISTRATION_REPLY_REJ:
      reason = "denied";
      if (strcmp ("fgcom", "guest") != 0)
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

char *
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
      printf ("%s\n", text);
      fflush (stdout);
}

