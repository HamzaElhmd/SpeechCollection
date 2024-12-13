#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>
#include <curl/curl.h>
#include <unistd.h>
#include <mongoc/mongoc.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>

#include "../include/code.h"
#include "../include/wavaudio.h"

#define AUDIO_BUFFER_SIZE 1001024
#define BUFFER_SIZE 1024
#define SOCKET_NAME "/tmp/auth.sock"

void* start_registrer_routine (void* args); 
void* cancel_registrer_routine (void* args);

static pthread_t start_registrer, cancel_registrer;
static int8_t registrer_handle = -1, system_flag = 0;
static char error_message[BUFFER_SIZE];

int main (int argc, char *argv[])
{
	mongoc_init();
	int8_t error_code, rv = 0;
	void *rv_cr;

       	rv_cr = malloc(sizeof(int8_t));
	if (!rv_cr) {
               	perror("ERROR : Memory allocation failed for the return value of cancel registrer thread.\n");
               	rv = -1;
		goto cleanup;
       	}

	error_code = pthread_create(&start_registrer, NULL, start_registrer_routine, NULL);
	if (error_code != 0) {
		fprintf(stderr, "ERROR :Start registrer thread creation in main thread failed.\n");
		rv = -1;
		goto cleanup;
	}

	error_code = pthread_create(&cancel_registrer, NULL, cancel_registrer_routine, NULL);
	if (error_code != 0) {
               	fprintf(stderr, "ERROR : Cancel registrer thread creation in main thread failed.\n");
               	rv = -1;
		goto cleanup;
       	}

	if (pthread_join(start_registrer, NULL)) {
                fprintf(stderr, "ERROR : Waiting for start register to terminate failed.\n");
                rv = -1;
                goto cleanup;
        }

	if (pthread_join(cancel_registrer, rv_cr)) {
		fprintf(stderr, "ERROR : Waiting for cancel register to terminate failed.\n");
		rv = -1;
		goto cleanup;
	}


	if (*((int8_t *)rv_cr) == -1) {
		fprintf(stderr, "ERROR : Cancel registrer exited with -1.\n");
		return -1;
	} else if (system_flag) {
		fprintf(stderr, "ERROR : Start regostrer exited with -1.\n");
		return -1;
	}

cleanup:
	if (!rv_cr) free(rv_cr);
	mongoc_cleanup();
	remove(SOCKET_NAME);
	if (registrer_handle != -1) close(registrer_handle);

	return rv;
}

static char *emailer_ip = "127.0.0.1";
static int emailer_port = 4200;

int8_t send_verification_code (const char *email_address, const char *consent_code) {

	int8_t sockt, error_code;
	struct sockaddr_in emailer_address;
	socklen_t address_len = sizeof(emailer_address);
	char request_buffer[BUFFER_SIZE];

	memset(request_buffer, 0, BUFFER_SIZE);

	sockt = socket(AF_INET, SOCK_STREAM, 0);
	if (sockt < 0) {
		perror("ERROR : Client socket for emailer service could not be created.\n");
		return 1;
	}

	memset(&emailer_address, 0, address_len);
	emailer_address.sin_family = AF_INET;
	emailer_address.sin_port = htons(emailer_port);

	if (inet_pton(AF_INET, emailer_ip, &emailer_address.sin_addr) <= 0) {
		close(sockt);
		perror("ERROR : Failed to convert emailer service address into network address structure.\n");
		return 1;
	}
	address_len = sizeof(emailer_address);

	error_code = connect(sockt, (struct sockaddr*) &emailer_address, address_len);
	if (error_code) {
		close(sockt);
		perror("ERROR : Connection to emailer service failed.\n");
		return 1;
	}

	sprintf(request_buffer, "%s %s", email_address, consent_code);
	printf("INFO : Size of request buffer is %ld\n", sizeof(request_buffer));
	size_t bytes_w = write(sockt, request_buffer, sizeof(request_buffer));
	if (bytes_w < 0) {
		close(sockt);
		perror("ERROR : Write failure to emailer service.\n");
		return 1;
	}

	fprintf(stdout, "INFO : Successfully written %ld bytes to emailer service.\n", bytes_w);
	close(sockt);

	return 0;
}

