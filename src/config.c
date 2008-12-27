#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "config.h"

/* global configuration structure */
struct fgcom_config config;

static GOptionEntry main_entries[] = 
{
  { "voipserver", 's', 0, G_OPTION_ARG_STRING, &config.iax_server, "set the server address for VoIP", NULL },
  { "verbose", 'v', 0, G_OPTION_ARG_NONE, &config.verbose, "show more information while working", NULL },
  { "version", 'V', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_show_version, "show version information", NULL },
  { "username", 'U', 0, G_OPTION_ARG_STRING, &config.username, "set the username for registration", NULL },
  { "password", 'P', 0, G_OPTION_ARG_STRING, &config.password, "set the password for registration", NULL },
  { "codec", 'C', 0, G_OPTION_ARG_STRING, &config.codec, "use one of the following codecs:\n\t\t\t\tspeex (default)\n\t\t\t\tgsm\n\t\t\t\tg711a\n\t\t\t\tg711u", NULL },
  { "fg", 'f', 0, G_OPTION_ARG_STRING, &config.fg, "set the address for FlightGear", NULL },
  { "fgport", 'p', 0, G_OPTION_ARG_INT, &config.fg_port, "set the port number for FlightGear", NULL },
  { "fgcopilotid", 'i', 0, G_OPTION_ARG_INT, &config.fg_copilot_conference_id, "set the id for copilot conference", NULL },
  { "atc", 'a', 0, G_OPTION_ARG_DOUBLE, &config.atc_frequency, "set the frequency for standalone (ATC) mode" , NULL },
  { "atclon", 'O', 0, G_OPTION_ARG_DOUBLE, &config.atc_lon, "set longtitude for standalone (ATC) mode", NULL },
  { "atclat", 'A', 0, G_OPTION_ARG_DOUBLE, &config.atc_lat, "set latitude for standalone (ATC) mode", NULL },
  { "micboost", 'm', 0, G_OPTION_ARG_NONE, &config.mic_boost, "enable mic boost", NULL },
  { "miclevel", 'L', 0, G_OPTION_ARG_DOUBLE, &config.mic_level, "set mic level (0.0-1.0)", NULL },
  { "speakerlevel", 'l', 0, G_OPTION_ARG_DOUBLE, &config.speaker_level, "set speaker level (0.0-1.0)", NULL },
  { "audio-in", 'i', 0, G_OPTION_ARG_STRING, &config.audio_in, "audio device for input", NULL },
  { "audio-out", 'o', 0, G_OPTION_ARG_STRING, &config.audio_out, "audio device for output", NULL },
  { "list-audio", NULL, G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (GOptionArgFunc)config_show_audio_devices, "show audio devices", NULL },
  { NULL }
};

int config_parse_cmd_options(int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context;

	context = g_option_context_new("- real radio simulation based on VoIP");
	g_option_context_add_main_entries(context, main_entries, NULL);

	if (!g_option_context_parse (context, &argc, &argv, &error))
	{
		g_print ("option parsing failed: %s\n", error->message);
		return(-1);
	}

	return(0);
}

static void config_show_version(void)
{
	printf("(c)2008 by H. Wirtz <dcoredump@gmail.com> && C. Ingels <charles@maisonblv.net>\n");
	//printf("Version %3.2f build %d\n",VERSION, SVN_BUILD);
	printf("Version %3.2f build %d\n",0,0);

	//fgcom_exit(0);
}

static void config_show_audio_devices(void)
{
	printf("TODO...\n");
}
