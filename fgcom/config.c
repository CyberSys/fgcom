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
  { "callsign", 'c', 0, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_cb_callsign,"set the callsign for ATC mode" , NULL },
  { "username", 'U', 0, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_cb_username, "set the username for registration", NULL },
  { "password", 'P', 0, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_cb_password, "set the password for registration", NULL },
  { "codec", 'C', 0, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_cb_codec, "use one of the following codecs:\n\t\t\t\tspeex (default)\n\t\t\t\tgsm\n\t\t\t\tg711a\n\t\t\t\tg711u", NULL },
  { "fg", 'f', 0, G_OPTION_ARG_STRING, &config.fg, "set the address for FlightGear (default 127.0.0.1)", NULL },
  { "fg-port", 'p', 0, G_OPTION_ARG_INT, &config.fg_port, "set the port number for connection to FlightGear (default 16661)", NULL },
  { "intercom-id", 'i', 0, G_OPTION_ARG_INT, &config.fg_intercom_id, "set the id for the intercom conference", NULL },
  { "intercom-port", 'I', 0, G_OPTION_ARG_INT, &config.fg_intercom_port, "set the port number for the connection to FlightGear intercom", NULL },
  { "frq", 'a', 0, G_OPTION_ARG_DOUBLE, &config.frequency, "set the frequency for standalone (ATC) mode" , NULL },
  { "lon", 'O', 0, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_cb_lon, "set longtitude for standalone (ATC) mode", NULL },
  { "lat", 'A', 0, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_cb_lat, "set latitude for standalone (ATC) mode", NULL },
  { "mic-boost", 'm', 0, G_OPTION_ARG_NONE, &config.mic_boost, "enable mic boost", NULL },
  { "mic-level", 'L', 0, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_cb_mic_level, "set mic level (0.0-1.0)", NULL },
  { "speaker-level", 'l', 0, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_cb_speaker_level, "set speaker level (0.0-1.0)", NULL },
  { "audio-in", 0x0, 0, G_OPTION_ARG_STRING, &config.audio_in, "audio device for input", NULL },
  { "audio-out", 0x0, 0, G_OPTION_ARG_STRING, &config.audio_out, "audio device for output", NULL },
  { "list-audio", 0x0, G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_show_audio_devices, "show audio devices", NULL },
  { "play", 0x0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_play, "play audio file instead of using mic", NULL },
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
	config.iax_server=g_strdup((gchar *)DEFAULT_VOIP_SERVER);
	config.verbose=FALSE;
	config.callsign=NULL;
	config.modelname=NULL;
	config.username=NULL;
	config.password=NULL;
	config.codec=IAXC_FORMAT_SPEEX;
	config.fg=g_strdup((gchar *)"localhost");
	config.fg_port=16661;
	config.fg_intercom_id=0;
	config.fg_intercom_port=0;
	config.frequency=0.0;
	config.lon=-9999.99;
	config.lat=-9999.99;
	config.mic_boost=FALSE;
	config.mic_level=DEFAULT_MIC_LEVEL;
	config.speaker_level=DEFAULT_SPEAKER_LEVEL;
	config.audio_in=g_strdup("default");
	config.audio_out=g_strdup("default");
	config.reg_id=-1;
	config.initialized=FALSE;
	config.connected=FALSE;
	config.mode=-1;
	config.reg=FALSE;
	config.play_file=NULL;
	config.fg_addr=NULL;
	config.fg_socket=NULL;

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

static gboolean config_cb_callsign(gchar *option_name,gchar *value,gpointer data,GError **error)
{
	config.callsign=strdup(value);

	return(TRUE);
}

static gboolean config_cb_lon(gchar *option_name,gchar *value,gpointer data,GError **error)
{
	GError *tmp_error=NULL;

	config.lon=g_ascii_strtod(value,NULL);

	if(config.lon<-180.0 || config.lon>180.0)
	{
		tmp_error=g_error_new(G_OPTION_ERROR,G_OPTION_ERROR_BAD_VALUE,"%s must be between -180.0 and 180.0 degree\n",option_name);
		g_propagate_error(error,tmp_error);
		return(FALSE);
	}

	return(TRUE);
}

