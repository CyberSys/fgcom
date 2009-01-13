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

#ifndef _fgcom_h
#define _fgcom_h

#define VERSION 3.0
#define DEFAULT_MAX_CALLS 2
#define DEFAULT_USER "guest\0"
#define DEFAULT_PASSWORD "guest\0"
#define DEFAULT_FRQ "02911000"
#define DEFAULT_PRESELECTION 2
#define DEFAULT_VOIP_SERVER "fgcom1.parasitstudio.de\0"
#define DEFAULT_MIC_LEVEL 0.5
#define DEFAULT_SPEAKER_LEVEL 0.5

enum { MODE_PLAY=0, MODE_FG, MODE_ATC };

/* public prototypes */
void fgcom_exit(gchar *text, gint exitcode);

/* private prototypes */
static int fgcom_iaxc_callback(iaxc_event e);
static void fgcom_quit (gint exitcode);
static gboolean fgcom_dial(gdouble frequency);
static gboolean fgcom_conference_command(gchar *command, ...);
static gboolean fgcom_send_audio(gchar *filename);
#endif
