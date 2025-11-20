//TCP Server - Template for Computer Networks assignment

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
#include <ctype.h>
#include <time.h>
#include "protocol.h"

#ifndef NO_ERROR
#define NO_ERROR 0
#endif
#define QLEN 6

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

int is_valid_city(const char *city) {
    const char* available_cities[] = {"bari", "roma", "milano", "napoli", "torino", "palermo", "genova", "bologna", "firenze", "venezia"};

    size_t n = sizeof(available_cities) / sizeof(available_cities[0]);

    for (size_t i = 0; i < n; i++) {
        #if defined WIN32
        if (_stricmp(city, available_cities[i]) == 0) {
        #else
        if (strcasecmp(city, available_cities[i]) == 0) {
        #endif
            return 1;
        }
    }
    return 0;
}

    int is_valid_type(char type) {
        char lower_type = tolower(type);
        return (lower_type == 't' || lower_type == 'h' || lower_type == 'w' || lower_type == 'p');
    }

float get_temperature(void) {
    return -10.0 + (rand() / (float)RAND_MAX) * 50.0;
}

float get_humidity(void) {
    return 20.0 + (rand() / (float)RAND_MAX) * 80.0;
}

float get_wind(void) {
    return (rand() / (float)RAND_MAX) * 100.0;
}

float get_pressure(void) {
    return 950.0 + (rand() / (float)RAND_MAX) * 100.0;
}



int main(int argc, char *argv[]) {

	srand(time(NULL));

	int port = SERVER_PORT;

	if (argc == 3) {
	    if (strcmp(argv[1], "-p") == 0) {
	        port = atoi(argv[2]);
	        if (port <= 0 || port > 65535) {
	            printf("Error: invalid port number (must be between 1-65535)\n");
	            return 1;
	        }
	    } else {
	        printf("Usage: %s [-p port]\n", argv[0]);
	        return 1;
	    }
	} else if (argc != 1) {
	    printf("Usage: %s [-p port]\n", argv[0]);
	    return 1;
	}

	printf("Server is listening on port %d...\n", port);

#if defined WIN32
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

	int my_socket;

	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (my_socket < 0) {
		printf("socket creation failed.\n");
	    clearwinsock();
	    return 1;
	}

	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = INADDR_ANY;
	sad.sin_port = htons(port);

	if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
	    printf("Error: bind() failed\n");
	    closesocket(my_socket);
	    clearwinsock();
	    return 1;
	}

	if (listen(my_socket, QLEN) < 0) {
	        printf("Error: listen() failed\n");
	        closesocket(my_socket);
	        clearwinsock();
	        return 1;
	    }

	struct sockaddr_in cad;
	int client_socket;
	int client_len;
	printf("Waiting for clients to connect...\n");

	while (1) {
	    client_len = sizeof(cad);

	    client_socket = accept(my_socket, (struct sockaddr *)&cad, &client_len);
	    if (client_socket < 0) {
	        printf("Error: accept() failed\n");
	        continue;
	    }

	    weather_request_t request;
	    int bytes_rcvd = recv(client_socket, (char*)&request, sizeof(request), 0);

	    if (bytes_rcvd != (int)sizeof(request)) {
	        printf("Error: recv() failed or incomplete request (%d bytes)\n", bytes_rcvd);
	        closesocket(client_socket);
	        continue;
	    }

	    printf("Request '%c %s' from client IP %s\n", request.type, request.city, inet_ntoa(cad.sin_addr));

	    weather_response_t response;

	    char req_type = (char) tolower((unsigned char) request.type);

	    if (!is_valid_type(req_type)) {
	        response.status = STATUS_INVALID_REQUEST;
	        response.type = '\0';
	        response.value = 0.0f;
	    } else if (!is_valid_city(request.city)) {
	        response.status = STATUS_CITY_NOT_FOUND;
	        response.type = '\0';
	        response.value = 0.0f;
	    } else {
	        response.status = STATUS_OK;
	        response.type = req_type;
	        switch (req_type) {
	            case 't': response.value = get_temperature(); break;
	            case 'h': response.value = get_humidity(); break;
	            case 'w': response.value = get_wind(); break;
	            case 'p': response.value = get_pressure(); break;
	            default:
	                response.status = STATUS_INVALID_REQUEST;
	                response.type = '\0';
	                response.value = 0.0f;
	        }
	    }

	    send(client_socket, (char*)&response, sizeof(response), 0);
	    closesocket(client_socket);
	}

	printf("Server terminated.\n");
	closesocket(my_socket);
	clearwinsock();
	return 0;
}
