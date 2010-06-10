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
#include "portmixer.h"
#include "config.h"
#include "sndfile.h"

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
static struct iaxc_sound *s_sounds;
static MUTEX sound_lock;




static int s_openal_error(const char* function, int err)
{
	fprintf(stderr, "OpenAl function %s failed with code %d\n", function, err);
	return -1;
}


static void s_openal_unqueue(T_AUDIO_DATA* priv)
{
	int i;
	ALint err;
	ALint processed;

	alGetSourcei(priv->source, AL_BUFFERS_PROCESSED, &processed);

    alGetError();
    for(i = 0; i < processed; i++)
{
	alSourceUnqueueBuffers(priv->source, 1, priv->buffers + priv->buffers_tail);
	err = alGetError();
	if (err)
	{
		s_openal_error("alSourceUnqueueBuffers", err);
		break;
	}
	if (++priv->buffers_tail >= priv->num_buffers)
	{
		priv->buffers_tail = 0;
	}
	++priv->buffers_free;
}
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
static int al_scan_audio_devices(struct iaxc_audio_driver *driver)
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
			
			PRINTDBG("\tdevice %d: '%s'\n", l_ndev, l_dummy);
			l_ndev++;
			l_dummy += strlen(l_dummy) + 1;
		}
		PRINTDBG("\n");
	}
	else
	{
		driver->devices = NULL;
		driver->nDevices = 0;
	}
	return(0);
}

