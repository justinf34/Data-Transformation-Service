#include <iostream>         // cout
#include <cstring>          // string, memset()
#include <stdlib.h>         // exit()
#include <stdio.h>          // memset()
#include <unistd.h>         // close
#include <csignal>          // signal

/* Network related */
#include <sys/socket.h>     // socket(), connect(), send(), and recv()
#include <arpa/inet.h>      // sockaddr_in
#include <netdb.h>
#include <netinet/in.h>

/* fork() */
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

/* Constants */
#define MAX_MSG_LEN 128
#define SERVER_IP "127.0.0.1"


int clientSock;                             /// Client socket
int listener_sock;                          /// Client listener socket
int service_pid[6];                          /// Container for all process id of microservice servers

/***
 *  Kills all microservice servers by sending a SIGKILL signal
 ***/
void kill_servers()
{
    cout << "master server: closing microservice servers..." << endl;
    int i;
    for (i = 0; i < 6; i++ )
    {
        if ( kill(service_pid[i], SIGKILL) == -1 )
        {
            cout << "master server: could not kill process" << service_pid[i] << endl;
        }
    }
}

/***
 *  This signal handler to gracefully exit
 *  by losing all the sockets
 ***/
void signalHandler(int sig)
{
    close(clientSock);
    close(listener_sock);
    kill_servers();
    exit(0);
}

/***
* Transforms client data by sending data to microservice servers
***/
int transformData(char *trans_seq, char *data);

/* Main driver for master_server.cpp */
int main(int argc, char *argv[])
{
    int pid;                                /// Client handler process process id
    struct sockaddr_in server;

	/* Assign signal handler */
	signal(SIGTERM, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGKILL, signalHandler);

    /* Error handling for invalid arguments */
    if (argc != 2)
    {
        // Need to check if it is a valid port number
        cout << "Usage: " << argv[0] << " <Listening Port>" << endl;
        exit(1);
    }

    /* Initializing listerner socket */
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Set up the transposrt-level end point to use TCP */
    if ((listener_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        cerr << "master server: socket() failed" << endl;
        exit(1);
    }

    /* Free up the port before binding */
    int yes = 1;
    if (setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)))
    {
        cerr << "master server: setsockopt () failed" << endl;
        exit(1);
    }

    /* Binding to port */
    if ( bind(listener_sock, (struct sockaddr *) &server, sizeof(server)) < 0 )
    {
        cerr << "master server: bind() failed" << endl;
        exit(1);
    }

    /* Starting the microservice servers */
    cout << "master server: starting microservice servers" << endl;

    char *args[] = {"127.0.0.1", NULL, NULL};                   /// arguments for starting microservice server
    string server_name;                                         /// binary file name of server
    int i;

    for ( i = 0; i < 6; i++ )
    {
        pid = fork();
        if ( pid < 0 )                                            // Case when one forks failed
        {
            cout << "master server: Could not start one of the servers. Closing master server..." << endl;
            /* kill all other servers */
        }
        else if ( pid == 0 )
        {
            switch (i)                                          // Choosing the right binary file and port num
            {
                case 0:
                    args[1] = "8081";
                    server_name = "./identity";
                    break;
                case 1:
                    args[1] = "8082";
                    server_name = "./reverse";
                    break;
                case 2:
                    args[1] = "8083";
                    server_name = "./upper";
                    break;
                case 3:
                    args[1] = "8084";
                    server_name = "./lower";
                    break;
                case 4:
                    args[1] = "8085";
                    server_name = "./ceasar";
                    break;
                case 5:
                    args[1] = "8086";
                    server_name = "./yours";
                    break;
            }
            execve(server_name.c_str(), args, NULL);
        }       
        cout << "Server pid:" << pid << endl;
        service_pid[i] = pid;                                      // add the pid to list so it can be killed later
    }

    /* Start listening for incoming connections */
    if ( listen(listener_sock, 5) == -1 )
    {
        cerr << "master server: listen()" << endl;
        exit(1);
    }

    cout << "The master_server is now ready to accept connections!!" << endl;
    cout << "master_server: listening to TCP port " << atoi(argv[1]) << "..." << endl;

    while (1)
    {
        /* Accepting a connection */
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
            close(listener_sock);

            int bytesRcv;
            int bytesSnt;
            char *ptr;                                                          /// pointer to where the client data is stored in the message
            char data[MAX_MSG_LEN];                                             /// client data to be transformed
            char trans[MAX_MSG_LEN];                                            /// client transformation sequence
            char inBuffer[MAX_MSG_LEN];                                         /// buffer to store incoming messages
            char outBuffer[MAX_MSG_LEN];                                        /// buffer to send data to client

            /* Keep running until the user wants to terminate connection */
            while ( 1 )
            {
              /* Clear buffers */
              memset(&inBuffer, 0, MAX_MSG_LEN);
              memset(&outBuffer, 0, MAX_MSG_LEN);

                /* Receive data */
                bytesRcv = recv(clientSock, (char *) &inBuffer, MAX_MSG_LEN, 0);
                if (bytesRcv <= 0)
                {
                    cout << "Client handler: recv() failed, or the connection is closed." << endl;
                    break;
                }

                /* Checking for client request */
                if ( strncmp(inBuffer, "EXT", 3) == 0 )                         // EXT - client is disconnecting
                {
                    cout << "Client Handler: closing connection with client..." << endl;
                    break;
                }

                else if ( strncmp(inBuffer, "SET", 3) == 0 )                    // SET - client wants to store data to be transformed
                {
                    memset(&data, 0, MAX_MSG_LEN);

                    cout << "Client Handler: setting client data..." << endl;
                    ptr = strstr(inBuffer, " ")  + 1;                           // parse out the reqest code and get client data
                    strcpy(data, ptr);                                          // storing it in data buffer
                    cout << "Client: " << data << endl;;
                }

                else if ( strncmp(inBuffer, "TRN", 3) == 0 )                    // TRN - client wants to perform some transformation to data
                {
                    memset(&trans, 0, MAX_MSG_LEN);

                    cout << "Client Handler: transforming client data..." << endl;
                     ptr = strstr(inBuffer, " ")  + 1;                          // parsing client transformations from message
                    strcpy(trans, ptr);
                    cout << "Client: " << trans << endl;

                    /* Transforming data */
                    if ( transformData(trans, data) < 0)
                    {
                        cout << "Client handler: Bad client transformation " << endl;
                        close(clientSock);
                        exit(-1);
                    }

                    /* Return transformed Data */
                    bytesSnt = send(clientSock, (char *) &data, strlen(data), 0);
                    cout << "Client handler: sending " << bytesSnt << " bytes" << endl;
                    if ( bytesSnt != strlen(data) )
                    {
                        cout << "Client handler: send() failed" << endl;
                        break;
                    }

                }

            } /* End of the client receive loop  */

            cout << "Client Handler: Disconnected with a client" << endl;
            close(clientSock);
            exit(0);

        }

        else
        {
            close(clientSock);
        }

    }

    return 0;
}


