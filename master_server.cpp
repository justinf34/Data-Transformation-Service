#include <iostream> // cout
#include <cstring>  // string, memset()
#include <stdlib.h> // exit()
#include <stdio.h>  // memset()
#include <unistd.h> // close

/* Network relate */
#include <sys/socket.h> // socket(), connect(), send(), and recv()
#include <arpa/inet.h>  // sockaddr_in, inet_addr()
#include <netdb.h>
#include <netinet/in.h>

/* fork() */
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

/* Constants */
#define MAX_MSG_LEN 128
#define SERVER_IP "127.0.0.1"

/***
 *  This signal handler to gracefully exit
 *  by losing all the sockets
 **/
void sig_handler(int sig);

int transformData(char *trans_seq, char *data);

int clientSock;

struct service {
    bool available;
    int sock;
    struct sockaddr *server;
    socklen_t len;
};

/* Main driver for master_server.cpp */
int main(int argc, char *argv[])
{
    int pid;
    struct sockaddr_in server;
    int listener_sock;

    // Need to set up signal handler to catch some weird termination conditions.

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
        cerr << "master_server: socket() failed" << endl;
        exit(1);
    }

    /* Free up the port before binding */
    int yes = 1;
    if (setsockopt(listener_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)))
    {
        cerr << "master_server: setsockopt () failed" << endl;
        exit(1);
    }

    /* Binding to port */
    if ( bind(listener_sock, (struct sockaddr *) &server, sizeof(server)) < 0 )
    {
        cerr << "master_server: bind() failed" << endl;
        exit(1);
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
            char *ptr;
            char data[MAX_MSG_LEN];
            char trans[MAX_MSG_LEN];
            char inBuffer[MAX_MSG_LEN];
            char outBuffer[MAX_MSG_LEN];

            /* Keep running until the user wants to terminate connection */
            while ( 1 )
            {
              memset(&inBuffer, 0, MAX_MSG_LEN);
              memset(&outBuffer, 0, MAX_MSG_LEN);

                bytesRcv = recv(clientSock, (char *) &inBuffer, MAX_MSG_LEN, 0);
                if (bytesRcv <= 0)
                {
                    cout << "Client handler: recv() failed, or the connection is closed." << endl;
                    break;
                }

                if ( strncmp(inBuffer, "EXT", 3) == 0 )
                {
                    cout << "Client Handler: closing connection with client..." << endl;
                    break;
                }

                else if ( strncmp(inBuffer, "SET", 3) == 0 )
                {
                    memset(&data, 0, MAX_MSG_LEN);

                    cout << "Client Handler: setting client data..." << endl;
                    ptr = strstr(inBuffer, " ")  + 1;
                    strcpy(data, ptr);
                    cout << "Client: " << data;
                }

                else if ( strncmp(inBuffer, "TRN", 3) == 0 )
                {
                    memset(&trans, 0, MAX_MSG_LEN);

                    cout << "Client Handler: transforming client data..." << endl;
                     ptr = strstr(inBuffer, " ")  + 1;
                    strcpy(trans, ptr);
                    cout << "Client: " << trans;

                    /* Transform data */
                    if ( transformData(trans, data) < 0)
                    {
                        cout << "Client handler: Bad client transformation " << endl;
                        close(clientSock);
                        exit(-1);
                    }

                    /* Return transformed Data */
                    bytesSnt = send(clientSock, (char *) &data, strlen(data), 0);
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
    int bytesRcv, i, serv_num;

    int data_size = strlen(data);
    int trans_size = strlen(trans_seq);
    char inMsg[MAX_MSG_LEN];

    int sock;
    struct sockaddr_in mcr_server;
    struct sockaddr *server;
    socklen_t len;
    int port_num;


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
    for (i = 0; i < trans_size - 1; i++)
    {
        serv_num = *(trans_seq + i) - '0';

        /* Error checking the service numbers */
        if ( serv_num > 6 || serv_num < 0 )
        {
            cout << "Client handler: service " << serv_num << " does not exist!" << endl;
            return -1;
        }

        /* Need to check if the server is available */

        /* DELETE LATER! */
        cout << "Clien handler: service number -> " << serv_num << endl;

        /* Determining the port number */
        switch (serv_num)
        {
        case 1:
            port_num = 8081;
            break;
        case 2:
            port_num = 8082;
            break;
        case 3:
            port_num = 8083;
            break;
        case 4:
            port_num = 8084;
            break;
        case 5:
            port_num = 8085;
        case 6:
            port_num = 8086;
            break;

        default:
            port_num = -1;
            break;
        }

        if ( port_num == -1 )
        {
            cerr << "Client handler: transformation service does not exist" << endl;
            return -1;
        }

        // /* Getting server info */
        // memset((char *) &mcr_server, 0, len);
        // memset((char *) server, 0, sizeof(server));
        // mcr_server.sin_family = AF_INET;
        // mcr_server.sin_port = htons(port_num);
        // server = (struct sockaddr *) &mcr_server;
        // len = sizeof(mcr_server);
        //
        // if (inet_pton(AF_INET, SERVER_IP, &mcr_server.sin_addr)==0)
        // {
        //     cerr << "Client handler: inet_pton() failed";
        //     return -1;
        // }
        //
        //
        // /* Sending data to microservice */
        // if ( sendto(sock, data_t, data_size, 0, server, len) ==-1 )
        // {
        //     cout << "Client handler: sendto() failed" << endl;
        //     return -3;
        // }
        //
        // /* Receiving the data */
        // memset(inMsg, 0, MAX_MSG_LEN);
        // if ( (bytesRcv = recvfrom(sock, (char *) &inMsg, MAX_MSG_LEN, 0, server, &len)) == -1 )
        // {
        //     cout << "Client handler: recvfrom() failed" << endl;
        //     return -3;
        // }
        //
        // memset(data_t, 0, data_size);
        // strcpy(data_t, inMsg);
    }

    /* Getting server info */
    // memset((char *) &mcr_server, 0, len);
    // memset((char *) server, 0, sizeof(server));
    port_num = 8085;                                      /// DELETE LATER!
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

    return 0;

}









int microserv_connect(struct service services[], int serv_num, int port_num)
{
    struct sockaddr_in mcr_server;
    struct sockaddr *server;
    socklen_t len = sizeof(mcr_server);
    int sock;

    /* Creating a UDP socket */

    if ( (sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 )
    {
        cout << "Client handler: socket() failed" << endl;
        return -1;
    }

    /* Initialize server info */
    memset((char *) &mcr_server, 0, sizeof(mcr_server));
    mcr_server.sin_family = AF_INET;
    mcr_server.sin_port = htons(port_num);
    server = (struct sockaddr *) &mcr_server;

    /* Store server info */
    services[serv_num - 1].sock = sock;
    services[serv_num - 1].server = server;
    services[serv_num - 1].len = len;

    return 0;

}


// void init_microServ(int serv_num, int port_num)
// {
//     pid_t pid;
//     char *arg[2];
//     switch (serv_num) {
//         case 1:
//             arg = {"./UD1", port_num};
//             break;
//         case 2:
//             arg = {"./UD2", port_num};
//             break;
//         case 3:
//             arg = {"./UD3", port_num};
//             break;
//         case 4:
//             arg = {"./UD4", port_num};
//             break;
//         case 5:
//             args = {"./UD5", port_num};
//             break;

//     }

//     pid = fork();
//     if (pid < 0)
//     {
//         cout << "Client handler: fork() failed" << endl;
//         exit(-1);
//     }

//     if (pid == 0)
//     {
//         exec(arg[0], arg);

//         /* This should never print */
//     }

//     else
//     {

//     }
// }
