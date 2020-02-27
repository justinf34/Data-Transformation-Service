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

int main(int argc, char *argv[])
{
	/* Assign signal handler */
	signal(SIGTERM, signalHandler);
	signal(SIGINT, signalHandler);

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
		cout << "UDP yours: could not initialize socket" << endl;
		exit(-1);
	}

	/* Run forever until a terminate signal arrives */
	while (1)
	{
		/* Clearing buffers */
		memset(&inBuffer, 0, MAX_MSG_LEN);
		memset(&outBuffer, 0, MAX_MSG_LEN);


		/* Receive data from clients */
		bytesRcv = recvfrom(serverSock, inBuffer, 1024, 0, (struct sockaddr*) &clientAddr, &len);
		if ( bytesRcv < 0 ) {
			cerr << "UDP yours: recvfrom() failed, or connection closed" << endl;
			exit(-1);
		}

		/* Chnage char depending on their ascii value */
		int i;
		int n = strlen(inBuffer);
		for (i=0; i < n; i++)
		{
			int ascii_val = int(inBuffer[i]);
			cout << inBuffer[i] << ": " << ascii_val << endl;
			if ( (ascii_val % 2) == 0 ) { inBuffer[i] = 48; }
			else { inBuffer[i] = 49; }
		}

		cout << "UDP yours Client " << inet_ntoa(clientAddr.sin_addr) << ":" << clientAddr.sin_port << ": " << inBuffer << endl;

		/* Sent data back to client */
		sendto(serverSock, inBuffer, strlen(inBuffer), 0, (struct sockaddr *) &clientAddr, len);

		/* Clearing buffers */
		memset(&inBuffer, 0, MAX_MSG_LEN);
		memset(&outBuffer, 0, MAX_MSG_LEN);
	}

	close(serverSock);
	return 0;
}
