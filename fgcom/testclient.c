#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include "iaxclient.h"

#define DEFAULT_MAX_CALLS 1
#define DEFAULT_FRQ "02910000"
#define DEFAULT_PRESELECTION 2
#define DEFAULT_VOIP_SERVER "fgcom1.parasitstudio.de\0"
#define DEFAULT_MIC_LEVEL 0.5
#define DEFAULT_SPEAKER_LEVEL 0.5

#define FGCOM_UDP_MAX_BUFFER 1024

/* public prototypes */
void fgcom_exit(gchar *text, gint exitcode);
void fgcom_send_audio(void);
gboolean fgcom_dial(gdouble frequency);
gboolean fgcom_hangup(void);
gboolean fgcom_conference_command(gchar *command, ...);
void event_state(int state, char *remote, char *remote_name, char *local, char *local_context);
void event_text(int type, char *message);
void event_register(int id, int reply, int count);
void event_register(int id, int reply, int count);
void event_netstats(struct iaxc_ev_netstats stat);
void event_level(double in, double out);
void event_unknown(int type);

/* private prototypes */
static int fgcom_iaxc_callback(iaxc_event e);
static void fgcom_quit (gint exitcode);
static void report(char *text);
static const char *map_state (int state);
static void fgcom_set_audio_interface(char* in_dev_name, char* out_dev_name);

static int last_state = 0;
static char tmp[1024];
static char delim = '\t';
static char states[256];
static const char *map[] = {
  "unknown", "active", "outgoing", "ringing", "complete", "selected",
  "busy", "transfer", NULL
};

/* Main program */
int main(int argc, char *argv[])
{
        signal (SIGINT, fgcom_quit);
        signal (SIGQUIT, fgcom_quit);
        signal (SIGHUP, fgcom_quit);

	/* setup iaxclient */
	if(iaxc_initialize(DEFAULT_MAX_CALLS))
		fgcom_exit("cannot initialize iaxclient!",100);
	iaxc_set_formats(IAXC_FORMAT_SPEEX,IAXC_FORMAT_ULAW|IAXC_FORMAT_ALAW|IAXC_FORMAT_GSM|IAXC_FORMAT_SPEEX);
	iaxc_set_event_callback(fgcom_iaxc_callback);

	fgcom_set_audio_interface("default","default");

	iaxc_millisleep(100);

	/* Start the IAX client */
	iaxc_start_processing_thread();

	fgcom_dial(123.450);

	sleep(3600);
	fgcom_exit("Exiting after 3600 seconds.",0);
}

/****************************************************************************
 *  public functions
 ****************************************************************************/

void fgcom_exit(gchar *text, gint exitcode)
{
	if(strlen(text)>0)
		g_fprintf(stderr,"%s\n",text);
	fgcom_quit(exitcode);
}

gboolean fgcom_hangup(void)
{
	g_printf("Hangup\n");

	iaxc_dump_call();
	iaxc_millisleep(100);

	return(TRUE);
}

gboolean fgcom_dial(gdouble frequency)
{
	char dest[80];

	g_snprintf(dest,sizeof(dest),"%s/%02d%-6d",DEFAULT_VOIP_SERVER,DEFAULT_PRESELECTION,(int)(frequency*1000));
	g_printf("Dialing [%s]",dest);

	iaxc_call(dest);
	iaxc_millisleep(100);

	return(TRUE);
}

/****************************************************************************
 *  private functions
 ****************************************************************************/

static int fgcom_iaxc_callback (iaxc_event e)
{
  switch (e.type)
    {
    case IAXC_EVENT_LEVELS:
      event_level (e.ev.levels.input, e.ev.levels.output);
      break;
    case IAXC_EVENT_TEXT:
      event_text (e.ev.text.type, e.ev.text.message);
      break;
    case IAXC_EVENT_STATE:
      event_state (e.ev.call.state, e.ev.call.remote, e.ev.call.remote_name,
                   e.ev.call.local, e.ev.call.local_context);
      break;
    case IAXC_EVENT_NETSTAT:
      event_netstats (e.ev.netstats);
    case IAXC_EVENT_REGISTRATION:
      event_register (e.ev.reg.id, e.ev.reg.reply, e.ev.reg.msgcount);
      break;
    default:
      event_unknown (e.type);
      break;
    }
  return 1;
}