static gboolean config_cb_lat(gchar *option_name,gchar *value,gpointer data,GError **error)
{
	GError *tmp_error=NULL;

	config.lat=g_ascii_strtod(value,NULL);

	if(config.lat<-90.0 || config.lat>90.0)
	{
		tmp_error=g_error_new(G_OPTION_ERROR,G_OPTION_ERROR_BAD_VALUE,"%s must be between -180.0 and 180.0 degree\n",option_name);
		g_propagate_error(error,tmp_error);
		return(FALSE);
	}

	return(TRUE);
}

static gboolean config_cb_codec(gchar *option_name,gchar *value,gpointer data,GError **error)
{
	GError *tmp_error=NULL;

	if(g_ascii_strcasecmp(value,"speex")==0)
		config.codec=IAXC_FORMAT_SPEEX;
	else if(g_ascii_strcasecmp(value,"gsm")==0)
		config.codec=IAXC_FORMAT_GSM;
	else if(g_ascii_strcasecmp(value,"g711a")==0)
		config.codec=IAXC_FORMAT_ALAW;
	else if(g_ascii_strcasecmp(value,"g711u")==0)
		config.codec=IAXC_FORMAT_ULAW;
	else
	{
		tmp_error=g_error_new(G_OPTION_ERROR,G_OPTION_ERROR_BAD_VALUE,"%s must be one of \"speex\",\"gsm\",\"g711a\",\"g711u\"\n",option_name);
		g_propagate_error(error,tmp_error);
		return(FALSE);
	}

	return(TRUE);
}

static gboolean config_cb_username(gchar *option_name,gchar *value,gpointer data,GError **error)
{
	GError *tmp_error=NULL;

	config.reg=TRUE; /* client wishes registering */
	config.username=strdup(value);

	return(TRUE);
}

static gboolean config_cb_password(gchar *option_name,gchar *value,gpointer data,GError **error)
{
	GError *tmp_error=NULL;

	config.reg=TRUE; /* client wishes registering */
	config.password=strdup(value);

	return(TRUE);
}

static gboolean config_cb_mic_level(gchar *option_name,gchar *value,gpointer data,GError **error)
{
	config.mic_level=g_ascii_strtod(value,NULL);

	if(config.mic_level<0.0)
		config.mic_level=0.0;
	else if(config.mic_level>1.0)
		config.mic_level=1.0;

	return(TRUE);
}

static gboolean config_cb_speaker_level(gchar *option_name,gchar *value,gpointer data,GError **error)
{
	config.speaker_level=g_ascii_strtod(value,NULL);

	if(config.speaker_level<0.0)
		config.speaker_level=0.0;
	else if(config.speaker_level>1.0)
		config.speaker_level=1.0;

	return(TRUE);
}

static gboolean config_play(gchar *option_name,gchar *value,gpointer data,GError **error)
{
	GError *tmp_error=NULL;

	config.mode=0; /* play file mode */
	config.play_file=strdup(value);

	return(TRUE);
}

static void config_show_version(void)
{
	g_printf("(c)2008,2009 by H. Wirtz <dcoredump@gmail.com> && C. Ingels <charles@maisonblv.net>\n");
	g_printf("Version %3.2f build TOBEFIXED\n",VERSION);
#ifdef __DEBUG__
	g_printf("Debug version, build time %s, %s\n", __DATE__, __TIME__);
#endif

	fgcom_exit("",0);
}

static void config_show_audio_devices(void)
{
	if(iaxc_initialize(AUDIO_SYSTEM,DEFAULT_MAX_CALLS)!=0)
		fgcom_exit("Cannot initialize iaxclient!",203);

	config.initialized=1;

	g_printf("Input audio devices:\n");
	config_report_devices (IAXC_AD_INPUT);
	g_printf("\n");
	g_printf("Output audio devices:\n");
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
  g_printf("\t%s\n",devs[current].name);
  for (i = 0; i < ndevs; i++)
    {
      if (devs[i].capabilities & flag && i != current)
        {
          g_printf("\t\t%s\n",devs[i].name);
        }
    }
}
