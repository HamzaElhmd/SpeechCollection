#include <stdio.h>
#include "../include/wavaudio.h"

#define AUDIO_FILENAME "../resources/test.wav"

int8_t test_wav_audio_pcm_playback()
{
	int8_t error_code;

	FILE *wavin = fopen(AUDIO_FILENAME, "rb");
	if (!wavin)
	{
		fprintf(stderr, "Error: test failed to open test.wav file.\n");
		return 1;
	}

	fseek(wavin, 0, SEEK_END);
	size_t filesize = ftell(wavin);
	fseek(wavin, 0, SEEK_SET);

	char buffer[AUDIO_SIZE];
	size_t frames_r = fread(buffer, filesize - 1, 1, wavin);
	if (frames_r == 0 || frames_r < (filesize - 1))
	{
		fprintf(stderr, "Error: couldn't read the content of the test.wav file.\n");
		return 1;
	}

	wavaudio_t *audio = malloc(sizeof(wavaudio_t));
	audio->channel = 1;
	audio->format = 16;
	audio->samplerate = 16000;

	error_code = wavaudio_set_pcm(audio, buffer, strlen(buffer));
	if (error_code == 1)
	{
		fprintf(stderr, "WAVAUDIO SET PCM TEST : failed.\n");
		return 1;
	}

	error_code = wavaudio_pcm_playback(audio->pcm, audio->pcmlen);
	if (error_code == 1)
	{
		fprintf(stderr, "WAVAUDIO PCM PLAYBACK TEST : failed");
		return 1;
	}

	free(audio);
	fprintf(stdout, "WAVAUDIO SET PCM TEST : success.\nWAVAUDIO PCM PLAYBACK TEST : success.\n");
	return 0;	
}

int main (int argc, char *argv[])
{

	int err = test_wavaudio_pcm_playback();
	fprintf(stdout, "Test status : %d.\n", err);

	return 0;
}