static void fgcom_quit (gint exitcode)
{
	gchar text[256];

	g_printf("shutting down - please wait...\n");

	g_printf("Unregistering VoIP client\n");
	iaxc_millisleep(100);
	g_printf("Shutdown VoIP client\n");
	iaxc_stop_processing_thread();
	iaxc_shutdown ();
	exit((int)exitcode);
}

void event_state(int state, char *remote, char *remote_name, char *local, char *local_context)
{
  last_state = state;
  /* This is needed for auto-reconnect */
  if (state == 0)
    {
      /* FIXME: we should wake up the main thread somehow */
      /* in fg mode the next incoming packet will do that anyway */
    }

  g_snprintf (tmp, sizeof (tmp),
            "S%c0x%x%c%s%c%.50s%c%.50s%c%.50s%c%.50s", delim, state,
            delim, map_state (state), delim, remote, delim, remote_name,
            delim, local, delim, local_context);
  report (tmp);
}

void event_text(int type, char *message)
{
  g_snprintf (tmp, sizeof (tmp), "T%c%d%c%.200s", delim, type, delim, message);
  g_printf("%s\n",message);
  report (tmp);
}

void event_register(int id, int reply, int count)
{
  const char *reason;
  switch (reply)
    {
    case IAXC_REGISTRATION_REPLY_ACK:
      reason = "accepted";
      break;
    case IAXC_REGISTRATION_REPLY_REJ:
      reason = "denied";
      /* fgcom_exit("Registering denied",201);*/
      g_printf("Registering denied!\n");
      break;
    case IAXC_REGISTRATION_REPLY_TIMEOUT:
      reason = "timeout";
      break;
    default:
      reason = "unknown";
    }
  g_snprintf (tmp, sizeof (tmp), "R%c%d%c%s%c%d", delim, id, delim,
            reason, delim, count);
  report (tmp);
}

void event_netstats(struct iaxc_ev_netstats stat)
{
  struct iaxc_netstat local = stat.local;
  struct iaxc_netstat remote = stat.remote;
  g_snprintf (tmp, sizeof (tmp),
            "N%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d%c%d",
            delim, stat.callNo, delim, stat.rtt,
            delim, local.jitter, delim, local.losspct, delim,
            local.losscnt, delim, local.packets, delim, local.delay,
            delim, local.dropped, delim, local.ooo, delim,
            remote.jitter, delim, remote.losspct, delim, remote.losscnt,
            delim, remote.packets, delim, remote.delay, delim,
            remote.dropped, delim, remote.ooo);
  report (tmp);
}

void event_level(double in, double out)
{
  g_snprintf (tmp, sizeof (tmp), "L%c%.1f%c%.1f", delim, in, delim, out);
  report (tmp);
}

void event_unknown(int type)
{
  g_snprintf (tmp, sizeof (tmp), "U%c%d", delim, type);
  report (tmp);
}

/****************************************************************************
 *  private functions
 ****************************************************************************/

static void report(char *text)
{
	g_printf("%s\n",text);
      	fflush (stdout);
}

static const char *map_state (int state)
{
  int i, j;
  int next = 0;
  *states = '\0';
  if (state == 0)
    {
      return "free";
    }
  for (i = 0, j = 1; map[i] != NULL; i++, j <<= 1)
    {
      if (state & j)
        {
          if (next)
            strcat (states, ",");
          strcat (states, map[i]);
          next = 1;
        }
    }
  return states;
}

static void fgcom_set_audio_interface(char* in_dev_name, char* out_dev_name)
{
        struct iaxc_audio_device *devs;       /* audio devices */
        int ndevs;                    /* audio dedvice count */
        int input, output, ring;      /* audio device id's */
        int input_id, output_id;
        int i;

        iaxc_audio_devices_get (&devs, &ndevs, &input, &output, &ring);

        // set up input audio interface
        for (i = 0; i < ndevs; i++)
        {
                if(strcmp(devs[i].name,in_dev_name)==0 && devs[i].capabilities &
 IAXC_AD_INPUT)
                {
                        input_id=i;
                        g_printf("set input audio: %s\n",in_dev_name);
                }
        }

        // set up output audio interface
        for (i = 0; i < ndevs; i++)
        {
                if(strcmp(devs[i].name,out_dev_name)==0 && devs[i].capabilities & IAXC_AD_OUTPUT)
                {
                        output_id=i;
                        g_printf("set output audio: %s\n",out_dev_name);
                }
        }

        iaxc_audio_devices_set(devs[input_id].devID,devs[output_id].devID,devs[output_id].devID);
}

