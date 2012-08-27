////////////////////////////////////////////////
//
//		pspaalib.c
//		Part of the PSP Advanced Audio Library
//		Created by Arshia001
//
//		This file includes all the public functions for the
//		PSPAALIB.
//
////////////////////////////////////////////////

#include <time.h>
#include <fcntl.h>
#include "pspaalib.h"
typedef struct
{
	int effects[7];
	ScePspFVector2 position;
	ScePspFVector2 velocity;
	float playSpeed;
	AalibVolume volume;
	float ampValue;
	float audioStrength;
	int initialized;
    int hardwareChannel;
    // my custom variables
    clock_t start_time;
    bool is_started;
    int is_SRC;
    void *bufs;
    int buff_num;
    int preloaded_blocks;
} AalibChannelData;

AalibChannelData channels[49];
int hardwareChannels[8+1];  //one more for the SRC channel
SceUID threads[8+1];
SceUID decode_thread;
ScePspFVector2 observerPosition={0,0},observerFront={0,1},observerVelocity={0,0};
int sample_size=PSP_AUDIO_SAMPLE_ALIGN(2048); // number of samples filled per call

void *mainBuf, *backBuf;
void *big_mem;
int buff_num;
SceUID f;
static SceUID play_sema, decode_sema, predecode_done_sema;

int GetFreeHardwareChannel()
{
	int i;
	for (i=0;i<8;i++)
	{
		if (!hardwareChannels[i])
		{
			return i;
		}
	}
	return -1;
}

int FreeHardwareChannel(int channel)
{
	int i;
	for (i=0;i<8;i++)
	{
		if (hardwareChannels[i]==channel)
		{
			while (AalibGetStatus(channel)!=PSPAALIB_STATUS_STOPPED)
			{
				sceKernelDelayThread(10);
			}
			sceAudioChRelease(i);
			hardwareChannels[i]=PSPAALIB_CHANNEL_NONE;
			return TRUE;
		}
	}
	return FALSE;
}

