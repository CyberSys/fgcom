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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <oggz/oggz.h>

#include "oggfile.h"

#ifdef __GNUC__
void g_printf(const char * fmt, ...) __attribute__ ((format (printf, 1, 2)));
#else
void g_printf(const char * fmt, ...);
#endif

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

static struct ogg_stream *audio_stream;

static struct op_node *
create_node(ogg_packet *op, long serialno, long timestamp)
{
	struct op_node *node;

	node = malloc(sizeof(struct op_node));
	node->timestamp = timestamp;
	node->serialno = serialno;
	node->op = malloc(sizeof(*op));
	memcpy(node->op, op, sizeof(*op));
	node->op->packet = malloc(op->bytes);
	memcpy(node->op->packet, op->packet, op->bytes);

	return node;
}

static void
append_node(struct ogg_stream *os, struct op_node *node)
{
	if ( os->first == NULL )
	{
		if ( os->last != NULL )
		{
			g_printf("Queue inconsistency, bailing...\n");
			return;
		}
		os->first = node;
		os->last = node;
		node->next = NULL;
	} else
	{
		if ( os->last == NULL )
		{
			g_printf("Queue inconsistency, bailing...\n");
			return;
		}
		os->last->next = node;
		os->last = node;
		node->next = NULL;
	}
}

static int
read_speex_cb(OGGZ *oggz, ogg_packet *op, long serialno, void *data)
{
	static int cnt = 0;
	const long timestamp = audio_stream->page_ts +
		audio_stream->page_count * SPEEX_FRAME_DURATION;

	audio_stream->page_count++;

	cnt++;

	// Ignore the first two packets, they are headers
	if ( cnt > 2 )
	{
		struct op_node *node = create_node(op, serialno, timestamp);
		append_node(audio_stream, node);
	}

	return 0;
}

static int
read_cb(OGGZ *oggz, ogg_packet *op, long serialno, void *data)
{
	struct theora_headers *th;

	const char theoraId[] = "\x80theora";
	const char speexId[] = "Speex   ";

	if ( memcmp(op->packet, speexId, strlen(speexId)) == 0 )
	{
		oggz_set_read_callback(oggz, serialno, read_speex_cb, NULL);
		audio_stream->serialno = serialno;
		read_speex_cb(oggz, op, serialno, data);
	} else
	{
		g_printf("Got unknown ogg packet, serialno=%d, size=%d, "
				"packetno=%d, granulepos=%d\n",
				serialno, op->bytes,
				op->packetno, op->granulepos);
	}
	return 0;
}

static int
read_page_cb(OGGZ *oggz, const ogg_page *og, long serialno, void *data)
{
	if ( serialno == audio_stream->serialno )
	{
		audio_stream->page_ts =
			ogg_page_granulepos((ogg_page *)og) * 1000 /
			SPEEX_SAMPLING_RATE;
		audio_stream->page_count = 0;
	}
	return 0;
}

int
load_ogg_file(const char *filename)
{
	OGGZ *oggz;

	oggz = oggz_open(filename, OGGZ_READ | OGGZ_AUTO);
	if ( oggz == NULL )
	{
		g_printf("Error opening ogg file\n");
		return -1;
	}
	g_printf("Successfully opened ogg file %s\n", filename);

	// Initialize internal streams
	audio_stream = calloc(1, sizeof(struct ogg_stream));

	oggz_set_read_callback(oggz, -1, read_cb, NULL);
	oggz_set_read_page(oggz, -1, read_page_cb, NULL);

	oggz_run(oggz);

	oggz_close(oggz);

	return 0;
}

static ogg_packet * get_next_op(struct ogg_stream *os, GTimeVal tv)
{
	ogg_packet *op = 0;
	long time_now;

	if ( !os )
		return NULL;

	time_now = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	if ( !os->current )
	{
		if ( !os->first )
			return NULL;

		// point to the beginning of the stream and reset the time base
		os->base_ts = time_now;
		os->current = os->first;
	}

	if ( os->current->timestamp < time_now - os->base_ts )
	{
		op = os->current->op;
		os->current = os->current->next;
	}

	return op;
}

ogg_packet * get_next_audio_op(GTimeVal now)
{
	return get_next_op(audio_stream, now);
}

int audio_is_eos()
{
	return audio_stream->current == NULL;
}

