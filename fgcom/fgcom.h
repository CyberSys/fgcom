
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
#define DEFAULT_SERVER "127.0.0.1\0"
#define DEFAULT_MIC_LEVEL 0.5
#define DEFAULT_SPEAKER_LEVEL 0.5
#define DEFAULT_POSTTION_UPDATE_FREQUENCY 7
#define DEFAULT_PACKET_TIMEOUT 15.0

#define FGCOM_UDP_MAX_BUFFER 1024

enum
{ MODE_PLAY = 0, MODE_FG, MODE_ATC };

struct fg_data
{
	gdouble COM1_FRQ;
	gint COM1_SRV;
	gdouble COM2_FRQ;
	gint COM2_SRV;
	gdouble NAV1_FRQ;
	gint NAV1_SRV;
	gdouble NAV2_FRQ;
	gint NAV2_SRV;
	gint PTT;
	gdouble TRANSPONDER;
	gdouble IAS;
	gdouble GS;
	gdouble LON;
	gdouble LAT;
	gint ALT;
	gdouble HEAD;
};

/* public prototypes */
void fgcom_exit (gchar * text, gint exitcode);
gboolean fgcom_parse_data (struct fg_data *data, gchar * from_fg);

/* private prototypes */
static void _fgcom_quit (gint exitcode);
#endif
