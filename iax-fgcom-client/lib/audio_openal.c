/*
 * iaxclient: a cross-platform IAX softphone library
 *
 * Copyrights:
 * Copyright (C) 2003-2006, Horizon Wimba, Inc.
 * Copyright (C) 2007, Wimba, Inc.
 *
 * Contributors:
 * Steve Kann <stevek@stevek.com>
 * Michael Van Donselaar <mvand@vandonselaar.org>
 * Shawn Lawrence <shawn.lawrence@terracecomm.com>
 *
 * This program is free software, distributed under the terms of
 * the GNU Lesser (Library) General Public License.
 *
 * Module: audio_openal
 * Purpose: Audio code to provide OpenAL driver support for IAX library
 * Developed by: Charles INGELS, <charles@maisonblv.net>.
 * Creation Date: December 18, 2008
 *
 * This library uses the OpenAL Library
 * For more information see: http://www.openal.com/
 *
 */

#if defined(WIN32)  ||  defined(_WIN32_WCE)
#include <stdlib.h>
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

#include "audio_openal.h"
#include "iaxclient_lib.h"
#include "ringbuffer.h"
#include "portmixer.h"
#include "config.h"

#ifdef SPAN_EC
#include "ec/echo.h"
static echo_can_state_t *ec;
#endif

#ifdef SPEEX_EC
#define restrict __restrict
#include "speex/speex_echo.h"
static SpeexEchoState *ec;
#endif


static ALCdevice *s_device;
static ALCcontext *s_context;
static char *s_list_devices;


int openal_initialize(struct iaxc_audio_driver *driver, int sample_rate)
{
	s_device = alcOpenDevice(NULL);
	if(!s_device) return (1);
	
	s_context = alcCreateContext(s_device, NULL);
	if(!s_context)
	{
		alcCloseDevice(s_device);
		return(2);
	}
	
	if(!alcMakeContextCurrent(s_context))
	{
		alcDestroyContext(s_context);
		alcCloseDevice(s_device);
		return (3);
	}
	
	return(0);
}


int openal_finalize()
{
	alcMakeContextCurrent(NULL);
	alcDestroyContext(s_context);
	alcCloseDevice(s_device);
}

