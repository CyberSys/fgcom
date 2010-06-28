#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "iaxclient.h"
#include "oggfile.h"

/* public prototypes */
void test_exit(char *text, int exitcode);
int hangup(void);
void event_state(int state, char *remote, char *remote_name, char *local, char *local_context);
void event_text(int type, char *message);
void event_register(int id, int reply, int count);
void event_register(int id, int reply, int count);
void event_netstats(struct iaxc_ev_netstats stat);
void event_level(double in, double out);
void event_unknown(int type);

/* private prototypes */
int iaxc_callback(iaxc_event e);
void quit (int exitcode);
void report(char *text);
const char *map_state (int state);
void set_audio_interface(char* in_dev_name, char* out_dev_name);

static int last_state = 0;
static char tmp[1024];
char delim = '\t';
char states[256];
const char *map[] = {
  "unknown", "active", "outgoing", "ringing", "complete", "selected",
  "busy", "transfer", NULL
};
int ring_id=0;

/* Main program */
int main(int argc, char *argv[])
{
	char dest[80];
	double frq=0.0;

        signal (SIGINT, quit);
        signal (SIGQUIT, quit);
        signal (SIGHUP, quit);

	/* setup iaxclient */
	if(iaxc_initialize(1))
		test_exit("cannot initialize iaxclient!",100);
	iaxc_set_formats(IAXC_FORMAT_SPEEX,IAXC_FORMAT_ULAW|IAXC_FORMAT_ALAW|IAXC_FORMAT_SPEEX);
	iaxc_set_event_callback(iaxc_callback);

	/* Start the IAX client */
	iaxc_start_processing_thread();

	// setup testmode
	iaxc_set_test_mode(1);

	/* Dial */
	if(argc>1)
	{
		if((frq=atof(argv[1]))>0.0)
			snprintf(dest,sizeof(dest),"fgcom1.parasitstudio.de/02%06d",(int)(frq*1000));
		else
		{
			printf("Unknown parameter: %s\n");
			test_exit("Stopping now.",199);
		}
	}
	else
		snprintf(dest,sizeof(dest),"fgcom1.parasitstudio.de/02123450");

	printf("Dialing [%s]",dest);
	iaxc_call(dest);

	/* Load ogg file */
	oggfile_load_ogg_file("lighthouse.ogg");

	while(1)
	{
		GTimeVal now;
		ogg_packet *op;

		g_get_current_time(&now);
		op=oggfile_get_next_audio_op(now);
		if(op!=NULL && op->bytes>0)
		{
			iaxc_push_audio(op->packet, op->bytes,SPEEX_SAMPLING_RATE*SPEEX_FRAME_DURATION/1000);
		}
	}

	test_exit("Exiting after 3600 seconds.",0);
}

void test_exit(char *text, int exitcode)
{
	if(strlen(text)>0)
		fprintf(stderr,"%s\n",text);
	quit(exitcode);
}

int hangup(void)
{
	printf("Hangup\n");

	iaxc_dump_call();
	iaxc_millisleep(100);

	return(1);
}

int iaxc_callback (iaxc_event e)
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

void quit (int exitcode)
{
	char text[256];

	printf("shutting down - please wait...\n");
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

  snprintf (tmp, sizeof (tmp),
            "S%c0x%x%c%s%c%.50s%c%.50s%c%.50s%c%.50s", delim, state,
            delim, map_state (state), delim, remote, delim, remote_name,
            delim, local, delim, local_context);
  report (tmp);
}

void event_text(int type, char *message)
{
  snprintf (tmp, sizeof (tmp), "T%c%d%c%.200s", delim, type, delim, message);
  printf("%s\n",message);
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
      printf("Registering denied!\n");
      break;
    case IAXC_REGISTRATION_REPLY_TIMEOUT:
      reason = "timeout";
      break;
    default:
      reason = "unknown";
    }
  snprintf (tmp, sizeof (tmp), "R%c%d%c%s%c%d", delim, id, delim,
            reason, delim, count);
  report (tmp);
}

void event_netstats(struct iaxc_ev_netstats stat)
{
  struct iaxc_netstat local = stat.local;
  struct iaxc_netstat remote = stat.remote;
  snprintf (tmp, sizeof (tmp),
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
  snprintf (tmp, sizeof (tmp), "L%c%.1f%c%.1f", delim, in, delim, out);
  report (tmp);
}

void event_unknown(int type)
{
  snprintf (tmp, sizeof (tmp), "U%c%d", delim, type);
  report (tmp);
}

void report(char *text)
{
	printf("%s\n",text);
      	fflush (stdout);
}

const char *map_state (int state)
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

void set_audio_interface(char* in_dev_name, char* out_dev_name)
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
                        printf("set input audio: %s\n",in_dev_name);
                }
        }

        // set up output audio interface
        for (i = 0; i < ndevs; i++)
        {
                if(strcmp(devs[i].name,out_dev_name)==0 && devs[i].capabilities & IAXC_AD_OUTPUT)
                {
                        output_id=i;
			ring_id=i;
                        printf("set output audio: %s\n",out_dev_name);
                }
        }

        iaxc_audio_devices_set(devs[input_id].devID,devs[output_id].devID,devs[output_id].devID);
}