int GetRawBuffer(short* buf,int length,float amp,int channel)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return GetBufferWav(buf,length,amp,channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return GetBufferOgg(buf,length,amp,channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return GetBufferSceMp3(buf,length,amp,channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return GetBufferAt3(buf,length,amp,channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

void GetProcessedBuffer(void* abuf,unsigned int length,int channel)
{
	short* buf=(short*) abuf;
	//Control Volume
	if (channels[channel].effects[PSPAALIB_EFFECT_STEREO_BY_POSITION])
	{
		channels[channel].volume=GetVolumes(channels[channel].position,observerPosition,observerFront);
	}
	else if(!channels[channel].effects[PSPAALIB_EFFECT_VOLUME_MANUAL])
	{
		channels[channel].volume=(AalibVolume){1.0f,1.0f};
	}
	if (channels[channel].effects[PSPAALIB_EFFECT_STRENGTH_BY_POSITION])
	{
		channels[channel].audioStrength=GetStrengthByPosition(channels[channel].position,observerPosition);
	}
	else
	{
		channels[channel].audioStrength=1.0f;
	}
	//Control Play Speed
	if (channels[channel].effects[PSPAALIB_EFFECT_DOPPLER])
	{
		channels[channel].playSpeed=GetDopplerPlaySpeed(channels[channel].position,channels[channel].velocity,observerPosition,observerVelocity);
	}
	else if (!channels[channel].effects[PSPAALIB_EFFECT_PLAYSPEED])
	{
		channels[channel].playSpeed=1.0f;
	}
	if (!channels[channel].effects[PSPAALIB_EFFECT_AMPLIFY])
	{
		channels[channel].ampValue=1.0f;
	}
	//Get Buffer
	if ((channels[channel].effects[PSPAALIB_EFFECT_PLAYSPEED])||(channels[channel].effects[PSPAALIB_EFFECT_DOPPLER])||(channels[channel].effects[PSPAALIB_EFFECT_MIX]))
	{		
		short* tempBuf;
		tempBuf=malloc((int)(length*channels[channel].playSpeed*2*sizeof(short)));
		GetRawBuffer(tempBuf,length*channels[channel].playSpeed,channels[channel].ampValue,channel);
		GetBufferSpeedEffect(buf,tempBuf,length,channels[channel].playSpeed,channels[channel].effects[PSPAALIB_EFFECT_MIX]);
		free(tempBuf);
	}
	else
	{
        GetRawBuffer(buf,length,channels[channel].ampValue,channel);
	}
}

int AalibGetPlayPosition(int channel) {
    AalibChannelData *data = &channels[channel];
    return data->is_started ? clock() - data->start_time : 0;
}

int PlayThread(SceSize argsize, void* args)
{
    printf("play thread excute start!\n");
	if (argsize!=sizeof(int))
	{
		sceKernelExitThread(0);
	}
    int arg = *((int*)args);
    if (arg < 0) {
        SRCChPlayThread(argsize, args);
    } else {
        ChPlayThread(argsize, args);
    }
}

int SRCChDecodeThread(SceSize argsize, void* args) {
    int stopReason;
    int channel;
    int sample_rate;
    int ret, i=0, t1, t2;
    int first_block=TRUE;
    int free_buff = buff_num;

    if (argsize != sizeof(int)) {
        sceKernelExitThread(0);
    }
    channel = *((int*)args);
    sample_rate = AalibGetSampleRate(channel);
    //printf("sample rate =%d\n", sample_rate);
    
    sceAudioSRCChRelease();
    ret = sceAudioSRCChReserve(sample_size, sample_rate, 2);
    sceAudioSRCOutputBlocking(0, big_mem);
    //printf("reserve SRC Channel %d\n", ret);

    int p;

    while(TRUE)
    {
        stopReason=AalibGetStopReason(channel);
        if (!stopReason)
        {
            goto Play;
        }
        else if (stopReason<0)
        {
            sceKernelDelayThread(100);
            continue;
        }
        else
        {
            goto Release;
        }
    }
Play:
    p = 0;
    while (!AalibGetStopReason(channel)) {
        sceKernelWaitSema(decode_sema, 1, 0);

        backBuf = big_mem + p;
        if (first_block) {
            first_block = FALSE;
            backBuf = big_mem;
            for (i = 0; i < buff_num; ++ i) {
                GetProcessedBuffer(backBuf, sample_size, channel);
                backBuf += sample_size * 4;
            }
            free_buff = buff_num - 1;
            p = sample_size * 4;
            backBuf = big_mem;
            sceKernelSignalSema(predecode_done_sema, 1);
            continue;
        } else if (free_buff <= 0){
            GetProcessedBuffer(backBuf, sample_size, channel);
        } else {
            free_buff --;
        }

        p += sample_size * 4;
        p %= sample_size * 4 * buff_num;
        sceKernelSignalSema(play_sema, 1);
    }
Release:
    sceAudioSRCChRelease();
    return 0;
}

//use sceAudioSRC* functions, in order to support different sample rates
int SRCChPlayThread(SceSize argsize, void* args)
{
    void *tmpBuf;
    int channel = hardwareChannels[8];
    int stopReason;
    int t1, t2;
    //printf("SRC Play thread started %d\n", clock());

    while(TRUE) {
        sceKernelWaitSema(play_sema, 1, 0);
//        printf("output thread waited time = %d, t1=%d, t2=%d, \n", t2 - t1, t1, t2);
        //tmpBuf = backBuf;
        //backBuf = mainBuf;
        //mainBuf = tmpBuf;
        //mainBuf = backBuf;
        
        sceKernelSignalSema(decode_sema, 1);
        //t1 = clock();
        sceAudioSRCOutputBlocking(PSP_AUDIO_VOLUME_MAX, backBuf);
        //t2 = clock();
        //printf("output cost %d %d %d\n", t2 - t1, t1, t2);
        if (AalibGetStopReason(channel) < 0) {
            break;
        }
    }
	sceKernelExitThread(0);
	return 0;    
}

//use sceAudio* functions, to make use of multiple hardware channel.
int ChPlayThread(SceSize argsize, void* args)
{
	int hardwareChannel=*((int*)args);
	int channel=hardwareChannels[hardwareChannel];
	int stopReason;
	void *mainBuf,*backBuf,*tempBuf;
    int sample_size = 512;

    //double buffer for pcm data
	mainBuf=memalign(64, sample_size * 4);
	backBuf=memalign(64, sample_size * 4);

	sceAudioChReserve(hardwareChannel,sample_size,PSP_AUDIO_FORMAT_STEREO);

	while(TRUE)
	{
		stopReason=AalibGetStopReason(channel);
		if (!stopReason)
		{
			goto Play;
		}
		else if (stopReason<0)
		{
			sceKernelDelayThread(100);
			continue;
		}
		else
		{
			goto Release;
		}
	}
Play:
    GetProcessedBuffer(mainBuf,sample_size,channel);
	while (!AalibGetStopReason(channel))
	{
		while (sceAudioGetChannelRestLen(hardwareChannel))
		{
			sceKernelDelayThread(10);
		}
		sceAudioOutputPanned(hardwareChannel,PSP_AUDIO_VOLUME_MAX,PSP_AUDIO_VOLUME_MAX,mainBuf);        
        GetProcessedBuffer(backBuf,sample_size,channel);
		tempBuf=mainBuf;
		mainBuf=backBuf;
		backBuf=tempBuf;
	}
Release:
	FreeHardwareChannel(channel);
	AalibStop(channel);
	sceKernelExitThread(0);
	return 0;    
}

int AalibInitChannels()
{
    char c[11];
    int i;
    printf("init channels(0-8)\n");
	for (i=0;i<8;i++)
	{
		sprintf(c,"aalibplay%i",i);
		threads[i]=sceKernelCreateThread(c,PlayThread,0x18,0x10000,0,NULL);
		if (threads[i]<0)
		{
            printf("create thread fail, %x\n", threads[i]);
			return PSPAALIB_WARNING_CREATE_THREAD;
		}
        printf("create thread id = %d\n", threads[i]);
    }
    return PSPAALIB_SUCCESS;
}

int AalibInitSRCChannel()
{
    threads[8] = sceKernelCreateThread("aalibplay8",PlayThread,0x18,0x10000,0,NULL);
    decode_thread = sceKernelCreateThread("decode_thread",SRCChDecodeThread,0x30,0x10000,0,NULL);
    if (threads[8] < 0 || decode_thread < 0) {
        return PSPAALIB_WARNING_CREATE_THREAD;
    }

    play_sema = sceKernelCreateSema("play_sema", 6, 0, 1, 0);
    decode_sema = sceKernelCreateSema("decode_sema", 6, 1, 1, 0);
    predecode_done_sema = sceKernelCreateSema("predecode_done_sema", 6, 0, 1, 0);
    buff_num = 4;
    big_mem = memalign(64, sample_size * 4 * buff_num);
    return PSPAALIB_SUCCESS;
}

int AalibInit()
{
    int ret;
    InitSceMp3();
	InitAt3();

    if ((ret = AalibInitChannels()) != PSPAALIB_SUCCESS) {
        return ret;
    }
    if ((ret = AalibInitSRCChannel()) != PSPAALIB_SUCCESS) {
        return ret;
    }
    return PSPAALIB_SUCCESS;
}

int AalibGetSampleRate(int channel)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
        return GetSampleRateWav(channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return GetSampleRateOgg(channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return GetSampleRateSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return GetSampleRateAt3(channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;    
}

int AalibSetAmplification(int channel,float amplificationValue)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	if (amplificationValue<0)
	{
		return PSPAALIB_ERROR_INVALID_AMPLIFICATION_VALUE;
	}
	channels[channel].ampValue=amplificationValue;
	return PSPAALIB_SUCCESS;
}

int AalibSetVolume(int channel,AalibVolume volume)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	channels[channel].volume=volume;
	return PSPAALIB_SUCCESS;
}

int AalibSetPlaySpeed(int channel,float playSpeed)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	channels[channel].playSpeed=playSpeed;
	return PSPAALIB_SUCCESS;
}

int AalibSetObserverVelocity(ScePspFVector2 velocity)
{
	observerVelocity=velocity;
	return PSPAALIB_SUCCESS;
}

int AalibSetObserverPosition(ScePspFVector2 position)
{
	observerPosition=position;
	return PSPAALIB_SUCCESS;
}

int AalibSetObserverFront(ScePspFVector2 front)
{
	observerFront=front;
	return PSPAALIB_SUCCESS;
}

int AalibSetPosition(int channel,ScePspFVector2 position)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	channels[channel].position=position;
	return PSPAALIB_SUCCESS;
}

int AalibSetVelocity(int channel,ScePspFVector2 velocity)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	channels[channel].velocity=velocity;
	return PSPAALIB_SUCCESS;
}

int AalibEnable(int channel,int effect)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	if ((effect<0)||(effect>6))
	{
		return PSPAALIB_ERROR_INVALID_EFFECT;
	}
	channels[channel].effects[effect]=TRUE;
	return PSPAALIB_SUCCESS;
}

