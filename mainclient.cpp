#include <iostream>           // cout
#include <sys/socket.h>       // socket(), connect(), send(), recv()
#include <arpa/inet.h>        // sockaddr_in, inet_addr()
#include <stdlib.h>           // atoi(), exit()
#include <cstring>            // memset()
#include <unistd.h>           // close()

#define MAX_MSG_SIZE 128      /// Max size of user data/msg

using namespace std;

/**
* Sends data to the TCP server that the client is connected to
* @param sock socket to send the data to
* @param buffer Container to that stores data to send
* @param msg_size size of `buffer`
* @return   status code
*/
int sendData(int sock, char *buffer, int msg_size);

/* Main driver for the master server */
int main(int argc, char *argv[])
{
    int client_sock;
    struct sockaddr_in server_addr;
    char inBuffer[MAX_MSG_SIZE];                /// buffer that accepts incoming messages
    char outBuffer[MAX_MSG_SIZE];               /// buffer that stores data to send to server
    char msg[MAX_MSG_SIZE];                     /// client data to be transformed
    int bytesRcv;

    bool hasMsg = false;                        /// Flag that makes sure that the client has data to transform

    /* Check for argument errors */
    if (argc != 3)
    {
        cout << "Usage: " << argv[0] << "<Server IP> <Server Port>" << endl;
        exit(1);
    }

    /* Create TCP Socket */
    client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_sock < 0)
    {
        cout << "client: socket() failed" << endl;
        exit(1);
    }

    /* Free up the port */
    int yes = 1;
    if (setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
        cout << "client: setsockopt() failed" << endl;
        exit(1);
    }

    /* Initialize server info */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    /* Connect to master server */
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        cout << "client: connect() failed" << endl;
        exit(1);
    }

    /* Run until client exits */
    while (1)
    {
        /* clearing buffers */
        memset(&msg, 0, MAX_MSG_SIZE);
        memset(&inBuffer, 0, MAX_MSG_SIZE);
        memset(&outBuffer, 0, MAX_MSG_SIZE);

        cout << "Actions" << endl
            << "0 - Exit" << endl
            << "1 - Set message" << endl
            << "2 - Transfomation" << endl
            << "Please enter an option: ";
        fgets(msg, MAX_MSG_SIZE, stdin);

        if (strncmp(msg, "0", 1) == 0)                                          // Case when client wants to exit
        {
            cout << "client: Shutting down service" << endl;
            strcpy(outBuffer, "EXT");                                           // EXT - exit status code for server

            /* Send data to the server */
            if ( sendData(client_sock, (char *) &outBuffer, strlen(outBuffer)) == -1) {
                cerr << "client: Might not be disconnected from server" << endl;
                close(client_sock);
                exit(1);
            }

            break;
        }

        else if (strncmp(msg, "1", 1) == 0)                                     // Case when client wants to set new data
        {
            memset(&msg, 0, MAX_MSG_SIZE);

            cout << "Enter message:" << endl;
            fgets(msg, MAX_MSG_SIZE-4, stdin);

            string set_temp = "SET " + string(strtok(msg, "\n"));               // SET - tells that data is to be stored as client data
            strcpy(outBuffer, set_temp.c_str());

            /* Send data to the server */
            if ( sendData(client_sock, (char *) &outBuffer, strlen(outBuffer)) == -1) {
                cerr << "client: could not send message to master server" << endl;
                close(client_sock);
                exit(1);
            }

            hasMsg = true;                                                      // allows client to send trans sequence
        }
        else if (strncmp(msg, "2", 1) == 0)                                     // Case when client wants to transform data
        {
            if ( !hasMsg ) {                                                    // Case when client does not have data to transform
                cout << "client: You do not have data to transform" << endl;
                continue;
            }

            memset(&msg, 0, MAX_MSG_SIZE);

            cout << "Enter transformation sequence (1-identity, 2-reverse, 3-upper, 4-lower, 5-ceasar, 6-yours):" << endl;
            fgets(msg, MAX_MSG_SIZE-4, stdin);

            string trn_temp = "TRN " + string(strtok(msg, "\n"));               // TRN - tells the server that it needs to perform trnasformations on client data
            strcpy(outBuffer, trn_temp.c_str());

            /* Sending data to the server */
            if ( sendData(client_sock, (char *) &outBuffer, strlen(outBuffer)) == -1 ) {
                cerr << "client: could not send transformation sequence to  master server" << endl;
                close(client_sock);
                exit(1);
            }

            /* Receiving data to the server */
            bytesRcv = recv(client_sock, (char *) &inBuffer, strlen(inBuffer), 0);
            if ( bytesRcv < 0 )
            {
                cout << "Client: recv() failed" << endl;
                break;
            }
            cout << "Received: " << inBuffer << endl << endl;
        }

    }

    close(client_sock);
    exit(0);
}


int sendData(int sock, char *buffer, int msg_size) {
    int bytesSent = send(sock, buffer, msg_size, 0);
    if ( bytesSent < 0 || bytesSent != strlen(buffer) )
    {
        cout << "client: error in sending" << endl;
        return -1;
    }
    return 0;
}
