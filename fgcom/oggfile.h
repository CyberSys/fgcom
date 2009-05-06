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
 * This file ist partially copied from iaxclient/stresstest/file.[ch]. The
 * original file can be found with
 * "svn co https://iaxclient.svn.sourceforge.net/svnroot/iaxclient iaxclient"
 */

#ifndef __OGGFILE_H__
#define __OGGFILE_H__

#include <glib.h>
#include <ogg/ogg.h>

struct op_node
{
        ogg_packet     *op;
        long           serialno;
        long           timestamp;
        struct op_node *next;
};

struct ogg_stream
{
        struct op_node *first;
        struct op_node *last;
        struct op_node *current;
        long           serialno;
        long           page_ts;
        long           page_count;
        long           base_ts;
        void           *data;
};

static const int SPEEX_FRAME_DURATION = 20;
static const int SPEEX_SAMPLING_RATE = 8000;

int oggfile_load_ogg_file(const char *filename);
ogg_packet *oggfile_get_next_audio_op(GTimeVal now);
#endif