int AalibDisable(int channel,int effect)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	channels[channel].effects[effect]=FALSE;
	return PSPAALIB_SUCCESS;
}

int AalibUnload(int channel)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	channels[channel].initialized=FALSE;
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return UnloadWav(channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return UnloadOgg(channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return UnloadSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return UnloadAt3(channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibLoadEx(char *filename, int channel, int flags)
{
}

int AalibLoad(cccUCS2* filename,int channel,int loadToRam, int is_SRC)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	memset(channels[channel].effects,0,7);
	channels[channel].position=(ScePspFVector2){0.0f,0.0f};
	channels[channel].velocity=(ScePspFVector2){0.0f,0.0f};
	channels[channel].playSpeed=1.0f;
	channels[channel].volume=(AalibVolume){1.0f,1.0f};
	channels[channel].ampValue=1.0f;
	channels[channel].initialized=TRUE;
    channels[channel].is_SRC = is_SRC;

    int ret = PSPAALIB_ERROR_INVALID_CHANNEL;
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		ret = LoadWav(filename,channel-PSPAALIB_CHANNEL_WAV_1,loadToRam);
        if (is_SRC) {
            PlayWav(channel-PSPAALIB_CHANNEL_WAV_1);
        }
	}
    if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		ret = LoadOgg(filename,channel-PSPAALIB_CHANNEL_OGG_1,loadToRam);
        if (is_SRC) {
            PlayOgg(channel-PSPAALIB_CHANNEL_OGG_1);
        }        
	}
    if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		ret = LoadSceMp3(filename,channel-PSPAALIB_CHANNEL_SCEMP3_1);
        if (is_SRC) {
            PlaySceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
        }        
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		ret = LoadAt3(filename,channel-PSPAALIB_CHANNEL_AT3_1);
        if (is_SRC) {
            PlayAt3(channel-PSPAALIB_CHANNEL_AT3_1);
        }                
	}

    if (ret != PSPAALIB_SUCCESS) {
        return ret;
    } else {
        if (channels[channel].is_SRC) {
            printf("try to preload 2048 samples\n");

            if (sceKernelPollSema(decode_sema, 0) > 0) {
                sceKernelSignalSema(decode_sema, 1);
            }
            if (sceKernelPollSema(play_sema, 1) > 0) {
                sceKernelSignalSema(play_sema, 1);
            }
            if (sceKernelPollSema(predecode_done_sema, 1) > 0) {
                sceKernelSignalSema(predecode_done_sema, 1);
            }

            //printf("set music to play status = %d\n", PlayOgg(channel - PSPAALIB_CHANNEL_OGG_1));
            int arg = - channel;
            sceKernelStartThread(decode_thread, sizeof(int), ((void *)(&channel)));
            sceKernelStartThread(threads[8], sizeof(int), ((void *)&arg));
            sceKernelWaitSema(predecode_done_sema, 1, 0);
            printf("SRC Channel decode thread predecode over\n");
        }

    }
    return ret;
}

