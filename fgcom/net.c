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
#include <gnet.h>
#include "iaxclient.h"
#include "fgcom.h"
#include "config.h"

extern struct fgcom_config config;

gboolean net_open(gchar *hostname, gint port)
{
	gint rv;
	gchar buffer[1024];
	guint n;

	gnet_init();

	/* Create the address */
	config.fg_addr=gnet_inetaddr_new(hostname,port);
	g_assert(config.fg_addr!=NULL);

	/* Create the socket */
	config.fg_socket=gnet_udp_socket_new_full(config.fg_addr,config.fg_port);
	g_assert(config.fg_socket!=NULL);
}

gboolean net_block_read(gchar *buffer)
{
	gint n=-1;

	/* Receive packet */
	n=gnet_udp_socket_receive(config.fg_socket,buffer,FGCOM_UDP_MAX_BUFFER,NULL);
	if (n<=0)
	{
		printf("Nothing to read from network\n");
		return(FALSE);
	}
	buffer[n]='\0';
	return(TRUE);
}

void net_close(void)
{
	if(config.fg_socket!=NULL)
		gnet_udp_socket_delete(config.fg_socket);
	if(config.fg_addr!=NULL)
		gnet_inetaddr_delete(config.fg_addr);
}
