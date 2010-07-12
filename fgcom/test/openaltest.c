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
#define BYTES_PER_SAMPLE 2*2
#define CAPTURE_TIME_SECS 3
#define BUFFERSIZE 192000

int main(int argc, char *argv[])
{
	alutInit (&argc, argv);
	ALubyte* buf=NULL;

	// Query version information
	fprintf(stderr,"Version:    %s\n",alGetString(AL_VERSION));
	fprintf(stderr,"Renderer:   %s\n",alGetString(AL_RENDERER));
	fprintf(stderr,"Vendor:     %s\n",alGetString(AL_VENDOR));
	fprintf(stderr,"Extensions: %s\n",alGetString(AL_EXTENSIONS));

	// Test simple Output
	fprintf(stderr,"Playing \"Hello World\"...\n");
	//play_hello();

	fprintf(stderr,"Usable capture devices:\n");
	show_capture_devices();

	fprintf(stderr,"Capturing for %d seconds...\n",CAPTURE_TIME_SECS);
	if((buf=capture())==NULL)
		fprintf(stderr,"Cannot capture audio data!\n");
	else
	{
		fprintf(stderr,"Playing captured audio...\n");
		if(play(buf)<0)
			fprintf(stderr,"Cannot play captured audio data!\n");
	}

	if(buf!=NULL)
		free(buf);

	fprintf(stderr,"Exit %s\n",argv[0]);
	alutExit();
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
	{
		alBufferData(Buffer,FORMAT,buf,SAMPLERATE*BYTES_PER_SAMPLE*CAPTURE_TIME_SECS,SAMPLERATE);
	}

	alGenSources(1,&Source);
	alSourceQueueBuffers(Source,1,&Buffer);
	alSourcePlay(Source);
	sleep(CAPTURE_TIME_SECS);
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

	// Open capture device
	capture_device=alcCaptureOpenDevice((ALCchar*)capture_device_name,SAMPLERATE,FORMAT,SAMPLERATE*BYTES_PER_SAMPLE*CAPTURE_TIME_SECS);
	if((err=alGetError())!=AL_NO_ERROR) 
    	{
                s_openal_error("alcCaptureOpenDevice",err);
		exit(10);
        }
	if(capture_device==NULL)
	{
                fprintf(stderr,"alcCaptureOpenDevice returned NULL\n");
		exit(14);
        }

	alcCaptureStart(capture_device);
	if((err=alGetError())!=AL_NO_ERROR) 
    	{
                s_openal_error("alcCaptureStart",err);
                exit(11);
        }

	sleep(CAPTURE_TIME_SECS);

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
		fprintf(stderr,"Samples: %d\n",samples);
		if((buf=calloc(sizeof(ALubyte)*samples*BYTES_PER_SAMPLE))==NULL)
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
		do
		{
			fprintf(stderr,"Capture device: %s\n",tmp);
			tmp+=strlen(tmp)+1;
		} while(strlen(tmp)>0);
	}
	fprintf(stderr,"Default capture device: %s\n",(char*)alcGetString(NULL,ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER));
}

static void s_openal_error(const char* function, int err)
{
        fprintf(stderr, "OpenAl function %s failed with code 0x%x: %s\n",function,err,alGetString(err));
}