int is_data_available (int fd) {
    	struct pollfd pfd;
    	pfd.fd = fd;
    	pfd.events = POLLIN;      // Check for data to read

    	int result = poll(&pfd, 1, 1000); // 1000 ms timeout

    	if (result > 0 && (pfd.revents & POLLIN)) {
        	return 1;             // Data is ready to be read
    	} else if (result == 0) {
        	return 0;             // Timeout occurred, no data ready
    	} else {
        	perror("poll");       // An error occurred
        	return -1;
    	}

}

void* start_registrer_routine (void *args) {

	struct sockaddr_un registrer_addr;
	socklen_t addrlen = sizeof(registrer_addr);
	int8_t error_code, client_handle, registrer_handle, tokens_count = 0;
	char text_buffer[BUFFER_SIZE], audio_buffer[AUDIO_BUFFER_SIZE];
	size_t bytes_s;
	char *consent_code, *tokens[100], *session_mem[10], email_addr_[255], *gender, 
		*nativity;

	registrer_handle = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (registrer_handle == -1)
	{
		perror("ERROR : Creation of registrer service socket failed.\n");
		system_flag = -1;
		return NULL;
	}

	memset(&registrer_addr, 0, addrlen);
	registrer_addr.sun_family = AF_UNIX;
	strcpy(registrer_addr.sun_path, SOCKET_NAME);
	addrlen = sizeof(registrer_addr);

	error_code = bind(registrer_handle, (struct sockaddr*) &registrer_addr, sizeof(registrer_addr));
	if (error_code)
	{
		sprintf(&error_message[0], "ERROR : Binding registrer service address path %s to registrer service UNIX socket %xu failed.\n", registrer_addr.sun_path, registrer_handle);
		perror(&error_message[0]);
		system_flag = -1;
		return NULL;
	}

        error_code = listen(registrer_handle, 100);
        if (error_code)
        {
        	perror("ERROR :  Marking registrer service socket as listening to incoming connections.\n");
		system_flag = -1;
		return NULL;
        }

	fprintf(stdout, "******************* LISTEN MODE *******************\n");

	while (1)
	{
		fprintf(stdout, "INFO : Accepting request...\n");
		memset(text_buffer, 0, BUFFER_SIZE);
		memset(audio_buffer, 0, AUDIO_BUFFER_SIZE);
		tokens_count = 0;

		client_handle = accept(registrer_handle, (struct sockaddr*) &registrer_addr, &addrlen);
		if (client_handle == -1)
		{
			perror("ERROR : Accepting incoming connection request failed.\n");
			continue;
		}

		bytes_s = read(client_handle, text_buffer, BUFFER_SIZE);
		if (bytes_s == -1)
		{
			perror("ERROR : Reading from client socket failed.\n");
			continue;
		}

		fprintf(stdout, "INFO : Command request received => %s\n", text_buffer);

		//size_t request_size = sizeof(text_buffer);

		tokens[tokens_count] = strtok(text_buffer, " ");
		tokens_count++;

		while ((tokens[tokens_count] = strtok(NULL, " ")) != NULL) tokens_count++;

		if (!strcmp(tokens[0], "SEND")) {
			consent_code = generate_random_code();

			if (insert_code(tokens[1], consent_code)) {
				if (write(client_handle, "ERROR", 5) < 0) {
                                        perror("ERROR : Failed to register code verification and email to database.\n");
                                        continue;
				}
			}

			if (!send_verification_code(tokens[1], consent_code)) {
				if (write(client_handle, "OK", 2) < 0) {
					perror("ERROR : Failed to send OK acknowledgement for sending verification code.\n");
					continue;
				}

				for (int i = 0; i < 3; i++) {
					session_mem[i] = (char*) malloc(strlen(tokens[i + 1]) + 1);
					strncpy(session_mem[i], tokens[i + 1], strlen(tokens[i + 1]) + 1);
				}

				gender = session_mem[1];
				nativity = session_mem[2];
				strcpy(email_addr_, tokens[1]);
			} else {
				if (write(client_handle, "ERROR", 5) < 0) {
                                        perror("ERROR : Failed to send ERROR message for failure to send verification code.\n");
                                       	continue;
				}
			}
		} else if (!strcmp(tokens[0], "VALIDATE")) {
			error_code = validate_consent_code(consent_code, email_addr_);
			if (error_code == 1) {
				fprintf(stderr, "ERROR : An error occured when fetching data.\n");
				if (write(client_handle, "ERROR", 5) < 0) {
                                        perror("ERROR : Failed to send ERROR acknowledgement for validating verification code.\n");
					continue;
				}
			} else if (error_code == 2) {
				fprintf(stderr, "INFO : Asking user to try again...\n");
				if (write(client_handle, "EMPTY", 5) < 0) {
                                        perror("ERROR : Failed to send EMPTY acknowledgement for validating verification code.\n");
                                        continue;
				}
			}

			if (write(client_handle, "OK", 2) < 0) {
                                        perror("ERROR :	Failed to send OK acknowledgement for validating verification code.\n");
                                        continue;
                         }

			fprintf(stdout, "INFO : Successfully validated verification code %s from %s.\n", consent_code, email_addr_);
			if (!consent_code) free(consent_code);
		} else if (!strcmp(tokens[0], "UPLOAD")) {

			char filename[100];
			strcpy(filename, tokens[1]);
			size_t filesize = atol(tokens[2]);

			fprintf(stdout, "INFO : filename => %s | filesize => %ld.\n", filename, filesize);

			wavaudio_t *audio = (wavaudio_t*) malloc(sizeof(wavaudio_t));
			if (!audio) {
				perror("ERROR : Memory allocation failed for wave audio file.\n");
			}

			if (write(client_handle, "OK", 2) < 0) {
                        	perror("ERROR : Failed to send OK Aknowledgement for uploading audio file");
                                continue;
                       	}

			fprintf(stdout, "INFO : Receiving file..\n");
			sleep(1);

			if (read(client_handle, audio_buffer, filesize) <= 0) {
				perror("ERROR : Failed to read file bytes into the audio buffer.\n");
				continue;
			}

			fprintf(stdout, "INFO : Successfully read %ld bytes.\n", filesize);

			strcpy(audio->owner, email_addr_);
			strcpy(audio->gender, gender);
			strcpy(audio->is_native, nativity);

			audio->channel = 1;
			audio->format = 16;
			audio->samplerate = 16000;

			if (wavaudio_set_pcm(audio, audio_buffer, filesize)) {
				fprintf(stderr, "ERROR : Couldn't set the audio pcm.\n");
				continue;
			}

			wavaudio_print(*audio);

			if (wavaudio_insert(*audio)) {
				fprintf(stderr, "ERRPR : Failed to insert the audio\n");
				continue;
			}

			if (audio) free(audio);
			for (int i = 0; i < 3; i++) {
    				if (session_mem[i]) {
        				free(session_mem[i]);
        				session_mem[i] = NULL; // Avoid dangling pointers.
    				}
			}
		}
		close(client_handle);
	}


	return NULL;
}