int transformData(char *trans_seq, char *data)
{
    int bytesRcv, j, serv_num;

    int data_size = strlen(data);                                               // size of client data
    int trans_size = strlen(trans_seq);                                         // number of transformations
    char inMsg[MAX_MSG_LEN];                                                    // data after sending it to a single server

    int sock;                                                                   // socket to send to microservice servers
    struct sockaddr_in mcr_server;
    struct sockaddr *server;
    socklen_t len;
    int port_num;                                                               // microservice server port number


    /* DELETE LATER! */
    cout << "CLIENT DATA: " << data << endl;
    cout << "CLIENT TRANS: " << trans_seq << endl;

    /* Create UDP Socket */
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if ( sock < 0 )
    {
        cerr << "Client handler: socket() failed" << endl;
        return -1;
    }

    /* Looping through the whole transformation sequence */
    for (j = 0; j < trans_size; j++)
    {
        serv_num = *(trans_seq + j) - '0';

        /* Error checking the service numbers */
        if ( serv_num > 6 || serv_num < 0 )
        {
            cout << "Client handler: service " << serv_num << " does not exist!" << endl;
            return -1;
        }

        /* Need to check if the server is available */

        /* DELETE LATER! */
        cout << "Client handler: service number -> " << serv_num << endl;

        /* Determining the port number based on the service number */
        switch (serv_num)
        {
        case 1:                 // Identity
            port_num = 8081;
            break;
        case 2:                 // Reverse
            port_num = 8082;
            break;
        case 3:                 // Upper
            port_num = 8083;
            break;
        case 4:                 // Lower
            port_num = 8084;
            break;
        case 5:                 // Ceasar
            port_num = 8085;
            break;
        case 6:                 // Yours
            port_num = 8086;
            break;
        default:
            port_num = -1;
            break;
        }

        if ( port_num == -1 )                                                   // Case when service number does not exist
        {
            cerr << "Client handler: transformation service does not exist" << endl;
            return -1;
        }


        /* Getting server info */
        len = sizeof(mcr_server);
        memset((char *) &mcr_server, 0, len);
        memset((char *) server, 0, sizeof(server));
        mcr_server.sin_family = AF_INET;
        mcr_server.sin_port = htons(port_num);
        server = (struct sockaddr *) &mcr_server;
        len = sizeof(mcr_server);

        if (inet_pton(AF_INET, SERVER_IP, &mcr_server.sin_addr)==0)
        {
            cerr << "Client handler: inet_pton() failed";
            return -1;
        }

        /* Sending data to microservice */
        if ( sendto(sock, data, data_size, 0, server, len) ==-1 )
        {
            cout << "Client handler: sendto() failed" << endl;
            return -1;
        }

        /* Receiving the data */
        memset(&inMsg, 0, MAX_MSG_LEN);
        if ( (bytesRcv = recvfrom(sock, (char *) &inMsg, MAX_MSG_LEN, 0, server, &len)) == -1 )
        {
            cout << "Client handler: recvfrom() failed" << endl;
            return -1;
        }

        cout << "After transformation: " << inMsg << endl;

        memset(data, 0, data_size);
        strcpy(data, inMsg);

    }

    return 0;
}
