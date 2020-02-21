#include <iostream> /// cout
#include <cstring>  /// string, memset()
#include <stdlib.h> /// exit()
#include <stdio.h>  /// memset()
#include <unistd.h> /// close

/// Network related
#include <sys/socket.h> /// socket(), connect(), send(), and recv()
#include <arpa/inet.h>  /// sockaddr_in, inet_addr()
#include <netdb.h>
#include <netinet/in.h>

// fork()
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

/// Constants
#define MAX_MSG_LEN 100

/***
 *  This signal handler to gracefully exit
 *  by losing all the sockets
 *  
 **/
void sig_handler(int sig);

/***
 *  Handles all client connection including storing 
 *  message and handling sequence response
 * 
 **/
void client_handler();

/***
 *  Create a tcp sender socket
 * 
 * 
 **/
void createSocket();

int clientSock;

/// Main driver for master_server.cpp
int main(int argc, char *argv[])
{
    pid_t pid = fork();
    struct sockaddr_in server;
    int listener_sock;
    char inBuffer[MAX_MSG_LEN];
    char outBuffer[MAX_MSG_LEN];

    /// Need to set up signal handler to catch some weird termination conditions.

    /// Error handling for invalid arguments
    if (argc != 2)
    {
        /// Need to check if it is a valid port number
        cout << "Usage: " << argv[0] << " <Listening Port>" << endl;
        exit(1);
    }

    /// Initializing listerner socket
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    /// Set up the transposrt-level end point to use TCP
    if ((listener_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        cerr << "master_server: socket() failed" << endl;
        exit(1);
    }

    /// Free up the port before binding
    int yes = 1;
    if (setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)))
    {
        cerr << "master_server: setsockopt () failed" << endl;
        exit(1);
    }

    /// Bind address and port to the end point
    if (bind(listener_sock, (struct sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cerr << "master_server: bind() failed" << endl;
        exit(1);
    }

    cout << "The master_server is now ready to accept connections!!" << endl;
    cout << "master_server: listening to TCP port " << atoi(argv[1]) << "..." << endl;

    while (1)
    {
        /// Accepting a connection
        if ((clientSock = accept(listener_sock, NULL, NULL)) < 0)
        {
            cerr << "master_server: accept() failed" << endl;
            exit(1);
        }

        if (pid < 0)
        {

            cerr << "master_server: fork() failed" << endl;
        }
        else if (pid == 0)
        {
            int byteRcv = 0;
            int len;
            char msg[MAX_MSG_LEN];
            char transformations[MAX_MSG_LEN];

            //
            bool set_msg = false;
            bool set_trans = false;

            memset(&inBuffer, 0, MAX_MSG_LEN);
            memset(&outBuffer, 0, MAX_MSG_LEN);

            /// Keep running until the user wants to terminate connection
            while (1)
            {

                /// Receive client message
                while ((len = recv(clientSock, (char *)&inBuffer, MAX_MSG_LEN, 0)) > 0)
                {
                    continue;
                }
            }
        }
        else
        {

            close(clientSock);
        }
    }

    return 0;
}
