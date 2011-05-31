/*
 * fgcom: a real radio client for FlightGear
 *
 * Copyrights:
 * Copyright (C) 2011 Holger Wirtz   <dcoredump@gmail.com>
 *
 * This program may be modified and distributed under the
 * terms of the GNU General Public License. You should have received
 * a copy of the GNU General Public License along with this
 * program; if not, write to the Free Software Foundation, Inc.
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <glib.h>
#include <gnet.h>
#include "fgcom.h"
#include "config.h"
#include "net.h"

extern struct fgcom_config config;

/* Main program */
int
main (int argc, char *argv[])
{
	gchar *home;
	gchar tmp[256];
	
	if ((home = getenv ("HOME")) != NULL)
	{
		sprintf (tmp, "%s/.fgcomrc", home);
		if (config_parse_cmd_options (tmp, argc, argv) == FALSE)
		{
			fgcom_exit ("Program stop due to option errors.", 100);
		}
	}

	if (config.verbose == TRUE)
		g_printf ("fgcom starting up...\n");

	// catch signals
	signal (SIGINT, _fgcom_quit);
	signal (SIGQUIT, _fgcom_quit);
	signal (SIGHUP, _fgcom_quit);
	//signal (SIGALRM, _fgcom_update_session);
}

/****************************************************************************
 *  public functions
 ****************************************************************************/

void
fgcom_exit (gchar * text, gint exitcode)
{
	if (strlen (text) > 0)
		g_fprintf (stderr, "%s\n", text);
	_fgcom_quit (exitcode);
}

gboolean
fgcom_parse_data (struct fg_data *data, gchar * from_fg)
{
	gchar *tmp;
	gchar tmp_string[256];

	tmp = g_strdup (strtok ((char *) from_fg, ","));
	while (tmp != NULL)
	{
		if (sscanf (tmp, "COM1_FRQ=%lf", &data->COM1_FRQ) == 1);
		else if (sscanf (tmp, "COM1_SRV=%d", &data->COM1_SRV) == 1);
		else if (sscanf (tmp, "COM2_FRQ=%lf", &data->COM2_FRQ) == 1);
		else if (sscanf (tmp, "COM2_SRV=%d", &data->COM2_SRV) == 1);
		else if (sscanf (tmp, "NAV1_FRQ=%lf", &data->NAV1_FRQ) == 1);
		else if (sscanf (tmp, "NAV1_SRV=%d", &data->NAV1_SRV) == 1);
		else if (sscanf (tmp, "NAV2_FRQ=%lf", &data->NAV2_FRQ) == 1);
		else if (sscanf (tmp, "NAV2_SRV=%d", &data->NAV2_SRV) == 1);
		else if (sscanf (tmp, "PTT=%d", &data->PTT) == 1);
		else if (sscanf (tmp, "TRANSPONDER=%d", &data->TRANSPONDER) == 1);
		else if (sscanf (tmp, "IAS=%d", &data->IAS) == 1);
		else if (sscanf (tmp, "GS=%d", &data->GS) == 1);
		else if (sscanf (tmp, "LON=%lf", &config.lon) == 1);
		else if (sscanf (tmp, "LAT=%lf", &config.lat) == 1);
		else if (sscanf (tmp, "ALT=%d", &config.alt) == 1);
		else if (sscanf (tmp, "HEAD=%lf", &data->HEAD) == 1);
		else if (sscanf (tmp, "CALLSIGN=%s", &tmp_string) == 1)
			config.callsign = strdup (tmp_string);
		else if (sscanf (tmp, "MODEL=%s", &tmp_string) == 1)
			config.modelname = strdup (tmp_string);
		tmp = strtok (NULL, ",");
	}
	return (TRUE);
}

/****************************************************************************
 *  private functions
 ****************************************************************************/

static void
_fgcom_quit (gint exitcode)
{
	gchar text[256];

	g_printf ("shutting down - please wait...\n");

	exit ((int) exitcode);
}
