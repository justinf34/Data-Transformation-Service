#include <iostream>     /// cout
#include <sys/socket.h> /// socket(), bind(), sendto(), and recv
#include <arpa/inet.h>  /// for sockaddr_in, inet_addr()
#include <netinet/in.h>
#include <stdlib.h>     /// for atoi(), exit()
#include <unistd.h>     /// for close()
#include <cstring>      /// for memset()
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
	    unsigned int len = sizeof(serverAddr);

	    char inBuffer[MAX_MSG_LEN];
	    char outBuffer[MAX_MSG_LEN];
	    int bytesRcv;

	    if ( argc != 2)
	    {
	        cerr << "Usage: " << argv[0] << " <Port num>" << endl;
	    }

			/* Initializin server socket */
			int initSock = init_servSock(serverSock, atoi(argv[1]), &serverAddr);
			if ( initSock < 0 )
			{
				cout << "UDP ceasar: could not initialize socket" << endl;
				exit(-1);
			}

			/* Run forever until a terminate signal arrives */
	    while (1)
	    {
					memset(&inBuffer, 0, MAX_MSG_LEN);
					memset(&outBuffer, 0, MAX_MSG_LEN);


					/* Receive data from clients */
					bytesRcv = recvfrom(serverSock, inBuffer, MAX_MSG_LEN, 0, (struct sockaddr*) &clientAddr, &len);
	        if ( bytesRcv < 0 ) {
	            cerr << "UDP ceasar: recvfrom() failed, or connection closed" << endl;
	            exit(-1);
	        }

	        cout << "UDP ceasar Client " << inet_ntoa(clientAddr.sin_addr) << ":" << clientAddr.sin_port << ": " << inBuffer << endl;

          /* Encode in using Ceasar cipher */
          int i;
          for (i=0; i < strlen(inBuffer); i++)
          {
              if ( isalpha(inBuffer[i]) != 0 )
              {
                  if ( isupper(inBuffer[i]) )
                  { inBuffer[i] = char(int(inBuffer[i] + 13 -65) % 26 + 65); }
                  else
                  { inBuffer[i] = char(int(inBuffer[i] + 13 - 97) % 26 + 97); }
              }
          }

					/* Sent data back to client */
	       	sendto(serverSock, inBuffer, strlen(inBuffer), 0, (struct sockaddr *) &clientAddr, len);

	        memset(&inBuffer, 0, MAX_MSG_LEN);
	        memset(&outBuffer, 0, MAX_MSG_LEN);
	    }

	    close(serverSock);
	    return 0;
}
