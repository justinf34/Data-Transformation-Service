#include <iostream>     /// cout
#include <sys/socket.h> /// socket(), bind(), sendto(), and recv
#include <arpa/inet.h>  /// for sockaddr_in, inet_addr()
#include <stdlib.h>     /// for atoi(), exit()
#include <cstring>      /// for memset()
#include <unistd.h>     /// for close()
#include <ctype.h>      /// touuper()
#include <csignal>

#define MSG_SIZE 100

using namespace std;


int serverSock;

void signalHandler ( int sig ){
	close(serverSock);
	exit(0);
}

int main(int argc, char *argv[])
{

    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);


    struct sockaddr_in serverAddr; 
    struct sockaddr_in clientAddr;
    unsigned int clientAddr_size;

    char inBuffer[MSG_SIZE];
    char outBuffer[MSG_SIZE];
    int bytesRcv;
    int bytesSent;
    int temp = 0;


    if ( argc != 2) 
    {
        cerr << "Usage: " << argv[0] << " <Port num>" << endl;
    }

    /// Initialize server info
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(atoi(argv[1]));


    /// Create UDP socket
    serverSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if ( serverSock < 0 ) 
    {
        cerr << "UDP4: socket() failed" << endl;
        exit(-1);
    }


    /// Clear up the port
    int yes = 1;
    if ( setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0 )
    {
        cerr << "UDP4: setsockopt() failed" << endl;
        exit(-1);        
    }


    /// Bind socket to the port
    if ( bind(serverSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0 )
    {
        cerr << "UDP4: bind() failed" << endl;
        exit(-1);        
    }


    while (1)
    {

        clientAddr_size = sizeof(clientAddr);
        bytesRcv = recvfrom(serverSock, (char *) &inBuffer, MSG_SIZE, 0, (struct sockaddr *) &clientAddr, &clientAddr_size);
        temp += 1;
        if ( bytesRcv < 0 ) {
            cerr << "UDP4: recvfrom() failed, or connection closed" << endl;
            exit(-1);
        }

        cout << "UDP4 Client " << inet_ntoa(clientAddr.sin_addr) << ":" << clientAddr.sin_port << ": " << inBuffer << endl;
        cout << temp << endl;

        int i;
        int n = strlen(inBuffer);
        for (i=0; i < n; i++)
        {
            tolower(inBuffer[i]);
        }

        cout << "UDP4: Result of service -> " << inBuffer << endl;

        bytesSent = sendto(serverSock, (char *) &inBuffer, bytesRcv, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
        if ( bytesSent < 0 || bytesSent != bytesRcv)
        {
            cerr << "UDP4: sendto() failed" << endl;
            exit(-1);
        }

        memset(&inBuffer, 0, MSG_SIZE);
        memset(&outBuffer, 0, MSG_SIZE);

    }
    
    close(serverSock);

    return 0;
}

