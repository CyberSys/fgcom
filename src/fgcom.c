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

        /* catch signals */
        signal (SIGINT, fgcom_quit);
        signal (SIGQUIT, fgcom_quit);
        signal (SIGTERM, fgcom_quit);

	/* setup iaxclient */
	if(iaxc_initialize(DEFAULT_MAX_CALLS))
		fgcom_exit("cannot initialize iaxclient!",100);
	config.initialized=TRUE;
	iaxc_set_formats(config.codec,IAXC_FORMAT_ULAW|IAXC_FORMAT_ALAW|IAXC_FORMAT_GSM|IAXC_FORMAT_SPEEX);
	iaxc_set_event_callback(fgcom_iaxc_callback);
	iaxc_start_processing_thread();

	/* Registering ? */
	if(config.reg==TRUE)
	{
		if(config.username!=NULL && config.password!=NULL)
		{
			/* Register client */
			if((config.reg_id=iaxc_register((char *)config.username,(char *)config.password,(char *)config.iax_server))>0)
			{
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
		iaxc_set_callerid((char *)DEFAULT_USER,(char *)DEFAULT_FRQ);
	}

	if(config.mode<2)
	{
		/* mode FlightGear and mode InterCom */
		printf("TODO\n");
	}
	else 
	{
		/* mode ATC */
		config.modelname=g_strdup("ATC");

		/* consistency checks */
		if(strlen(config.callsign)<0)
			fgcom_exit("Callsign must be set for mode ATC\n",110);
		if(config.atc_lat<-180.0)
			fgcom_exit("ATC latitude must be set for mode ATC\n",110);
		if(config.atc_lon<-180.0)
			fgcom_exit("ATC longtitude must be set for mode ATC\n",110);
		if(config.atc_frequency<=0.0)
			fgcom_exit("ATC frequency must be set for mode ATC\n",110);

		config.connected=fgcom_dial(config.atc_frequency);

		fgcom_conference_command("ADD",config.callsign,config.atc_lon,config.atc_lat,100);

		while(1)
		{
			sleep(5);
			fgcom_conference_command("UPDATE",config.callsign,config.atc_lon,config.atc_lat,100);
		}
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
		g_snprintf(dest,sizeof(dest)-1,"%s:%s@%s/%02d%-6d",config.username,config.password,config.iax_server,DEFAULT_PRESELECTION,(int)frequency*1000);
	}
	else
		g_snprintf(dest,sizeof(dest)-1,"%s/%02d%-6d",config.iax_server,DEFAULT_PRESELECTION,(int)frequency*1000);

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
	{
		fgcom_conference_command("DEL",config.callsign);
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
		struct timeval now = fgcom_get_now();

		ogg_packet *op;

		op=get_next_audio_op(now);
		if(op!=NULL&&op->bytes>0)
		{
			iaxc_push_audio(op->packet, op->bytes,SPEEX_SAMPLING_RATE*SPEEX_FRAME_DURATION/1000);
	}
}

static struct timeval *fgcom_get_now(void)
{
        struct timeval tv;
#ifdef WIN32
        FILETIME ft;
        LARGE_INTEGER li;
        __int64 t;
        static int tzflag;
        const __int64 EPOCHFILETIME = 116444736000000000i64;

        GetSystemTimeAsFileTime(&ft);
        li.LowPart  = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t  = li.QuadPart;       /* In 100-nanosecond intervals */
        t -= EPOCHFILETIME;     /* Offset to the Epoch time */
        t /= 10;                /* In microseconds */
        tv.tv_sec  = (long)(t / 1000000);
        tv.tv_usec = (long)(t % 1000000);
#else
        gettimeofday(&tv, 0);
#endif
        return(tv);
}

