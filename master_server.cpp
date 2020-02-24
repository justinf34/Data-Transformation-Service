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


struct micro_serv {
    string address;
    int port_num;
};

int clientSock;

/// Main driver for master_server.cpp
int main(int argc, char *argv[])
{
    int pid;
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


    /// Start listening for incoming connections
    if ( listen(listener_sock, 5) == -1 )
    {
        cerr << "master server: listen()" << endl;
        exit(1);
    }


    cout << "The master_server is now ready to accept connections!!" << endl;
    cout << "master_server: listening to TCP port " << atoi(argv[1]) << "..." << endl;

    while (1)
    {
        /// Accepting a connection
        if ( (clientSock = accept(listener_sock, NULL, NULL)) == -1) 
        {
            cerr << "master_server: accept() failed" << endl;
            exit(1);
        }

        cout << "master server: accepted a connection" << endl;

        pid = fork();

        if (pid < 0)
        {

            cerr << "master_server: fork() failed" << endl;
        }
        else if (pid == 0)
        {
            int byteRcv = 0;
            int len;
            char data[MAX_MSG_LEN * 10];
            char trans[MAX_MSG_LEN *  10];
            struct micro_serv services[6];


            /// Delete later
            services[0].address = "127.0.0.1";
            services[0].port_num = 8085;


            /// Flags
            bool set_data = false;
            bool set_trans = false;

            memset(&inBuffer, 0, MAX_MSG_LEN);
            memset(&outBuffer, 0, MAX_MSG_LEN);

            /// Keep running until the user wants to terminate connection
            while (1)
            {


            } /// End of the client handler while loop
        }

        else
        {
            close(clientSock);
        }
    }

    return 0;
}




            //     /// Receive client message
            //     while ((len = recv(clientSock, (char *)&inBuffer, MAX_MSG_LEN, 0)) > 0)
            //     {
            //         cout << "Client handler: received " << len << endl;
            //         if (set_data || set_trans ){
            //             if ( set_data ) { strncat(data, inBuffer, len); }
            //             else { strncat(data, inBuffer, len); }
            //         }
            //         else
            //         {
            //             if ( strncmp(inBuffer, "EXT", 3) == 0 )
            //             {
            //                 cout << "Client handler: closing connection with client" << endl;
            //                 close(clientSock);
            //                 exit(1); 
            //             }
            //             else if ( strncmp(inBuffer, "SET", 3) == 0 )
            //             {
            //                 cout << "Client handler: setting data for client" << endl;
            //                 memset(&data, 0, sizeof(data));
            //                 strcpy(data, inBuffer + 3);
            //                 set_data = true;
            //             }
            //             else if ( strncmp(inBuffer, "TRN", 3) == 0 )
            //             {
            //                 strcpy(trans, inBuffer + 3);
            //                 cout << "Client handler: setting transformation sequence for client" << endl;
            //                 memset(&trans, 0, sizeof(trans));
            //                 set_trans = true;
            //             }
            //             else if ( strncmp(inBuffer, "OFF", 3) == 0 )
            //             {
            //                 cout << "Client handler: turning off microservice server" << endl;
            //                 /// Turn service off and let client know
            //                 /// Need to parse out the service number
            //                 /// Send the right signal to the correct pid
            //                 /// Let client know if sucess or not
            //                 continue;
            //             }
            //             else if ( strncmp(inBuffer, "ONS", 3) == 0 )
            //             {
            //                 cout << "Client handler: turning on microservice server" << endl;
            //                 /// Turn on service and let client know if successful
            //                 /// Need to parse out the service number and port
            //                 /// Start the correct process with the right arguments
            //                 /// Let the client know if success or not
            //                 continue;
            //             }
            //         }

            //         memset(&inBuffer, 0, sizeof(inBuffer));
            //     }   //// End of receving client msg while loop


            //     if ( set_data ) {
            //         cout << "Client handler: client data -> " << data << endl; 
            //         set_data = false; 
            //     }
            //     else if ( set_trans )
            //     {
            //         cout << "Client handler: client transformation -> " << trans << endl;
            //         /// Connect to the correct service
            //         int send_sock;
            //         struct sockaddr_in servAddr;
            //         struct sockaddr_in fromAddr;
            //         unsigned int size;

            //         int msgLen;
            //         int bytesSent;

            //         /// Create a UDP socket
            //         send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            //         if ( send_sock < 0)
            //         {
            //             cerr << "Client handler: socket() failed" << endl;
            //             close(clientSock);
            //             exit(1);
            //         }


            //         /// Free up the port
            //         int opt = 1;
            //         if ( setsockopt(send_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0 )
            //         {
            //             cout << "Client handler: setsockopt() failed" << endl;
            //             close(clientSock);
            //             exit(1);
            //         }
                    

            //         /// Initialize server information
            //         memset(&servAddr, 0, sizeof(servAddr));
            //         servAddr.sin_family = AF_INET;
            //         servAddr.sin_port = htons(services[0].port_num);
            //         servAddr.sin_addr.s_addr = inet_addr(services[0].address.c_str());

            //         msgLen = strlen(data);


            //         /// Send data
            //         bytesSent = sendto(send_sock, (char *) &data, msgLen, 0, (struct sockaddr *) &servAddr, sizeof(servAddr));
            //         if ( bytesSent != msgLen )
            //         {
            //             cout << "Client handling: error in sending" << endl;
            //             close(clientSock);
            //             exit(1);
            //         }

            //         /// Receive data
            //         byteRcv = 0;
            //         int len;
            //         size = sizeof(fromAddr);
            //         while ( len = recvfrom(send_sock, (char *) &inBuffer, MAX_MSG_LEN, 0, (struct sockaddr *) &fromAddr, &size ) > 0)
            //         {
            //             strcpy(outBuffer, inBuffer);

            //             /// send back to client
            //             bytesSent = send(clientSock, (char *) &outBuffer, strlen(outBuffer), 0);
            //             if ( bytesSent < 0 || bytesSent != strlen(outBuffer) )
            //             {
            //                 cout << "Client Handler: send() failed" << endl;
            //                 close(clientSock);
            //                 exit(1);
            //             }

            //             memset(&inBuffer, 0, sizeof(inBuffer));
            //             memset(&outBuffer, 0, sizeof(outBuffer));
            //         }
            //         close(send_sock);

            //         set_trans = false;
            //         memset(&trans, 0, sizeof(trans));
            //     }
