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

	if ( argc != 2)
	{
		cerr << "Usage: " << argv[0] << " <Port num>" << endl;
	}

	/* Initializing server socket */
	int initSock = init_servSock(serverSock, atoi(argv[1]), &serverAddr);
	if ( initSock < 0 )
	{
		cout << "UDP upper: could not initialize socket" << endl;
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
			cerr << "UDP upper: recvfrom() failed, or connection closed" << endl;
			exit(-1);
		}

		/* upper case all lower cased letters */
		int i;
		int n = strlen(inBuffer);
		for (i=0; i < n; i++)
		{
			inBuffer[i] = toupper(inBuffer[i]);
		}

		cout << "UDP upper Client " << inet_ntoa(clientAddr.sin_addr) << ":" << clientAddr.sin_port << ": " << inBuffer << endl;

		/* Sent data back to client */
		sendto(serverSock, inBuffer, strlen(inBuffer), 0, (struct sockaddr *) &clientAddr, len);

		/* Clearing buffer */
		memset(&inBuffer, 0, MAX_MSG_LEN);
		memset(&outBuffer, 0, MAX_MSG_LEN);
	}

	close(serverSock);
	return 0;
}
