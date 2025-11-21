/*
 * main.c
 *
 * TCP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP client
 * portable across Windows, Linux and macOS.
 */


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
                printf("Errore: numero di porta non valido\n");
                return 1;
            }
            i++;
        }
        else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            request_str = argv[i + 1];
            i++;
        }
        else {
            printf("Utilizzo: %s [-s server] [-p port] -r \"tipo citta\"\n", argv[0]);
            return 1;
        }
    }

    if (request_str == NULL) {
        printf("Errore: -r è obbligatorio!\n");
        printf("Uso: %s [-s server] [-p port] -r \"tipo citta\"\n", argv[0]);
        return 1;
    }

    char type;
    char city[64];

    if (sscanf(request_str, "%c %s", &type, city) != 2) {
        printf("Errore: la richiesta è espressa in modo errato\n");
        printf("Forma corretta: \"t citta\" (esempio: \"t bari\")\n");
        return 1;
    }

#if defined WIN32
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Errore nella funzione WSAStartup()\n");
		return 0;
	}
#endif
	int c_socket;

	c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (c_socket < 0) {
	    printf("Creazione della socket fallita\n");
	    clearwinsock();
	    return 1;
	}

	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));

	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(server_ip);
	sad.sin_port = htons(port);


	if (connect(c_socket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
	   printf("Errore di connessione al server\n");
	   closesocket(c_socket);
	   clearwinsock();
	   return 1;
	}
	weather_request_t request;
	request.type = type;
	strcpy(request.city, city);

	if (send(c_socket, (char*)&request, sizeof(request), 0) < 0) {
	    printf("Errore: funzione send() fallita\n");
	    closesocket(c_socket);
	    clearwinsock();
	    return 1;
	}

	weather_response_t response;
	int bytes_rcvd = recv(c_socket, (char*)&response, sizeof(response), 0);

	if (bytes_rcvd <= 0) {
	    printf("Errore: funzione recv() fallita\n");
	    closesocket(c_socket);
	    clearwinsock();
	    return 1;
	}

	if (bytes_rcvd != sizeof(response)) {
	    printf("Errore: ricevuti dati incompleti (%d bytes al posto di %d)\n", bytes_rcvd, sizeof(response));
	    closesocket(c_socket);
	    clearwinsock();
	    return 1;
	}

	printf("Ricevuto risultato dal server ip %s. ", server_ip);

	if (response.status == STATUS_OK) {

	    switch (response.type) {
	        case 't':
	            printf("%s: Temperatura = %.1f°C\n", city, response.value);
	            break;

	        case 'h':
	            printf("%s: Umidità = %.1f%%\n", city, response.value);
	            break;

	        case 'w':
	            printf("%s: Vento = %.1f km/h\n", city, response.value);
	            break;

	        case 'p':
	            printf("%s: Pressione = %.1f hPa\n", city, response.value);
	            break;

	        default:
	            printf("Unknown response type\n");
	            break;
	    }

	} else {
	    switch (response.status) {
	        case STATUS_CITY_NOT_FOUND:
	            printf("Città non disponibile\n");
	            break;

	        case STATUS_INVALID_REQUEST:
	            printf("Richiesta non valida\n");
	            break;

	        default:
	            printf("Errore sconosciuto\n");
	            break;
	    }
	}


	closesocket(c_socket);

	printf("Client terminato.\n");

	clearwinsock();
	return 0;
}
