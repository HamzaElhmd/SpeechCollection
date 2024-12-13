#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mongoc/mongoc.h>
#include <alsa/asoundlib.h>
#include <libswresample/swresample.h>
#include "../include/wavaudio.h"

#define DEVICE "default"

int8_t wavaudio_fwrite(char *bytes, size_t len)
{
	FILE *out = fopen("temp.wav", "wb");
	if (!out || !bytes)
	{
		fprintf(stderr, "Error: couldn't open temp.wav for writing.\n");
		return 1;
	}
	fprintf(stdout, "INFO : Opened temp.wav for writing received bytes.\n");

	size_t frames_w = fwrite(bytes, 1, len, out);
	if (frames_w < len)
	{
		fprintf(stderr, "Error: couldn't write bytes to temp.wav.\n ");
		return 1;
	}
	fprintf(stdout, "Successfully written %ld bytes to temp.wav.\n", len);

	fclose(out);
	return 0;
}

int8_t wavaudio_fread(char *filename, char *bytes)
{
	if (!filename || !bytes)
	{
		fprintf(stderr, "Error: invalid null arguments to wavaudio_fread().\n");
		return 1;
	}

	FILE *in = fopen(filename, "rb");
	if (!in)
	{
		fprintf(stderr, "Error: [%s] no such file or read permission denied.\n", 
				filename);
		return 1;
	}

	size_t filesize;
	size_t byte_size;

	fseek(in, 0, SEEK_END);
	filesize = ftell(in);
	fseek(in, 0, SEEK_SET);

	byte_size = sizeof(bytes);

	if ((AUDIO_SIZE + 1024) < filesize)
	{
		fprintf(stderr, "Error: buffer overflow when reading temporary file.\n");
		fprintf(stderr, "Buffer size : %ld\nFile size : %ld\n",
			       	byte_size, filesize);
		return 1;
	}

	size_t frames_r = fread(bytes, 1, filesize, in);
	if (frames_r != filesize)
	{
		if (feof(in))
		{
			fprintf(stdout, "End of file reached\n");
		} else if (ferror(in))
		{
			fprintf(stdout, "An error occured when reading from temporary file.\n");
			return 1;
		}
	}

	return 0;
}

int8_t wavaudio_fremove(char *filename)
{
	if (!filename)
	{
		fprintf(stderr, "Error: invalid null arguments to wavaudio_remove()\n");
		return 1;
	}

	remove(filename);

	return 0;
}

int8_t wavaudio_pcm_playback(const int16_t *pcm, const size_t pcmsize)
{
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	int8_t error_code;
	int16_t samplerate = 16000;
	size_t frames_r;

	error_code = snd_pcm_open(&handle, DEVICE, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if (error_code < 0)
	{
		fprintf(stderr, "Error: failed to open pcm device for playback.\n");
		return 1;
	}

	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);

	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_rate(handle, params, samplerate, 0);
	snd_pcm_hw_params_set_channels(handle, params, 1);

	error_code = snd_pcm_hw_params(handle, params);
	if (error_code < 0)
	{
		fprintf(stderr, "Error: failed to attach parameters to playback pcm device.\n");
		snd_pcm_close(handle);
		return 1;
	}

	frames_r = snd_pcm_writei(handle, pcm, pcmsize);
	if (frames_r < 0)
	{
		fprintf(stderr, "Error: failed to write to playback device pcm data.\n");
		snd_pcm_close(handle);
		return 1;
	}

	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	return 0;
}

int8_t wavaudio_set_pcm(wavaudio_t *audio, char *bytes, size_t len)
{
	int8_t error_code;
	char tempbuff[AUDIO_SIZE + 1024];

	if (!audio || !bytes)
	{
		fprintf(stderr, "error: invalid null args to wavaudio_set_pcm.\n");
		return 1;
	}

	fprintf(stdout, "INFO : Bytes length => %ld\n", len);

	error_code = wavaudio_fwrite(bytes, len);
	if (error_code)
		return 1;

	system("ffmpeg -i temp.wav -ac 1 -ar 16000 temp_2.wav");

	error_code = wavaudio_fread("temp_2.wav", tempbuff);
	if (error_code)
		return 1;

	system("ffmpeg -i temp_2.wav -f s16le -acodec pcm_s16le values.pcm");

	FILE *pcmin = fopen("values.pcm", "rb");
	if (!pcmin)
	{
		fprintf(stderr, "Error: couldn't open file values.pcm\n");
		return 1;
	}

	fseek(pcmin, 0, SEEK_END);
	size_t pcmfile_s = ftell(pcmin);
	fseek(pcmin, 0, SEEK_SET);

	size_t pcm_s = fread(audio->pcm, sizeof(int16_t), pcmfile_s / sizeof(int16_t) ,pcmin);
	if (pcm_s != (pcmfile_s / sizeof(int16_t)))
	{
		if (feof(pcmin))
			fprintf(stdout, "End of file reached.\n");
		else if (ferror(pcmin))
		{
			fprintf(stderr, "Error: couldn't read from values.pcm file.\n");
			return 1;
		}
	}

	audio->pcmlen = pcmfile_s / sizeof(int16_t);
	audio->duration = audio->pcmlen / SAMPLE_RATE;

	fclose(pcmin);
	wavaudio_fremove("temp_2.wav");
	wavaudio_fremove("temp.wav");
	wavaudio_fremove("values.pcm");
	return 0;
}

