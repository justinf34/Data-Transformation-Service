#include <iostream>
#include <unistd.h>
#include <cstring>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define BUFFER_SIZE 40



int main(int argc, char *argv[])
{
    int server_sock;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    unsigned int size;

    char inBuffer[BUFFER_SIZE];
    // char outBuffer[BUFFER_SIZE];
    int bytesSent;
    int bytesRecv;

    /// Error checking
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <Port num>" << endl;
        exit(1);
    }


    /// Create UDP socket
    server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if ( server_sock < 0 )
    {
        cerr << "UDP5: socket() failed" << endl;
        exit(1);
    }


    /// Free up the port before binding
    int opt = 1;
    if ( setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0 )
    {
        cerr << "UDP5: setsockopt() failed" << endl;
        exit(1);
    }

    /// Initialize server info
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(atoi(argv[1]));


    /// Bind socket to local address
    if ( bind(server_sock, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0 )
    {
        cerr << "UDP5: bind() failed" << endl;
        exit(1);
    }

    while(1) 
    {
        size = sizeof(clientAddr);

        /// Receive message from client(s)
        bytesRecv = recvfrom(server_sock, (char *) &inBuffer, BUFFER_SIZE, 0, (struct sockaddr *) &clientAddr, &size);


        /// Ceasar Cipher
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

        bytesSent = sendto(server_sock, (char *) &inBuffer, bytesRecv, 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
        if ( bytesSent < 0 || bytesSent != bytesRecv )
        {
            cerr << "UDP5: sendto() failed" << endl;
            exit(1);
        }

    }

}