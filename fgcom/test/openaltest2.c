#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

void play_hello(void);
int play(ALubyte* buf);
ALubyte* capture(void);
void show_capture_devices(void);
static void s_openal_error(const char* function, int err);

ALint err=0;

#define SAMPLERATE 8000
#define FORMAT AL_FORMAT_STEREO16
#define BUFFERSIZE 24000

int main(int argc, char *argv[])
{
	alutInit (&argc, argv);
	ALubyte* buf=NULL;

	// Test simple Output
	play_hello();

	show_capture_devices();
	
	if((buf=capture())==NULL)
		fprintf(stderr,"Cannot capture audio data!\n");
	else
	{
		if(play(buf)<0)
			fprintf(stderr,"Cannot play captured audio data!\n");
	}

	if(buf!=NULL)
		free(buf);

	exit(0);
}

void play_hello(void)
{
	ALuint helloBuffer,helloSource;
	helloBuffer=alutCreateBufferHelloWorld();
	alGenSources(1,&helloSource);
	alSourcei(helloSource,AL_BUFFER,helloBuffer);
	alSourcePlay(helloSource);
	alutSleep(1);
	alutExit();
}

int play(ALubyte* buf)
{
	ALuint Buffer;
	ALuint Source;

	alGetError(); // clear any error messages
	alGenBuffers(1,&Buffer);
	if(alGetError()!=AL_NO_ERROR) 
    	{
        	fprintf(stderr,"Cannot create alBuffer\n");
		return(-1);
	}
	else
		alBufferData(Buffer,FORMAT,buf,BUFFERSIZE,SAMPLERATE);

    alGenSources(1,&Source);
    alSourcei(Source,AL_BUFFER,Buffer);
    alSourcePlay(Source);
    alutSleep(1);
    alutExit();
}

ALubyte* capture(void)
{
	ALCdevice* capture_device=NULL;
	ALint samples=0;
	ALubyte* buf=NULL;
	ALCdevice* capture_device_name=NULL;

	alGetError(); // clear any error messages

	capture_device_name=(ALCdevice*)alcGetString(NULL,ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);

	// check for capturing extension
	if(alcIsExtensionPresent(capture_device_name,"ALC_EXT_CAPTURE")==AL_FALSE)
	{
                fprintf(stderr,"Cannot find capture extension within OpenAL!\n");
		exit(15);
	}

	err=alGetError(); // clear any error messages
	capture_device=alcCaptureOpenDevice((ALCchar*)capture_device_name,SAMPLERATE,FORMAT,BUFFERSIZE);
	if(capture_device=NULL)
	{
                fprintf(stderr,"alcCaptureOpenDevice returned NULL\n");
		exit(14);
        }
	if((err=alGetError())!=AL_NO_ERROR) 
    	{
                s_openal_error("alcCaptureOpenDevice",err);
		exit(10);
        }

	alcCaptureStart(capture_device);
	if((err=alGetError())!=AL_NO_ERROR) 
    	{
                s_openal_error("alcCaptureStart",err);
                exit(11);
        }

	sleep(5);

	alcCaptureStop(capture_device);
	if((err=alGetError())!=AL_NO_ERROR) 
    	{
                s_openal_error("alcCaptureStop",err);
                exit(11);
        }

	// create buffer for capturing
	alcGetIntegerv(capture_device, ALC_CAPTURE_SAMPLES,(ALCsizei)sizeof(ALint),&samples);
	if((err=alGetError())!=AL_NO_ERROR) 
    	{
                s_openal_error("alcGetIntegerv",err);
                exit(11);
        }
	else
    	{
		if((buf=malloc(sizeof(ALubyte)*samples*2*6))==NULL)
		{
			fprintf(stderr,"Cannot malloc for buf\n");
			exit(12);
		}
	
		if(samples>0)
		{
			alcCaptureSamples(capture_device,(ALCvoid *)buf,samples); 
		}
	}

	alcCaptureCloseDevice(capture_device);

	return(buf);
}

void show_capture_devices(void)
{
	ALCchar* d=NULL;
	ALCchar* tmp=NULL;

	d=(char*)alcGetString(NULL,ALC_CAPTURE_DEVICE_SPECIFIER);
	if(d!=NULL)
	{
		tmp=d;
		fprintf(stderr,"Capture device: %s\n",tmp);
		while(strlen(tmp)>0)
		{
			tmp+=strlen(tmp)+1;
			if(strlen(tmp)>0)
				fprintf(stderr,"Capture device: %s\n",tmp);
		}
	}
	fprintf(stderr,"Default capture device: %s\n",(char*)alcGetString(NULL,ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER));
}

static void s_openal_error(const char* function, int err)
{
        fprintf(stderr, "OpenAl function %s failed with code %d: %s\n",function,err,alGetString(err));
}
