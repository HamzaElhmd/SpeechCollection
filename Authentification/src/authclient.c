#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define SOCKET_NAME "/tmp/auth.sock"

int main (int argc, char * argv[])
{
	struct sockaddr_un auth_server_addr;
	socklen_t addrlen = sizeof(auth_server_addr);
	int8_t error_code, auth_client_socket;
	ssize_t packet_s;
	char *request = "VALIDATE 321987";
	fprintf(stdout, "COMMAND SENT : %s\n", request);

	auth_client_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (auth_client_socket == -1)
	{
		fprintf(stderr, "Error creating UNIX socket.\n");
		return 1;
	}

	memset(&auth_server_addr, 0, addrlen);
	auth_server_addr.sun_family = AF_UNIX;
	strcpy(auth_server_addr.sun_path, SOCKET_NAME);

	error_code = connect(auth_client_socket, 
			(struct sockaddr *) &auth_server_addr, addrlen);
	if (error_code == -1)
	{
		fprintf(stderr, "Failed to connect to UNIX server %s\n", SOCKET_NAME);
		return 1;
	}

	fprintf(stdout, "Connected to server %s\n", SOCKET_NAME);

	packet_s = write(auth_client_socket, request, strlen(request));

	fprintf(stdout, "COMMAND SENT AFTER WRITE() : %s\n", request);

	fprintf(stdout, "Wrote %d bytes to the server %s.\nMessage:%s.\n", packet_s, SOCKET_NAME, request);

	close(auth_client_socket);
	return 0;
}
