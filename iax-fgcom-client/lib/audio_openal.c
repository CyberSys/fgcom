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
#include <glib.h>

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
static int s_selected_input;
static int s_selected_output; 
static int s_selected_ring;
static int s_running;


static void func_dummy(void)
{
}

/*
 * +--------------------------------------------------------------------------+
 * | scan_devices                                                             |
 * +--------------------------------------------------------------------------+
 * | Scan for all available audio devices on the current system.              |
 * | Builds a structure that contains all found devices.                      |
 * +--------------------------------------------------------------------------+
 * | Parameters                                                               |
 * |   driver (in/out): a structure that describes the audio driver (must be  |
 * |                  : previously allocated by the caller).                  |
 * +--------------------------------------------------------------------------+
*/
static int scan_audio_devices(struct iaxc_audio_driver *driver)
{
	const ALCchar *l_devlist;
	const ALCchar *l_dummy;
	int l_ndev;
	
	l_devlist = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
	if(l_devlist)
	{
		l_dummy = l_devlist;
		l_ndev = 0;
		while(strlen(l_dummy) > 0)
		{
			l_ndev++;
			l_dummy += strlen(l_dummy) + 1;
		}
		PRINTDBG("Found %d audio devices\n", l_ndev);
		
		driver->devices = (struct iaxc_audio_device *)malloc(l_ndev * sizeof(struct iaxc_audio_device));
		if(!driver->devices)
		{
			driver->devices = NULL;
			driver->nDevices = 0;
			return (-1);
		}
		
		driver->nDevices = l_ndev;
		
		l_dummy = l_devlist;
		l_ndev = 0;
		while(strlen(l_dummy) >0)
		{
			driver->devices[l_ndev].name = strdup(l_dummy);
			driver->devices[l_ndev].capabilities = 0;
			driver->devices[l_ndev].devID = l_ndev;
			
			PRINTDBG("Device %d: '%s'\n", l_ndev, l_dummy);
			l_ndev++;
			l_dummy += strlen(l_dummy) + 1;
		}
	}
	else
	{
		driver->devices = NULL;
		driver->nDevices = 0;
	}
	return(0);
}

static int select_devices(struct iaxc_audio_driver *driver, int input, int output, int ring)
{
	g_return_if_fail(driver != NULL);
	
	if((input < 0 || input >= driver->nDevices) ||
	   (output < 0 || output >= driver->nDevices) ||
	   (ring < 0 || ring >= driver->nDevices))
		return(1);
	
	s_selected_input = input;
	s_selected_output = output;
	s_selected_ring = ring;
	if ( s_running )
	{
		/*
		pa_stop(driver);
		pa_start(driver);
		*/
	}
	return 0;
}

static int selected_devices(struct iaxc_audio_driver *d, int *input, int *output, int *ring)
{
	g_return_if_fail(input != NULL);
	g_return_if_fail(output != NULL);
	g_return_if_fail(ring != NULL);
	
	*input = s_selected_input;
	*output = s_selected_output;
	*ring = s_selected_ring;
	return 0;
}














/*
 * +--------------------------------------------------------------------------+
 * | openal_initialize                                                        |
 * +--------------------------------------------------------------------------+
 * | Start the OpenAL initialization.                                         |
 * +--------------------------------------------------------------------------+
 * | Parameters                                                               |
 * |   driver (in/out): a structure that describes the audio driver (must be  |
 * |                  : previously allocated by the caller).                  |
 * |   sample_rate (in): the sample rate which will be used by the device.    |
 * +--------------------------------------------------------------------------+
*/
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
	
	if(scan_audio_devices(driver) != 0)
	{
		PRINTDBG("A problem occured when scanning for audio devices.\n");
		return(4);
	}
	
	if(driver->nDevices == 0)
	{
		PRINTDBG("No audio device found.\n");
		return(5);
	}
	
	driver->initialize = openal_initialize;
	driver->destroy = openal_finalize;
	driver->select_devices = select_devices;
	driver->selected_devices = selected_devices;
	driver->start = func_dummy;
	driver->stop = func_dummy;
	driver->output = func_dummy;
	driver->input = func_dummy;
	driver->input_level_get = func_dummy;
	driver->input_level_set = func_dummy;
	driver->output_level_get = func_dummy;
	driver->output_level_set = func_dummy;
	driver->play_sound = func_dummy;
	driver->stop_sound = func_dummy;
	driver->mic_boost_get = func_dummy;
	driver->mic_boost_set = func_dummy;
	
	s_selected_input = s_selected_output = s_selected_ring = 0;
	s_running = 0;
	
	return(0);
}


int openal_finalize(struct iaxc_audio_driver *driver)
{
	int l_index;
	
	g_return_if_fail(driver != NULL);
	
	alcMakeContextCurrent(NULL);
	alcDestroyContext(s_context);
	alcCloseDevice(s_device);
	
	if(driver->devices != NULL)
	{
		for(l_index = 0; l_index < driver->nDevices; l_index++)
		{
			if(driver->devices[l_index].name)
			{
				free(driver->devices[l_index].name);
				driver->devices[l_index].name = NULL;
			}
		}
		free(driver->devices);
		driver->devices = NULL;
		driver->nDevices = 0;
	}
}

