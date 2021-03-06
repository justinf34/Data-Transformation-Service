#include <iostream>     					// cout
#include <sys/socket.h> 					// socket(), bind(), sendto(), recvfrom()
#include <arpa/inet.h>  					// for sockaddr_in, inet_addr()
#include <netinet/in.h>
#include <stdlib.h>     					// for atoi(), exit()
#include <unistd.h>     					// for close()
#include <cstring>      					// for memset()
#include <csignal>
#include "udp_serv.h"						// initSock()

using namespace std;

int serverSock;								/// server socket

/**
* Handles incoming signals
*
* @param sig the signal code that this process receives
*/
void signalHandler ( int sig ){
	close(serverSock);
	exit(0);
}

/* Main driver for this server */
int main(int argc, char *argv[])
{
	/* Assign signal handler */
	signal(SIGTERM, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGKILL, signalHandler);

	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	unsigned int len = sizeof(serverAddr);

	char inBuffer[MAX_MSG_LEN];								/// buffer container for incoming messages
	char outBuffer[MAX_MSG_LEN];							/// buffer container for outgoing messages
	int bytesRcv;

	/* Argument error checking */
	if ( argc != 2)
	{
		cerr << "Usage: " << argv[0] << " <Port num>" << endl;
	}

	/* Initializing server socket */
	int initSock = init_servSock(serverSock, atoi(argv[1]), &serverAddr);
	if ( initSock < 0 )
	{
		cout << "UDP identity: could not initialize socket" << endl;
		exit(-1);
	}

	/* Run forever until a terminate signal arrives */
	while (1)
	{
		/* Clearing buffers */
		memset(&inBuffer, 0, MAX_MSG_LEN);
		memset(&outBuffer, 0, MAX_MSG_LEN);

		/* Receive data from clients */
		bytesRcv = recvfrom(serverSock, inBuffer, MAX_MSG_LEN, 0, (struct sockaddr*) &clientAddr, &len);
		if ( bytesRcv < 0 ) {
			cerr << "UDP identity: recvfrom() failed, or connection closed" << endl;
			exit(-1);
		}

		cout << "UDP identity Client " << inet_ntoa(clientAddr.sin_addr) << ":" << clientAddr.sin_port << ": " << inBuffer << endl;

		/* Sent data back to client */
		sendto(serverSock, inBuffer, bytesRcv, 0, (struct sockaddr *) &clientAddr, len);

		/* Clearing buffer */
		memset(&inBuffer, 0, MAX_MSG_LEN);
		memset(&outBuffer, 0, MAX_MSG_LEN);
	}

	close(serverSock);
	return 0;
}
