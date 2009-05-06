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

#ifndef _event_h
#define _event_h

/* public prototypes */
void event_state(int state, char *remote, char *remote_name, char *local, char *local_context);
void event_text(int type, char *message);
void event_register(int id, int reply, int count);
void event_register(int id, int reply, int count);
void event_netstats(struct iaxc_ev_netstats stat);
void event_level(double in, double out);
void event_unknown(int type);

/* private prototypes */
static void report(char *text);
static const char *map_state (int state);

/* Globals */
static char tmp[1024];          /* report output buffer */
static int last_state = 0;      /* previous state of the channel */
static char states[256];        /* buffer to hold ascii states */
static char delim = '\t';       /* output field delimiter */
static const char *map[] = {
  "unknown", "active", "outgoing", "ringing", "complete", "selected",
  "busy", "transfer", NULL
};

#endif