static int al_select_devices(struct iaxc_audio_driver *driver, int input, int output, int ring)
{
	if(driver != NULL)
		return(-1);
	
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

static int al_selected_devices(struct iaxc_audio_driver *d, int *input, int *output, int *ring)
{
        if(input != NULL || output != NULL || ring != NULL)
                return(-1);
	
	*input = s_selected_input;
	*output = s_selected_output;
	*ring = s_selected_ring;
	return 0;
}


static int al_play_sound(struct iaxc_sound *inSound, int ring)
{
	return(0);
}

int al_stop_sound(int id)
{
	return 0;
}

int al_mic_boost_get(struct iaxc_audio_driver *d)
{
	return 0;
}

int al_mic_boost_set(struct iaxc_audio_driver *d, int enable)
{
	return 0;
}

int al_destroy(struct iaxc_audio_driver *d)
{
	return 0;
}


static int al_input(struct iaxc_audio_driver *d, void *samples, int *nSamples)
{
	int err;
	T_AUDIO_DATA* priv = (T_AUDIO_DATA*)(d->priv);

	ALCint available;
	ALCsizei request;

	alcGetIntegerv(priv->in_dev, ALC_CAPTURE_SAMPLES, sizeof(available), &available);
	/* do not return less data than caller wanted, iaxclient does not like it */
	request = (available < *nSamples) ? 0 : *nSamples;
	if (request > 0)
	{
		err = alcGetError(priv->in_dev);
		alcCaptureSamples(priv->in_dev, samples, request);    
		err = alcGetError(priv->in_dev);
		if (err)
		{
			s_openal_error("alcCaptureSamples", err);
			*nSamples = 0;
			return 1;
		}
	// software mute, but keep data flowing for sync purposes
		if (priv->input_level == 0)
		{
			memset(samples, 0, 2 * request);
		}
	}
	*nSamples = request;
    
	return 0;
}

static int al_output(struct iaxc_audio_driver *d, void *samples, int nSamples)
{
	T_AUDIO_DATA* priv = (T_AUDIO_DATA*)(d->priv);

	s_openal_unqueue(priv);
	/* If we run out of buffers, wait for an arbitrary number to become free */
	if (priv->buffers_free == 0)
	{
		while(priv->buffers_free < 4)
		{
			iaxc_millisleep(100);
			s_openal_unqueue(priv);
		}
	}
    
	if (priv->buffers_free > 0)
	{
		ALuint buffer = priv->buffers[priv->buffers_head++];
		if (priv->buffers_head >= priv->num_buffers)
		{
			priv->buffers_head = 0;
		}
	
		alBufferData(buffer, AL_FORMAT_MONO16, samples, nSamples * 2, priv->sample_rate);
		alSourceQueueBuffers(priv->source, 1, &buffer);
		--priv->buffers_free;
	
		/* delay start of output until we have 2 buffers */
		if (priv->buffers_free == priv->num_buffers - 2)
		{
			ALint state;
	
			alGetSourcei(priv->source, AL_SOURCE_STATE, &state);
			if (state != AL_PLAYING)
			{
				alSourcePlay(priv->source);
			}
		}
	} else 
	{
		fprintf(stderr, "openal_output buffer overflow\n");
		return 1;
	}
    
	return 0;
}

static int al_play_file(const char *file)
{
	SF_INFO l_fi;
	SNDFILE *l_pfile;
	ALsizei l_nb_samples;
	ALsizei l_sample_rate;
	ALshort *l_pbuf;
	ALenum l_format;
	ALuint l_snd_buf;
	ALuint l_source;
	ALint l_snd_status;

	
	l_pfile = sf_open(file, SFM_READ, &l_fi);
	if (!l_pfile) return(-1);
	
	l_nb_samples = l_fi.channels * l_fi.frames;
	l_sample_rate = l_fi.samplerate;

	l_pbuf = (ALshort *)malloc(l_nb_samples * sizeof(ALshort));
	if(!l_pbuf)
	{
		sf_close(l_pfile);
		return(-2);
	}

	l_format = l_fi.channels==1?AL_FORMAT_MONO16:AL_FORMAT_STEREO16;

	if(sf_read_short(l_pfile, l_pbuf, l_nb_samples) < l_nb_samples)
	{
		free(l_pbuf);
		sf_close(l_pfile);
		return(-3);
	}
	
	sf_close(l_pfile);
	
	alGenBuffers(1, &l_snd_buf);

	alBufferData(l_snd_buf, l_format, l_pbuf, l_nb_samples * sizeof(ALushort), l_sample_rate);
	if(alGetError() != AL_NO_ERROR)
	{
		alDeleteBuffers(1, &l_snd_buf);
		free(l_pbuf);
		return(-4);
	}

	alGenSources(1, &l_source);

	alSourcei(l_source, AL_BUFFER, l_snd_buf);
	
	alSourcePlay(l_source);
	do
	{
		alGetSourcei(l_source, AL_SOURCE_STATE, &l_snd_status);
	}while(l_snd_status == AL_PLAYING);

	alSourcei(l_source, AL_BUFFER, 0);
	alDeleteSources(1, &l_source);
	alDeleteBuffers(1, &l_snd_buf);
}



int al_start(struct iaxc_audio_driver *d)
{
	T_AUDIO_DATA* priv = (T_AUDIO_DATA*)(d->priv);

	return 0;
}

int al_stop(struct iaxc_audio_driver *d)
{
	T_AUDIO_DATA* priv = (T_AUDIO_DATA*)(d->priv);

	return 0;
}

double al_input_level_get(struct iaxc_audio_driver *d)
{
	T_AUDIO_DATA* priv = (T_AUDIO_DATA*)(d->priv);

	return priv->input_level;
}

double al_output_level_get(struct iaxc_audio_driver *d)
{
	T_AUDIO_DATA* priv = (T_AUDIO_DATA*)(d->priv);

	return priv->output_level;
}

int al_input_level_set(struct iaxc_audio_driver *d, double level)
{
	T_AUDIO_DATA* priv = (T_AUDIO_DATA*)(d->priv);
	priv->input_level = (level < 0.5) ? 0 : 1;

	return 0;
}

int al_output_level_set(struct iaxc_audio_driver *d, double level)
{
	T_AUDIO_DATA* priv = (T_AUDIO_DATA*)(d->priv);
	priv->output_level = level;
	alSourcef(priv->source, AL_GAIN, level);

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
	T_AUDIO_DATA *l_data;
	ALenum l_err;
	
	l_data = (T_AUDIO_DATA *)malloc(sizeof(T_AUDIO_DATA));
	if (!l_data)
		return (6);
	
	s_device = alcOpenDevice(NULL);
	if(!s_device) return (1);
	
	s_context = alcCreateContext(s_device, NULL);
	if(!s_context)
	{
		alcCloseDevice(s_device);
		free(l_data);
		return(2);
	}
	
	if(!alcMakeContextCurrent(s_context))
	{
		alcDestroyContext(s_context);
		alcCloseDevice(s_device);
		free(l_data);
		return (3);
	}
	
	if(al_scan_audio_devices(driver) != 0)
	{
		PRINTDBG("A problem occured when scanning for audio devices.\n");
		return(4);
	}
	
	if(driver->nDevices == 0)
	{
		printf("No audio device found.\n");
		return(5);
	}
	
	driver->initialize = openal_initialize;
	driver->destroy = openal_finalize;
	driver->select_devices = al_select_devices;
	driver->selected_devices = al_selected_devices;
	driver->start = al_start;
	driver->stop = al_stop;
	driver->output = al_output;
	driver->input = al_input;
	driver->input_level_get = al_input_level_get;
	driver->input_level_set = al_input_level_set;
	driver->output_level_get = al_output_level_get;
	driver->output_level_set = al_output_level_set;
	driver->play_sound = al_play_sound;
	driver->stop_sound = al_stop_sound;
	driver->mic_boost_get = al_mic_boost_get;
	driver->mic_boost_set = al_mic_boost_set;
	
	s_selected_input = s_selected_output = s_selected_ring = 0;
	s_running = 0;
	
	/*
	 *  Creating audio buffers.
	 */
	l_data->num_buffers = MAX_NUM_AUDIO_BUFFERS;
	l_data->buffers = (ALuint *)malloc(sizeof(ALuint) * l_data->num_buffers);
	if (!l_data->buffers)
	{
		alcDestroyContext(s_context);
		alcCloseDevice(s_device);
		free(l_data);
		return(6);
	}
	alGenBuffers(l_data->num_buffers, l_data->buffers);
	/*
	 *  Creating one audio source.
	 */
	alGetError();
	alGenSources(1, &l_data->source);
	l_err = alGetError();
	switch(l_err)
	{
		case AL_OUT_OF_MEMORY:
			PRINTDBG("Out of memory.\n");
			break;
		case AL_INVALID_VALUE:
			PRINTDBG("Invalid value.\n");
			break;
		case AL_INVALID_OPERATION:
			PRINTDBG("Invalid operation.\n");
			break;
		default:
			PRINTDBG("Source generation ok.\n");
			break;
	};
	
	driver->priv = l_data;
	
	/* al_play_file("cow.wav"); */
	
	return(0);
}


/*
 * +--------------------------------------------------------------------------+
 * | openal_finalize                                                          |
 * +--------------------------------------------------------------------------+
 * | Close OpenAL.                                                            |
 * +--------------------------------------------------------------------------+
 * | Parameters                                                               |
 * |   driver (in/out): a structure that describes the audio driver (must be  |
 * |                  : previously allocated by the caller).                  |
 * +--------------------------------------------------------------------------+
*/
int openal_finalize(struct iaxc_audio_driver *driver)
{
	int l_index;
	T_AUDIO_DATA *l_data;
	
	if(driver != NULL)
		return(-1);
	
	l_data = driver->priv;
	
	alDeleteBuffers(l_data->num_buffers, l_data->buffers);
	
	free(l_data->buffers);
	
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

