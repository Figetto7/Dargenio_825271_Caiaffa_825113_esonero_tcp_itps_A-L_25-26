// TCP Client - Template for Computer Networks assignment


#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

#ifndef NO_ERROR
#define NO_ERROR 0
#endif


void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

int main(int argc, char *argv[]) {
    //Default Values
    char *server_ip = "127.0.0.1";
    int port = SERVER_PORT;
    char *request_str = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            server_ip = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[i + 1]);
            if (port <= 0 || port > 65535) {
                printf("Error: port number invalid (1-65535)\n");
                return 1;
            }
            i++;
        }
        else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            request_str = argv[i + 1];
            i++;
        }
        else {
            printf("Usage: %s [-s server] [-p port] -r \"type city\"\n", argv[0]);
            return 1;
        }
    }

    if (request_str == NULL) {
        printf("Error: -r is mandatory!\n");
        printf("Usage: %s [-s server] [-p port] -r \"type city\"\n", argv[0]);
        return 1;
    }

    char type;
    char city[64];

    if (sscanf(request_str, "%c %s", &type, city) != 2) {
        printf("Error: invalid form of request\n");
        printf("Correct form: \"t city\" (example: \"t bari\")\n");
        return 1;
    }

#if defined WIN32
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif
	int c_socket;

	c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (c_socket < 0) {
	    printf("Socket creation failed\n");
	    clearwinsock();
	    return 1;
	}

	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));

	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(server_ip);
	sad.sin_port = htons(port);

	printf("Server address configured: %s:%d\n", server_ip, port);

	if (connect(c_socket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
	   printf("Connection to server failed\n");
	   closesocket(c_socket);
	   clearwinsock();
	   return 1;
	}
	weather_request_t request;
	request.type = type;
	strcpy(request.city, city);

	if (send(c_socket, (char*)&request, sizeof(request), 0) < 0) {
	    printf("Error: send() failed\n");
	    closesocket(c_socket);
	    clearwinsock();
	    return 1;
	}

	weather_response_t response;
	int bytes_rcvd = recv(c_socket, (char*)&response, sizeof(response), 0);

	if (bytes_rcvd <= 0) {
	    printf("Error: recv() failed or connection closed\n");
	    closesocket(c_socket);
	    clearwinsock();
	    return 1;
	}

	if (bytes_rcvd != sizeof(response)) {
	    printf("Error: received incomplete data (%d bytes instead of %d)\n", bytes_rcvd, sizeof(response));
	    closesocket(c_socket);
	    clearwinsock();
	    return 1;
	}

	printf("Result received from Server %s: ", server_ip);

	if (response.status == STATUS_OK) {

	    switch (response.type) {
	        case 't':
	            printf("%s: Temperature = %.1f°C\n", city, response.value);
	            break;

	        case 'h':
	            printf("%s: Humidity = %.1f%%\n", city, response.value);
	            break;

	        case 'w':
	            printf("%s: Wind = %.1f km/h\n", city, response.value);
	            break;

	        case 'p':
	            printf("%s: Pressure = %.1f hPa\n", city, response.value);
	            break;

	        default:
	            printf("Unknown response type\n");
	            break;
	    }

	} else {
	    switch (response.status) {
	        case STATUS_CITY_NOT_FOUND:
	            printf("City not available\n");
	            break;

	        case STATUS_INVALID_REQUEST:
	            printf("Invalid request\n");
	            break;

	        default:
	            printf("Unknown error\n");
	            break;
	    }
	}


	closesocket(c_socket);

	printf("Client terminated.\n");

	clearwinsock();
	return 0;
}