static char *ip = "127.0.0.1";
static int16_t port = 5299;

static bool ping_dotnet() {

	int sock;
    	struct sockaddr_in server_addr;

    	sock = socket(AF_INET, SOCK_STREAM, 0);
    	if (sock < 0) {
        	perror("ERROR : Socket creation failed");
        	return 0;
    	}

	memset(&server_addr, 0, sizeof(server_addr));
    	server_addr.sin_family = AF_INET;
    	server_addr.sin_port = htons(port);

    	if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        	perror("ERROR : Invalid IP address");
        	close(sock);
        	return 0;
    	}

    	if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        	perror("Connection failed");
        	close(sock);
        	return 0;
    	}

    return 1;
}

static int8_t count = 0;

void* cancel_registrer_routine (void* args) {

	void *rv = malloc (sizeof(int8_t));
	if (!rv) {
		perror("ERROR : Memory allocation for return value of cancel registrer thread failed.\n");
		count  = -1;
		return ((void*) &count);
	}

	*((int8_t*)rv) = 0;
	check:
    	while (ping_dotnet()) {
        	sleep(2); // Wait for 2 seconds before retrying
    	}

   	if (count < 5) {
        	fprintf(stderr, "ATTEMPTING RECONNECTION ...\n");
        	count++;
        	goto check;
    	}

    	if (pthread_cancel(start_registrer)) {
        	fprintf(stderr, "ERROR : Cancel gateway thread failed.\n");
        	*((int*)rv) = -1;
    	}

    	return rv;
}
