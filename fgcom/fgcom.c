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
#include "mode.h"

extern struct fgcom_config config;

/* Main program */
int main(int argc, char *argv[])
{
	char* home;

	if((home=getenv("HOME"))!=NULL)
	{
		sprintf(tmp,"%s/.fgcomrc",home);
		if(config_parse_cmd_options(tmp,argc,argv)==FALSE)
		{
			fgcom_exit("Program stop due to option errors.",100);
		}
	}

	if(config.verbose==TRUE)
		g_printf("fgcom starting up...\n");

        /* catch signals */
        signal (SIGINT, fgcom_quit);
        signal (SIGQUIT, fgcom_quit);
        signal (SIGHUP, fgcom_quit);
        signal (SIGALRM, fgcom_update_session);

	/* setup iaxclient */
	if(iaxc_initialize(DEFAULT_MAX_CALLS))
		fgcom_exit("cannot initialize iaxclient!",100);
	config.initialized=TRUE;
	iaxc_set_formats(config.codec,IAXC_FORMAT_ULAW|IAXC_FORMAT_ALAW|IAXC_FORMAT_GSM|IAXC_FORMAT_SPEEX);
	iaxc_set_event_callback(fgcom_iaxc_callback);

	if(config.verbose==TRUE)
		g_printf("iaxclient initialized\n");

	/* Setting the audio interface **/
	fgcom_set_audio_interface(config.audio_in,config.audio_out);

	iaxc_millisleep(100);

	/* Registering ? */
	if(config.reg==TRUE)
	{
		if(config.verbose==TRUE)
			g_printf("Trying registration at %s\n",config.iax_server);

		if(config.username!=NULL && config.password!=NULL)
		{
			/* Register client */
			if((config.reg_id=iaxc_register((char *)config.username,(char *)config.password,(char *)config.iax_server))>0)
			{
				if(config.verbose==TRUE)
					g_printf("Registered as %s at %s with id: %d\n",config.username,config.iax_server,config.reg_id);

				iaxc_set_callerid((char *)config.username,"0");
			}
			else
			{
				g_printf("Registering as %s at %s failed: %d\n",config.username,config.iax_server,config.reg_id);
				fgcom_exit("",125);
			}
		}
		else
			fgcom_exit("Username and password are needed for registration",150);
	}
	else
	{
		if(config.verbose==TRUE)
			g_printf("Setting callerid to: %s/%s\n",DEFAULT_FRQ,DEFAULT_USER);
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
	config.mode=MODE_FG;	/* default mode */

	if(config.frequency>0.0 || config.lon>0.0 || config.lat>0.0)
	{
		config.mode=MODE_ATC;

		if(config.play_file!=NULL)
			config.mode=MODE_PLAY;
	}

	/* Handle modes */
	switch(config.mode)
	{
		case MODE_FG:
			/* mode FlightGear and mode InterCom */
			mode_fg();
			break;
		case MODE_ATC:
			/* mode ATC */
			mode_atc();
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

gboolean fgcom_hangup(void)
{
	if(config.verbose==TRUE)
		g_printf("Hangup\n");

	iaxc_dump_call();
	config.connected=FALSE;
	iaxc_millisleep(100);

	return(TRUE);
}

gboolean fgcom_dial(gdouble frequency)
{
	char dest[80];

	if(strlen(config.iax_server)==0)
	{
		g_printf("No iax server given!\n");
		return(FALSE);
	}

	if(config.username!=NULL && config.password!=NULL)
	{
		if(strlen(config.username)!=0 && strlen(config.password)!=0)
		{
			g_snprintf(dest,sizeof(dest),"%s:%s@%s/%02d%-6d",config.username,config.password,config.iax_server,DEFAULT_PRESELECTION,(int)(frequency*1000));
			if(config.verbose==TRUE)
				g_printf("%s:XXXXXXX@%s/%02d%-6d",config.username,config.iax_server,DEFAULT_PRESELECTION,(int)(frequency*1000));
		}
	}
	else
	{
		g_snprintf(dest,sizeof(dest),"%s/%02d%-6d",config.iax_server,DEFAULT_PRESELECTION,(int)(frequency*1000));
		if(config.verbose==TRUE)
			g_printf("Dialing [%s]",dest);
	}

	iaxc_call(dest);
	iaxc_millisleep(100);

	return(TRUE);
}

gboolean fgcom_conference_command(gchar *command, ...)
{
	gchar text[80];
	va_list argPtr;

	if(config.connected!=TRUE)
		return(FALSE);

	va_start(argPtr,command);

	if(g_ascii_strcasecmp(command,"ADD")==0 || g_ascii_strcasecmp(command,"UPDATE")==0)
	{
		gchar *callsign=va_arg(argPtr,gchar *);
		gdouble lon=va_arg(argPtr,gdouble);
		gdouble lat=va_arg(argPtr,gdouble);
		gint alt=va_arg(argPtr,gint);

		g_snprintf(text,sizeof(text),"FGCOM:%s:%s:%f:%f:%d:%s",command,callsign,lon,lat,alt,config.modelname);
	}
	else if(g_ascii_strcasecmp(command,"DEL")==0)
	{
		gchar *callsign=va_arg(argPtr,gchar *);

		g_snprintf(text,sizeof(text),"FGCOM:%s:%s",command,callsign);
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

gboolean fgcom_parse_data(struct fg_data *data, gchar *from_fg)
{
	gchar *tmp;
	gchar tmp_string[256];

	tmp=g_strdup(strtok((char *)from_fg,","));
	while(tmp!=NULL)
	{
		if(sscanf(tmp,"COM1_FRQ=%lf",&data->COM1_FRQ)==1);
		else if(sscanf(tmp,"COM1_SRV=%d",&data->COM1_SRV)==1);
		else if(sscanf(tmp,"COM2_FRQ=%lf",&data->COM2_FRQ)==1);
		else if(sscanf(tmp,"COM2_SRV=%d",&data->COM2_SRV)==1);
		else if(sscanf(tmp,"NAV1_FRQ=%lf",&data->NAV1_FRQ)==1);
		else if(sscanf(tmp,"NAV1_SRV=%d",&data->NAV1_SRV)==1);
		else if(sscanf(tmp,"NAV2_FRQ=%lf",&data->NAV2_FRQ)==1);
		else if(sscanf(tmp,"NAV2_SRV=%d",&data->NAV2_SRV)==1);
		else if(sscanf(tmp,"PTT=%d",&data->PTT)==1);
		else if(sscanf(tmp,"TRANSPONDER=%d",&data->TRANSPONDER)==1);
		else if(sscanf(tmp,"IAS=%d",&data->IAS)==1);
		else if(sscanf(tmp,"GS=%d",&data->GS)==1);
		else if(sscanf(tmp,"LON=%lf",&config.lon)==1);
		else if(sscanf(tmp,"LAT=%lf",&config.lat)==1);
		else if(sscanf(tmp,"ALT=%d",&config.alt)==1);
		else if(sscanf(tmp,"HEAD=%lf",&data->HEAD)==1);
		else if(sscanf(tmp,"CALLSIGN=%s",&tmp_string)==1)
			config.callsign=strdup(tmp_string);
		else if(sscanf(tmp,"MODEL=%s",&tmp_string)==1)
			config.modelname=strdup(tmp_string);
		tmp=strtok(NULL,",");
	}
	return(TRUE);
}

void fgcom_update_session(gint exitcode)
{
	if(config.mode==MODE_ATC)
		config.alt=0.0;

	fgcom_conference_command("UPDATE",config.callsign,config.lon,config.lat,(gint)config.alt);
	alarm(DEFAULT_POSTTION_UPDATE_FREQUENCY);
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

	if(config.fg_socket!=NULL)
		net_close();
	if(config.connected==TRUE)
	{
		fgcom_conference_command("DEL",config.callsign);
		sleep(1);
	}
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
		if(strcmp(devs[i].name,in_dev_name)==0 && devs[i].capabilities & IAXC_AD_INPUT)
		{
			input_id=i;
			if(config.verbose==TRUE)
				g_printf("set input audio: %s[%d]\n",in_dev_name,input_id);
		}
	}

	// set up output audio interface
	for (i = 0; i < ndevs; i++)
	{
		if(strcmp(devs[i].name,out_dev_name)==0 && devs[i].capabilities & IAXC_AD_OUTPUT)
		{
			output_id=i;
			if(config.verbose==TRUE)
				g_printf("set output audio: %s[%d]\n",out_dev_name,output_id);
		}
	}

	iaxc_audio_devices_set(devs[input_id].devID,devs[output_id].devID,devs[output_id].devID);
}
