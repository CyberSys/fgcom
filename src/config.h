/* configuration data */
struct fgcom_config
{
	/* group main options */
	gchar iax_server[256];
	gboolean verbose;

	/* group VoIP server */
        gchar username[80];
        gchar password[80];
	gchar codec[80];

	/* group FG arguments */
	gchar fg[256];
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
	gchar audio_in[256];
	gchar audio_out[256];
};

int config_parse_cmd_options(int argc, char *argv[]);

static void config_show_version(void);
static void config_show_audio_devices(void);
