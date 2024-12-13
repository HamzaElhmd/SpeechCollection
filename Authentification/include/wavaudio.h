#ifndef WAVAUDIO_H
#define WAVAUDIO_H

#define AUDIO_SIZE 1000000 // 1 mB
#define SAMPLE_RATE 16000 // 16 kHz
#define FORMAT 16 // signed 16-bit
#define CHANNEL 1 // mono

#include <stdint.h>

typedef struct {
	int16_t pcm[AUDIO_SIZE/2];
	int8_t channel;
	int8_t format;
	int16_t samplerate;
	size_t pcmlen;
	char owner[1024];
	float duration;
	char is_native[100];
	char gender[100];
} wavaudio_t;


int8_t wavaudio_set_pcm(wavaudio_t *audio, char *bytes, size_t len);
int8_t wavaudio_insert(const wavaudio_t audio);
int8_t wavaudio_fwrite(char *bytes, size_t len);
int8_t wavaudio_fread(char *filename, char *bytes);
int8_t wavaudio_fremove(char *filename);
int8_t wavaudio_pcm_playback(const int16_t *pcm, size_t pcmsize);

void wavaudio_print(const wavaudio_t audio);

#endif
