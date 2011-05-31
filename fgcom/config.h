
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

#include <glib.h>
#include <gnet.h>

/* configuration data */
struct fgcom_config
{
	/* group main options */
	gchar *server;
	gboolean verbose;
	gchar *callsign;
	gchar *modelname;

	/* group FG arguments */
	gchar *fg;
	gint fg_port;
	gint fg_intercom_id;
	gint fg_intercom_port;

	/* group ATC data */
	gdouble frequency;
	gdouble lon;
	gdouble lat;
	gint alt;

	/* group audio parameters */
	gboolean mic_boost;
	gdouble mic_level;
	gdouble speaker_level;
	gchar *audio_in;
	gchar *audio_out;

	/* program internal data */
	gint mode;
	GInetAddr *fg_addr;
	GUdpSocket *fg_socket;
} config;

/* public prototypes */
gboolean config_parse_cmd_options (char *filename, int argc, char *argv[]);

/* private prototypes */
static gboolean _config_cb_callsign (gchar * option_name, gchar * value,
							 gpointer data, GError ** error);
static gboolean _config_cb_lon (gchar * option_name, gchar * value,
						 gpointer data, GError ** error);
static gboolean _config_cb_lat (gchar * option_name, gchar * value,
						 gpointer data, GError ** error);
static gboolean _config_cb_mic_level (gchar * option_name, gchar * value,
							  gpointer data, GError ** error);
static gboolean _config_cb_speaker_level (gchar * option_name, gchar * value,
								 gpointer data, GError ** error);
static void _config_show_version (void);
static void _config_show_audio_devices (void);

#endif