int AalibPlay(int channel)
{
    printf("function called %d\n", clock());
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	if (!channels[channel].initialized)
	{
		return PSPAALIB_ERROR_UNINITIALIZED_CHANNEL;
	}
	if (!channels[channel].is_SRC && !AalibGetStopReason(channel))
	{
		return PSPAALIB_SUCCESS;
	}
	int hardwareChannel=(channels[channel].is_SRC == FALSE ? GetFreeHardwareChannel() : 8);
	if (hardwareChannel==-1)
	{
		return PSPAALIB_WARNING_NO_FREE_CHANNELS;
	}
	hardwareChannels[hardwareChannel] = channel;
    channels[channel].hardwareChannel = hardwareChannel;

    printf("start audio thread at hardware %d\n", hardwareChannel);
    printf("thread id = %d\n", threads[hardwareChannel]);

    int arg = hardwareChannel;
    if (channels[channel].is_SRC) {
        //arg = -arg;
        //printf("play thread start, ret = %d\n", sceKernelStartThread(threads[hardwareChannel],sizeof(int),(void*)(&arg)));
        sceKernelSignalSema(play_sema, 1);
        //printf("decode thread start, ret = %d\n", sceKernelStartThread(decode_thread,sizeof(int),(void*)(&channel)));
    } else {
        sceKernelStartThread(threads[hardwareChannel],sizeof(int),(void*)(&arg));
        //printf("play thread start, ret = %d\n", sceKernelStartThread(threads[hardwareChannel],sizeof(int),(void*)(&arg)));
    }
	
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return PlayWav(channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return PlayOgg(channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return PlaySceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return PlayAt3(channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibStop(int channel)
{
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return StopWav(channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return StopOgg(channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return StopSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return StopAt3(channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibPause(int channel)
{
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return PauseWav(channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return PauseOgg(channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return PauseSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return PauseAt3(channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibRewind(int channel)
{
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return RewindWav(channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return RewindOgg(channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return RewindSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return RewindAt3(channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibSeek(int channel,int time)
{
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return SeekWav(channel-PSPAALIB_CHANNEL_WAV_1,time);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return SeekOgg(channel-PSPAALIB_CHANNEL_OGG_1,time);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibSetAutoloop(int channel,int autoloop)
{
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return SetAutoloopWav(channel-PSPAALIB_CHANNEL_WAV_1,autoloop);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return SetAutoloopOgg(channel-PSPAALIB_CHANNEL_OGG_1,autoloop);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return SetAutoloopSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1,autoloop);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return SetAutoloopAt3(channel-PSPAALIB_CHANNEL_AT3_1,autoloop);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibGetStopReason(int channel)
{
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		return GetStopReasonWav(channel-PSPAALIB_CHANNEL_WAV_1);
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		return GetStopReasonOgg(channel-PSPAALIB_CHANNEL_OGG_1);
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		return GetStopReasonSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1);
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		return GetStopReasonAt3(channel-PSPAALIB_CHANNEL_AT3_1);
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}

int AalibGetStatus(int channel)
{
	if ((channel<1)||(channel>48))
	{
		return PSPAALIB_ERROR_INVALID_CHANNEL;
	}
	if ((PSPAALIB_CHANNEL_WAV_1<=channel)&&(channel<=PSPAALIB_CHANNEL_WAV_32))
	{
		if (GetStopReasonWav(channel-PSPAALIB_CHANNEL_WAV_1))
		{
			return PSPAALIB_STATUS_STOPPED;
		}
		else if (GetPausedWav(channel-PSPAALIB_CHANNEL_WAV_1))
		{
			return PSPAALIB_STATUS_PAUSED;
		}
		else
		{
			return PSPAALIB_STATUS_PLAYING;
		}
	}
	if ((PSPAALIB_CHANNEL_OGG_1<=channel)&&(channel<=PSPAALIB_CHANNEL_OGG_10))
	{
		if (GetStopReasonOgg(channel-PSPAALIB_CHANNEL_OGG_1))
		{
			return PSPAALIB_STATUS_STOPPED;
		}
		else if (GetPausedOgg(channel-PSPAALIB_CHANNEL_OGG_1))
		{
			return PSPAALIB_STATUS_PAUSED;
		}
		else
		{
			return PSPAALIB_STATUS_PLAYING;
		}
	}
	if ((PSPAALIB_CHANNEL_SCEMP3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_SCEMP3_2))
	{
		if (GetStopReasonSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1))
		{
			return PSPAALIB_STATUS_STOPPED;
		}
		else if (GetPausedSceMp3(channel-PSPAALIB_CHANNEL_SCEMP3_1))
		{
			return PSPAALIB_STATUS_PAUSED;
		}
		else
		{
			return PSPAALIB_STATUS_PLAYING;
		}
	}
	if ((PSPAALIB_CHANNEL_AT3_1<=channel)&&(channel<=PSPAALIB_CHANNEL_AT3_2))
	{
		if (GetStopReasonAt3(channel-PSPAALIB_CHANNEL_AT3_1))
		{
			return PSPAALIB_STATUS_STOPPED;
		}
		else if (GetPausedAt3(channel-PSPAALIB_CHANNEL_AT3_1))
		{
			return PSPAALIB_STATUS_PAUSED;
		}
		else
		{
			return PSPAALIB_STATUS_PLAYING;
		}
	}
	return PSPAALIB_ERROR_INVALID_CHANNEL;
}
