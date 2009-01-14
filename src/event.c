/*
 * fgcom: a real radio VoIP client for FlightGear based on iaxclient
 *
 * Copyrights:
 * Copyright (C) 2006-2008 Holger Wirtz   <dcoredump@gmail.com>
 *                                        <wirtz@parasitstudio.de>
 * Copyright (C) 2008,2009 Holger Wirtz   <dcoredump@gmail.com>
 *                                        <wirtz@parasitstudio.de>
 *                         Charles Ingels <charles@maisonblv.net>
 *
 * This program may be modified and distributed under the
 * terms of the GNU General Public License. You should have received
 * a copy of the GNU General Public License along with this
 * program; if not, write to the Free Software Foundation, Inc.
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <string.h>
#include <glib.h>
#include "iaxclient.h"
#include "fgcom.h"
#include "config.h"
#include "event.h"

extern struct fgcom_config config;

/****************************************************************************
 *  public functions
 ****************************************************************************/

void event_state(int state, char *remote, char *remote_name, char *local, char *local_context)
{
  last_state = state;
  /* This is needed for auto-reconnect */
  if (state == 0)
    {
      config.connected = FALSE;
      /* FIXME: we should wake up the main thread somehow */
      /* in fg mode the next incoming packet will do that anyway */
    }

  snprintf (tmp, sizeof (tmp),
            "S%c0x%x%c%s%c%.50s%c%.50s%c%.50s%c%.50s", delim, state,
            delim, map_state (state), delim, remote, delim, remote_name,
            delim, local, delim, local_context);
  report (tmp);
}

void event_text(int type, char *message)
{
  snprintf (tmp, sizeof (tmp), "T%c%d%c%.200s", delim, type, delim, message);
  printf("%s\n",message);
  report (tmp);
}

void event_register(int id, int reply, int count)
{
  const char *reason;
  switch (reply)
    {
    case IAXC_REGISTRATION_REPLY_ACK:
      reason = "accepted";
      break;
    case IAXC_REGISTRATION_REPLY_REJ:
      reason = "denied";
      /* fgcom_exit("Registering denied",201);*/
      printf("Registering denied!\n");
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

void event_netstats(struct iaxc_ev_netstats stat)
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

void event_level(double in, double out)
{
  snprintf (tmp, sizeof (tmp), "L%c%.1f%c%.1f", delim, in, delim, out);
  report (tmp);
}

void event_unknown(int type)
{
  snprintf (tmp, sizeof (tmp), "U%c%d", delim, type);
  report (tmp);
}

/****************************************************************************
 *  private functions
 ****************************************************************************/

static void report(char *text)
{
	if(config.verbose==TRUE)
	{
		printf("%s\n",text);
      		fflush (stdout);
	}
}

static const char *map_state (int state)
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

