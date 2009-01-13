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
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include "iaxclient.h"
#include "fgcom.h"
#include "config.h"
#include "event.h"
#include "oggfile.h"

extern struct fgcom_config config;

/* Main program */
int main(int argc, char *argv[])
{
	if(config_parse_cmd_options("/home/wirtz/.fgcomrc",argc,argv)==FALSE)
	{
		fgcom_exit("Program stop due to option errors.",100);
	}

	if(config.verbose==TRUE)
		printf("fgcom starting up...\n");

        /* catch signals */
        signal (SIGINT, fgcom_quit);
        signal (SIGQUIT, fgcom_quit);
        signal (SIGTERM, fgcom_quit);
        signal (SIGALRM, fgcom_update_session);

	/* setup iaxclient */
	if(iaxc_initialize(DEFAULT_MAX_CALLS))
		fgcom_exit("cannot initialize iaxclient!",100);
	config.initialized=TRUE;
	iaxc_set_formats(config.codec,IAXC_FORMAT_ULAW|IAXC_FORMAT_ALAW|IAXC_FORMAT_GSM|IAXC_FORMAT_SPEEX);
	iaxc_set_event_callback(fgcom_iaxc_callback);

	if(config.verbose==TRUE)
		printf("iaxclient initialized\n");

	iaxc_millisleep(100);

	/* Registering ? */
	if(config.reg==TRUE)
	{
		if(config.verbose==TRUE)
			printf("Trying registration at %s\n",config.iax_server);

		if(config.username!=NULL && config.password!=NULL)
		{
			/* Register client */
			if((config.reg_id=iaxc_register((char *)config.username,(char *)config.password,(char *)config.iax_server))>0)
			{
				if(config.verbose==TRUE)
					printf("Registered as %s at %s with id: %d\n",config.username,config.iax_server,config.reg_id);

				iaxc_set_callerid((char *)config.username,"0");
			}
			else
			{
				printf("Registering as %s at %s failed: %d\n",config.username,config.iax_server,config.reg_id);
				fgcom_exit("",125);
			}
		}
		else
			fgcom_exit("Username and password are needed for registration",150);
	}
	else
	{
		if(config.verbose==TRUE)
			printf("Setting callerid to: %s/%s\n",DEFAULT_FRQ,DEFAULT_USER);
		iaxc_set_callerid((char *)DEFAULT_USER,(char *)DEFAULT_FRQ);
	}

	/* Start the IAX client */
	iaxc_start_processing_thread();

	/* set audio levels */
	if(config.mic_level>0.0)
		iaxc_input_level_set(config.mic_level);
	if(config.mic_level>0.0)
		iaxc_output_level_set(config.speaker_level);
	if(config.mic_boost==TRUE)
		iaxc_mic_boost_set(1);
	else
		iaxc_mic_boost_set(0);

	/* mode decission */
	config.mode=MODE_FG;
	if(config.frequency>0.0 || config.lon>0.0 || config.lat>0.0)
	{
		config.mode=MODE_ATC;

		if(config.play_file!=NULL)
			config.mode=MODE_PLAY;
	}

	/* Handle modes */
	switch(config.mode)
	{
		case MODE_PLAY:
			/* mode play file */
			if(config.verbose==TRUE)
				printf("Mode: Play\n");

			config.modelname=g_strdup("LIGHTHOUSE");

			/* change audio settings */ /* is this needed??? */
			/* iaxc_mic_boost_set(0);
			iaxc_input_level_set(0.0); */

			/* consistency checks */
			if(config.callsign==NULL)
				fgcom_exit("callsign must be set for mode Play\n",110);
			if(config.lat<-180.0)
				fgcom_exit("latitude must be set for mode Play\n",110);
			if(config.lon<-180.0)
				fgcom_exit("longtitude must be set for mode Play\n",110);
			if(config.frequency<=0.0)
				fgcom_exit("frequency must be set for mode Play\n",110);

			/* start the position update */
			alarm(DEFAULT_POSTTION_UPDATE_FREQUENCY);

			/* dial and conect */
			config.connected=fgcom_dial(config.frequency);

			/* tell our position */
			fgcom_conference_command("ADD",config.callsign,config.lon,config.lat,100);

			/* play the sound endless */
			while(1)
			{
				fgcom_send_audio((char *)config.play_file);
			}

			break;
		case MODE_FG:
			/* mode FlightGear and mode InterCom */
			if(config.verbose==TRUE)
				printf("Mode: FlightGear\n");

			/* start the position update */
			alarm(DEFAULT_POSTTION_UPDATE_FREQUENCY);

			break;
		case MODE_ATC:
			/* mode ATC */
			if(config.verbose==TRUE)
				printf("Mode: ATC\n");

			config.modelname=g_strdup("ATC");

			/* consistency checks */
			if(config.callsign==NULL)
				fgcom_exit("callsign must be set for mode ATC\n",110);
			if(config.lat<-180.0)
				fgcom_exit("latitude must be set for mode ATC\n",110);
			if(config.lon<-180.0)
				fgcom_exit("longtitude must be set for mode ATC\n",110);
			if(config.frequency<=0.0)
				fgcom_exit("frequency must be set for mode ATC\n",110);

			config.connected=fgcom_dial(config.frequency);

			fgcom_conference_command("ADD",config.callsign,config.lon,config.lat,100);

			/* start the position update */
			alarm(DEFAULT_POSTTION_UPDATE_FREQUENCY);

			while(1)
			{
				sleep(10000);
			}
			break;
		default:
			fgcom_exit("Cannot detect mode! Please check your commandline options or configuration file and read the manual.\n",876);
			break;

	}
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

/****************************************************************************
 *  private functions
 ****************************************************************************/

static gboolean fgcom_dial(gdouble frequency)
{
	char dest[80];

	if(strlen(config.iax_server)==0)
	{
		g_printf("Cannot connect to %d on server %s\n",frequency,config.iax_server);
		return(FALSE);
	}

	if(config.username!=NULL && config.password!=NULL)
	{
		g_snprintf(dest,sizeof(dest)-1,"%s:%s@%s/%02d%-6d",config.username,config.password,config.iax_server,DEFAULT_PRESELECTION,(int)(frequency*1000));
	}
	else
		g_snprintf(dest,sizeof(dest)-1,"%s/%02d%-6d",config.iax_server,DEFAULT_PRESELECTION,(int)(frequency*1000));

	if(config.verbose==TRUE)
		g_printf("Dialing [%s]",dest);

	iaxc_call(dest);
	iaxc_millisleep(100);

	return(TRUE);
}

static gboolean fgcom_conference_command(gchar *command, ...)
{
	gchar text[80];
	va_list argPtr;

	va_start(argPtr,command);

	if(g_ascii_strcasecmp(command,"ADD")==0 || g_ascii_strcasecmp(command,"UPDATE")==0)
	{
		gchar *callsign=va_arg(argPtr,gchar *);
		gdouble lon=va_arg(argPtr,gdouble);
		gdouble lat=va_arg(argPtr,gdouble);
		gint alt=va_arg(argPtr,gint);

		g_snprintf(text,sizeof(text)-1,"FGCOM:%s:%s:%f:%f:%d:%s",command,callsign,lon,lat,alt,config.modelname);
	}
	else if(g_ascii_strcasecmp(command,"DEL")==0)
	{
		gchar *callsign=va_arg(argPtr,gchar *);

		g_snprintf(text,sizeof(text)-1,"FGCOM:%s:%s",command,callsign);
	}
	else
	{
		g_printf("Command %s unknown!\n",command);
		va_end(argPtr);
		return(FALSE);
	}

	if(config.verbose==TRUE)
		g_printf("Sending [%s]\n",text);
	iaxc_send_text((char *)text);

	va_end(argPtr);

	return(TRUE);
}

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

	if(config.connected==TRUE)
		fgcom_conference_command("DEL",config.callsign);
	if(config.reg_id>0)
	{
		if(config.verbose==TRUE)
			g_printf("Unregistering VoIP client\n");
    		iaxc_unregister(config.reg_id);
		iaxc_millisleep(100);
	}
	if(config.initialized==TRUE)
	{
		if(config.verbose==TRUE)
			g_printf("Shutdown VoIP client\n");
		iaxc_stop_processing_thread();
		iaxc_shutdown ();
	}
	exit((int)exitcode);
}

static gboolean fgcom_send_audio(gchar *filename)
{
	if(filename!=NULL)
	{
		if(load_ogg_file(filename))
		{
			fgcom_exit("Failed loading ogg file.\n",333);
                }
        }
	else
		return(FALSE);

	while(1)
        {
		GTimeVal now;
		ogg_packet *op;

		g_get_current_time(&now);
		op=get_next_audio_op(now);
		if(op!=NULL&&op->bytes>0)
		{
			iaxc_push_audio(op->packet, op->bytes,SPEEX_SAMPLING_RATE*SPEEX_FRAME_DURATION/1000);
		}
	}
}

static void fgcom_update_session(gint exitcode)
{
	fgcom_conference_command("UPDATE",config.callsign,config.lon,config.lat,100);
	alarm(DEFAULT_POSTTION_UPDATE_FREQUENCY);
}
