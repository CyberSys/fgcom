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

#ifndef _config_h
#define _config_h

#include "glib.h"

/* configuration data */
struct fgcom_config
{
	/* group main options */
	gchar *iax_server;
	gboolean verbose;

	/* group VoIP server */
        gchar *username;
        gchar *password;
	gint codec;

	/* group FG arguments */
	gchar *fg;
	gint fg_port;
	gint fg_copilot_conference_id;

	/* group ATC data */
	gdouble atc_frequency;
	gdouble atc_lon;
	gdouble atc_lat;

	/* group audio parameters */
	gboolean mic_boost;
	gdouble mic_level;
	gdouble speaker_level;
	gchar *audio_in;
	gchar *audio_out;

	/* program internal data */
	int reg_id;
	int initialized;
	int connected;
} config;

/* public prototypes */
gboolean config_parse_cmd_options(char *filename, int argc, char *argv[]);

/* private prototypes */
static void config_show_version(void);
static void config_show_audio_devices(void);
static void config_report_devices(int io);

#endif
