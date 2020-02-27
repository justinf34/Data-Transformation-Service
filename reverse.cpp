#include <iostream>     /// cout
#include <sys/socket.h> /// socket(), bind(), sendto(), and recv
#include <arpa/inet.h>  /// for sockaddr_in, inet_addr()
#include <stdlib.h>     /// for atoi(), exit()
#include <cstring>      /// for memset()
#include <unistd.h>     /// for close()
#include <csignal>
#include "udp_serv.h"

using namespace std;

int serverSock;

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
	    unsigned int clientAddr_size;

	    char inBuffer[MAX_MSG_LEN];
	    char outBuffer[MAX_MSG_LEN];
	    int bytesRcv;
	    int bytesSent;

	    if ( argc != 2)
	    {
	        cerr << "Usage: " << argv[0] << " <Port num>" << endl;
	    }

			/* Initializin server socket */
			int initSock = init_servSock(serverSock, atoi(argv[1]), &serverAddr);
			if ( initSock < 0 )
			{
				cout << "UDP reverse: could not initialize socket" << endl;
				exit(-1);
			}

			/* Run forever until a terminate signal arrives */
	    while (1)
	    {
	        clientAddr_size = sizeof(clientAddr);

					/* Receive data from clients */
	        bytesRcv = recvfrom(serverSock, (char *) &inBuffer, MAX_MSG_LEN, 0, (struct sockaddr *) &clientAddr, &clientAddr_size);
	        if ( bytesRcv < 0 ) {
	            cerr << "UDP reverse: recvfrom() failed, or connection closed" << endl;
	            exit(-1);
	        }

	        cout << "UDP reverse Client " << inet_ntoa(clientAddr.sin_addr) << ":" << clientAddr.sin_port << ": " << inBuffer << endl;

					/* Reverse client data */
					int i;
					int n = strlen(inBuffer);
					for (i=0; i < n / 2; i++)
					{
							swap(inBuffer[i], inBuffer[n - i - 1]);
					}

					cout << "UDP reverse: After transformation -> " << inBuffer << endl;

					/* Sent data back to client */
	        bytesSent = sendto(serverSock, (char *) &inBuffer, bytesRcv, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
	        if ( bytesSent < 0 || bytesSent != bytesRcv)
	        {
	            cerr << "UDP reverse: sendto() failed" << endl;
	            exit(-1);
	        }

	        memset(&inBuffer, 0, MAX_MSG_LEN);
	        memset(&outBuffer, 0, MAX_MSG_LEN);
	    }

	    close(serverSock);
	    return 0;
}
