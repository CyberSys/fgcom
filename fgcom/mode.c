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

#include <glib.h>
#include "iaxclient.h"
#include "fgcom.h"
#include "config.h"

extern struct fgcom_config config;

void mode_fg(void)
{
	gdouble com1frq=-1.0;
	gdouble com2frq=-1.0;
	gint ptt=0;
	GTimer *packet_age;

	/* mode FlightGear and mode InterCom */
	if(config.verbose==TRUE)
		printf("Mode: FlightGear\n");

	alarm(DEFAULT_POSTTION_UPDATE_FREQUENCY);

	if(net_open(config.fg,config.fg_port)==FALSE)
		fgcom_exit("Cannot open connection to FG!\n",567);

	/* start the position update */
	alarm(DEFAULT_POSTTION_UPDATE_FREQUENCY);

	/* create a timer for measuring the packet age */
	packet_age=g_timer_new();
	g_timer_start(packet_age);

	while(TRUE)
	{
		gchar from_fg[FGCOM_UDP_MAX_BUFFER];
		struct fg_data data;

		/* read data from FG */
		if(net_block_read(from_fg)==TRUE)
		{
			if(g_timer_elapsed(packet_age,NULL)>DEFAULT_PACKET_TIMEOUT)
			{
				/* packet is older than DEFAULT_PACKET_TIMEOUT seconds */
				fgcom_hangup("packet too old");
				g_timer_reset(packet_age);
				if(config.verbose==TRUE)
				{
					g_printf("fgcom detected packets older than %d seconds: dropping the call!\n",(int)DEFAULT_PACKET_TIMEOUT);
				}
				sleep(5);
			}
			else
			{
				g_timer_reset(packet_age);
			}
		}
		else
		{
			printf("Read error from FG!\n");
			continue;
		}
		if(config.verbose==TRUE)
			printf(">%s",from_fg);

		/* parse data from FG */
		if(fgcom_parse_data(&data, from_fg)==FALSE)
			printf("Parsing data from FG failed: %s\n",from_fg);

		/* frequency change detection */
		if(data.COM1_FRQ!=com1frq)
		{
			if(config.verbose==TRUE)
				printf("Frequency change %3.3lf->%3.3lf\n",com1frq,data.COM1_FRQ);
			com1frq=data.COM1_FRQ;
			if(config.connected==TRUE)
				fgcom_hangup("frequency change");
			if((fgcom_dial(com1frq))!=TRUE)
			{
				g_printf("Connecting failed, trying again...\n");
				sleep(1);
			}
		}

		/* PTT change detection */
		if(data.PTT!=ptt)
		{
			ptt=data.PTT;
			if(ptt==1)
			{
				if(config.verbose==TRUE)
					printf("PTT pressed.\n");
				printf("PTT-Press:%d\n",iaxc_input_level_set(config.mic_level));
				//iaxc_output_level_set(0.0);
			}
			else if(ptt==0)
			{
				if(config.verbose==TRUE)
					printf("PTT released.\n");
				printf("PTT-Release:%d\n",iaxc_input_level_set(0.0));
				//iaxc_output_level_set(config.speaker_level);
			}
			else
			{
				printf("Unknown PTT state: %d\n",ptt);
			}
		}
	}
}

void mode_atc(void)
{
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

	/* start the position update */
	alarm(DEFAULT_POSTTION_UPDATE_FREQUENCY);

	/* change audio settings */ /* This may be not needed if iaxclient fully supports test mode */
	iaxc_mic_boost_set(config.mic_boost);
	iaxc_input_level_set(config.mic_level);
	iaxc_output_level_set(config.speaker_level);

	while(TRUE)
	{
		if(config.connected==FALSE)
		{
			if((fgcom_dial(config.frequency))!=TRUE)
			if(config.verbose==TRUE)
				printf("Connecting failed, trying again...\n");
			sleep(1);
		}
		else
		{
			sleep(5);
		}
	}
}

void mode_play(void)
{
	/* mode play file */
	if(config.verbose==TRUE)
		printf("Mode: Play\n");

	config.modelname=g_strdup("LIGHTHOUSE");

	/* change audio settings */ /* This may be not needed if iaxclient fully supports test mode */
	iaxc_mic_boost_set(0);
	iaxc_input_level_set(0.0);

	/* consistency checks */
	if(config.callsign==NULL)
		fgcom_exit("callsign must be set for mode Play\n",110);
	if(config.lat<-180.0)
		fgcom_exit("latitude must be set for mode Play\n",110);
	if(config.lon<-180.0)
		fgcom_exit("longtitude must be set for mode Play\n",110);
	if(config.frequency<=0.0)
		fgcom_exit("frequency must be set for mode Play\n",110);

	// load audio file
	if(config.play_file!=NULL)
	{
		if(oggfile_load_ogg_file(config.play_file))
			fgcom_exit("Failed loading ogg file.\n",333);
	}
	else
		fgcom_exit("No audio filename.\n",334);

	while(TRUE)
	{
		if(config.connected==FALSE)
		{
			if((fgcom_dial(config.frequency))!=TRUE)
			{
				if(config.verbose==TRUE)
				g_printf("Connecting failed, trying again...\n");
				sleep(1);
			}
			else
			{
				/* start the position update */
				alarm(DEFAULT_POSTTION_UPDATE_FREQUENCY);
			}
		}
		else
		{
			/* play the sound endless */
			if(config.verbose==TRUE)
				g_printf("Playing %s\n",config.play_file);
			fgcom_send_audio();
		}
	}
}
