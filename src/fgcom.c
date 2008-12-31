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
#include "iaxclient.h"
#include "fgcom.h"
#include "config.h"
#include "event.h"

extern struct fgcom_config config;

/****************************************************************************
 *  public functions
 ****************************************************************************/

void fgcom_exit(char *text, int exitcode)
{
	if(strlen(text)>0)
		fprintf(stderr,"%s\n",text);
	fgcom_quit(exitcode);
}

/* Main program */
int main(int argc, char *argv[])
{
	char dest[30];
	char text[256];
	char callsign[20];
	char model[80];
	float lon;
	float lat;
	int alt;

	strcpy(callsign,"D-CORE");
	strcpy(model,"Aircraft/Dragonfly/Models/Dragonfly.xml");
	lon=52.475449;
	lat=13.415937;
	alt=123;

        /* catch signals */
        signal (SIGINT, fgcom_quit);
        signal (SIGQUIT, fgcom_quit);
        signal (SIGTERM, fgcom_quit);

	config_parse_cmd_options("/home/wirtz/.fgcomrc",argc,argv);

	if(iaxc_initialize(DEFAULT_MAX_CALLS))
		fgcom_exit("cannot initialize iaxclient!",100);
	config.initialized=TRUE;

	iaxc_set_callerid((char *)DEFAULT_USER,(char *)DEFAULT_FRQ);
	iaxc_set_formats (DEFAULT_IAX_CODEC, IAXC_FORMAT_ULAW | IAXC_FORMAT_GSM | IAXC_FORMAT_SPEEX);
	iaxc_set_event_callback (fgcom_iaxc_callback);
        iaxc_start_processing_thread ();

	/* Register client */
	if(strlen(config.username)>0 && strlen(config.password))
	{
		config.reg_id=iaxc_register((char *)config.username,(char *)config.password,(char *)config.iax_server);

		if(config.verbose==TRUE)
			printf("Registered VoIP client with id: %d\n",config.reg_id);
	}

	//sprintf(dest,"%s:%s@%s/%s",(char *)DEFAULT_USER,(char *)DEFAULT_PASSWORD,(char *)DEFAULT_VOIP_SERVER,(char *)DEFAULT_FRQ);
	sprintf(dest,"%s/%s",(char *)DEFAULT_VOIP_SERVER,(char *)DEFAULT_FRQ);
        iaxc_call(dest);
	iaxc_millisleep(100);

	sprintf(text,"FGCOM:%s:%s:%f:%f:%d:%s","ADD",(char *)DEFAULT_USER,lon,lat,alt,model);
		printf("Send: [%s]\n",text);
		iaxc_send_text(text);

	while(1)
	{
		sprintf(text,"FGCOM:%s:%s:%f:%f:%d:%s","UPDATE",(char *)DEFAULT_USER,lon,lat,alt,model);
		printf("Send: [%s]\n",text);
		iaxc_send_text(text);
		sleep(5);
	}
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

static void fgcom_quit (int exitcode)
{
	char text[256];

	if(config.connected>0)
	{
		sprintf(text,"FGCOM:%s:%s","DEL",(char *)DEFAULT_USER);
		if(config.verbose==TRUE)
			printf("Send: [%s]\n",text);
		iaxc_send_text(text);
	}
	if(config.reg_id>0)
	{
		if(config.verbose==TRUE)
			printf("Unregistering VoIP client\n");
    		iaxc_unregister(config.reg_id);
		iaxc_millisleep(100);
	}
	if(config.initialized>=0)
	{
		if(config.verbose==TRUE)
			printf("Shutdown VoIP client\n");
		iaxc_shutdown ();
	}
	exit(exitcode);
}
