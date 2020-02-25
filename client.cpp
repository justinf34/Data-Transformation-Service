#include <iostream>
#include <sys/socket.h> /// socket(), connect(), send(), recv()
#include <arpa/inet.h>  /// sockaddr_in, inet_addr()
#include <stdlib.h>     /// atoi(), exit()
#include <cstring>      /// memset()
#include <unistd.h>     /// close()

#define MAX_MSG_SIZE 128

using namespace std;


int sendData(int sock, char *buffer, int msg_size);


int main(int argc, char *argv[])
{
    int client_sock;
    struct sockaddr_in server_addr;
    char inBuffer[MAX_MSG_SIZE];
    char outBuffer[MAX_MSG_SIZE];
    char input[MAX_MSG_SIZE];
    unsigned int size;
    int bytesRcv;
    int i;

    bool hasMsg = false;                        /// Flag that makes sure that the client has data to transform

    /// Check for argument errors
    if (argc != 3)
    {
        cout << "Usage: " << argv[0] << "<Server IP> <Server Port>" << endl;
        exit(1);
    }

    ///Create TCP socket
    client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_sock < 0)
    {
        cout << "client: socket() failed" << endl;
        exit(1);
    }


    /// Free up the port
    int yes = 1;
    if (setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
        cout << "client: setsockopt() failed" << endl;
        exit(1);
    }

    /// Initialize the master server information
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    /// Connecting to master server info
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        cout << "client: connect() failed" << endl;
        exit(1);
    }

    cout << "Here" << endl;

    /// Run until client exits
    while ( 1 )
    {
        memset(&inBuffer, 0, sizeof(inBuffer));
        memset(&outBuffer, 0, sizeof(outBuffer));

        cout << "Actions" << endl
             << "0 - Exit" << endl
             << "1 - Set data" << endl
             << "2 - Transform data" << endl
             << "3 - Turn on a service" << endl
             << "4 - Turn off a service" << endl
             << "Please enter an option: ";
        fgets(input, MAX_MSG_SIZE, stdin);

        if ( strncmp(input, "0", 1) == 0 )
        {
            cout << "Shutting down connection with master server..." << endl;
            strcpy(outBuffer, "EXT");
            if ( sendData(client_sock, (char *)&outBuffer, strlen(outBuffer)) == -1)
            {
                cout << "Could not disconnect from the server" << endl;
                continue;
            }
            cout << "Goodvbye!" << endl;
            close(client_sock);
            exit(0);
        }

        else if ( strncmp(input, "1", 1) == 0 )
        {
            strcpy(outBuffer, "SET");
            memset(&input, 0, sizeof(input));

            cout << "Enter your data (Please less than 120 chars):" << endl;
            fgets(input, MAX_MSG_SIZE - 3, stdin);
            size = strlen(input);
            if ( sendData(client_sock, (char *)&outBuffer, size) == -1 )
            {
                cout << "Could not send data to server" << endl;
                continue;
            } 

            hasMsg = true;       

        }

        else if ( strncmp(input, "2", 1) == 0 )
        {
            if ( !hasMsg )
            {
                cout << "Need a data to transform" << endl;
                continue; 
            }

            bool validInput = true;
            strcpy(outBuffer, "TRN");
            memset(&input, 0, sizeof(input));

            cout << "Enter your transformation sequence (1-6):" << endl;
            fgets(input, MAX_MSG_SIZE - 3, stdin);
            size = strlen(input);

            /// Checking for non-numeric transformation
            for (i = 0; i < size; i++)
            {
                if( isalpha(outBuffer[i]) ) 
                {
                    cout << "Only input integers from 1 to 6" << endl;
                    validInput = false;
                    break;
                }
            }

            if ( !validInput )
            {
                continue;
            }

            if ( sendData(client_sock, (char *)&outBuffer, size) == -1 )
            {
                cout << "Could not send data to server" << endl;
                continue;
            }

            /// Receive transformed data

            bytesRcv = recv(client_sock, (char *) &inBuffer, MAX_MSG_SIZE, 0);

            if ( bytesRcv <= 0)
            {
                cout << "client recv() failed, or the connection is closed" << endl;
            }

            cout << "Transformed data: " << endl << inBuffer << endl;

        }

        else
        {
            cout << "Unknown command" << endl;
            continue;
        }


    }

}



/**
 * 
 * 
 * 
 * 
 * */
int sendData(int sock, char *buffer, int msg_size) {
    int bytesSent = send(sock, buffer, msg_size, 0);
    if (bytesSent < 0 || bytesSent != strlen(buffer))
    {
        cout << "client: error in sending" << endl;
        return -1;
    }
    return 0;
}