static char* int16_array_to_string(const int16_t* array, size_t length) {
    if (array == NULL || length == 0) {
        return strdup(""); // Return an empty string for null or empty array
    }

    // Calculate the maximum buffer size required
    // Each int16_t can have up to 6 characters (-32768), plus a comma, plus a null terminator
    size_t buffer_size = length * (6 + 1);
    char* result = (char*)malloc(buffer_size);
    if (result == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    result[0] = '\0'; // Initialize the string as empty

    // Convert each number to a string and append to the result
    for (size_t i = 0; i < length; ++i) {
        char temp[8]; // Temporary buffer for the current number
        snprintf(temp, sizeof(temp), "%d", array[i]);
        strcat(result, temp);
        if (i < length - 1) {
            strcat(result, ",");
        }
    }

    return result; // The caller is responsible for freeing this memory
}

const char *mongodb_uri = "mongodb://127.0.0.1:27017";

int8_t wavaudio_insert(const wavaudio_t audio)
{

	mongoc_client_t *cli;
	mongoc_database_t *sc_db;
	mongoc_collection_t *wav_coll;
	bson_t *doc;
	bson_t *response;
	bson_error_t error;
	int8_t rc = 0, err;
	size_t pcm_array_size = audio.pcmlen * 6;
	size_t additional_size = 200 + strlen(audio.owner) + strlen(audio.gender) + strlen(audio.is_native); // Metadata and formatting
	char json_str[pcm_array_size + additional_size];
	char *frames_c;

	cli = mongoc_client_new(mongodb_uri);
	if (cli == NULL)
	{
		fprintf(stderr, "ERROR: MongoDB client failed to connect to mongoc.\n");
		return 1;
	}

	sc_db = mongoc_client_get_database(cli, "speechCollector");
	if (sc_db == NULL)
	{
		fprintf(stderr, "ERROR : failed to get speechCollector database.\n");
		rc = 1;
		goto clean;
	}

	wav_coll = mongoc_database_get_collection(sc_db, "wavAudio");
	if (wav_coll == NULL)
	{
		fprintf(stderr, "ERROR : failed to get the wavAudio collection.\n");
		rc = 1;
		goto clean;
	}

	frames_c = int16_array_to_string(audio.pcm, audio.pcmlen);
	sprintf(json_str, "{\"pcm\": [%s], \"size\": %ld, \"duration\": %.2lf, \"owner\": \"%s\", \"gender\": \"%s\", \"isNative\": \"%s\"}", frames_c, audio.pcmlen, audio.duration, audio.owner, audio.gender, audio.is_native);

	doc = bson_new();
	err = bson_init_from_json(doc, json_str, strlen(json_str), &error);
	if (!err)
	{
		fprintf(stderr, "ERROR : failed to initialize bson document from json string.\n");
		fprintf(stderr, "%s", error.message);
		rc = 1;
		goto clean;
	}

	response = bson_new();
	err = mongoc_collection_insert_one(wav_coll, doc, NULL, response, &error);
	if (err != 1)
	{
		fprintf(stderr, "ERROR : failed to insert audio into wavAudio collection.\n");
		rc = 1;
		goto clean;
	}

clean:
	mongoc_database_destroy(sc_db);
	bson_destroy(doc);
	bson_destroy(response);
	mongoc_collection_destroy(wav_coll);
	mongoc_client_destroy(cli);
	return rc;
}

void wavaudio_print(const wavaudio_t audio) {

	fprintf(stdout, "size : %ld, owner: %s, duration: %lf, samplerate: %d\n",
	      audio.pcmlen, audio.owner, audio.duration, audio.samplerate );

}
