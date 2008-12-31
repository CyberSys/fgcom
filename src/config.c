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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "iaxclient.h"
#include "fgcom.h"
#include "config.h"

/* command line configuration options */
static GOptionEntry main_entries[] = 
{
  { "server", 's', 0, G_OPTION_ARG_STRING, &config.iax_server, "set the server address for VoIP", NULL },
  { "verbose", 'v', 0, G_OPTION_ARG_NONE, &config.verbose, "show more information while working", NULL },
  { "version", 'V', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_show_version, "show version information", NULL },
  { "username", 'U', 0, G_OPTION_ARG_STRING, &config.username, "set the username for registration", NULL },
  { "password", 'P', 0, G_OPTION_ARG_STRING, &config.password, "set the password for registration", NULL },
  { "codec", 'C', 0, G_OPTION_ARG_STRING, &config.codec, "use one of the following codecs:\n\t\t\t\tspeex (default)\n\t\t\t\tgsm\n\t\t\t\tg711a\n\t\t\t\tg711u", NULL },
  { "fg", 'f', 0, G_OPTION_ARG_STRING, &config.fg, "set the address for FlightGear (default 127.0.0.1)", NULL },
  { "fg-port", 'p', 0, G_OPTION_ARG_INT, &config.fg_port, "set the port number for FlightGear (default 16661)", NULL },
  { "fg-copilot-id", 'i', 0, G_OPTION_ARG_INT, &config.fg_copilot_conference_id, "set the id for copilot conference", NULL },
  { "atc-frq", 'a', 0, G_OPTION_ARG_DOUBLE, &config.atc_frequency, "set the frequency for standalone (ATC) mode" , NULL },
  { "atc-lon", 'O', 0, G_OPTION_ARG_DOUBLE, &config.atc_lon, "set longtitude for standalone (ATC) mode", NULL },
  { "atc-lat", 'A', 0, G_OPTION_ARG_DOUBLE, &config.atc_lat, "set latitude for standalone (ATC) mode", NULL },
  { "mic-boost", 'm', 0, G_OPTION_ARG_NONE, &config.mic_boost, "enable mic boost", NULL },
  { "miclevel", 'L', 0, G_OPTION_ARG_DOUBLE, &config.mic_level, "set mic level (0.0-1.0)", NULL },
  { "speaker-level", 'l', 0, G_OPTION_ARG_DOUBLE, &config.speaker_level, "set speaker level (0.0-1.0)", NULL },
  { "audio-in", 'i', 0, G_OPTION_ARG_STRING, &config.audio_in, "audio device for input", NULL },
  { "audio-out", 'o', 0, G_OPTION_ARG_STRING, &config.audio_out, "audio device for output", NULL },
  { "list-audio", 0x0, G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_show_audio_devices, "show audio devices", NULL },
  { NULL }
};

/****************************************************************************
 *  public functions
 ****************************************************************************/

/* function for parsing all command line configuration options after reading
   the own configuration file '~/.fgcomrc' */
gboolean config_parse_cmd_options(char *filename, int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context;

	/* set the defaults */
	config.reg_id=-1;
	config.initialized=-1;
	config.connected=-1;
	config.iax_server=g_strdup((gchar *)DEFAULT_VOIP_SERVER);
	config.username=g_strdup((gchar *)DEFAULT_USER);
	config.password=g_strdup((gchar *)DEFAULT_PASSWORD);
	config.codec=DEFAULT_IAX_CODEC;

	/* create the cmd line options */
	context = g_option_context_new("- real radio simulation based on VoIP");
	g_option_context_add_main_entries(context, main_entries, NULL);

	/* parse the cmd line options */
	if(!g_option_context_parse(context, &argc, &argv, &error))
	{
		g_print ("option parsing failed: %s\n", error->message);
		return(FALSE);
	}

	return(TRUE);
}

/****************************************************************************
 *  private functions
 ****************************************************************************/

static void config_show_version(void)
{
	printf("(c)2008 by H. Wirtz <dcoredump@gmail.com> && C. Ingels <charles@maisonblv.net>\n");
	printf("Version %3.2f build TOBEFIXED\n",VERSION);

	fgcom_exit("",0);
}

static void config_show_audio_devices(void)
{
	if(iaxc_initialize(DEFAULT_MAX_CALLS)!=0)
		fgcom_exit("Cannot initialize iaxclient!",203);

	config.initialized=1;

	printf("Input audio devices:\n");
	config_report_devices (IAXC_AD_INPUT);
	printf("\n");
	printf("Output audio devices:\n");
	config_report_devices (IAXC_AD_OUTPUT);

	fgcom_exit("",0);
}

static void config_report_devices(int io)
{
  struct iaxc_audio_device *devs;       /* audio devices */
  int ndevs;                    /* audio dedvice count */
  int input, output, ring;      /* audio device id's */
  int current, i;
  int flag = io ? IAXC_AD_INPUT : IAXC_AD_OUTPUT;
  iaxc_audio_devices_get (&devs, &ndevs, &input, &output, &ring);
  current = io ? input : output;
  printf("\t%s\n",devs[current].name);
  for (i = 0; i < ndevs; i++)
    {
      if (devs[i].capabilities & flag && i != current)
        {
          printf("\t\t%s\n",devs[i].name);
        }
    }
}
