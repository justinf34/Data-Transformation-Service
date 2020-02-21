#include <iostream>
#include <sys/socket.h> /// socket(), connect(), send(), recv()
#include <arpa/inet.h>  /// sockaddr_in, inet_addr()
#include <stdlib.h>     /// atoi(), exit()
#include <cstring>      /// memset()
#include <unistd.h>     /// close()

#define MAX_MSG_SIZE 100

using namespace std;

int main(int argc, char *argv[])
{
    int client_sock;
    struct sockaddr_in server_addr;
    char inBuffer[MAX_MSG_SIZE];
    char outBuffer[MAX_MSG_SIZE];
    char msg[MAX_MSG_SIZE];
    int bytesRcv;
    int bytesSent;

    bool hasMsg = false;

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

    bool terminate = false;
    while (!terminate)
    {
        cout << "Actions" << endl
             << "0 - Exit" << endl
             << "1 - Set message" << endl
             << "2 - Transfomation" << endl
             << "Please enter an option: ";
        fgets(msg, MAX_MSG_SIZE, stdin);

        if (strncmp(msg, "0", 1) == 0)
        {
            cout << "client: Shutting down service" << endl;
            strcpy(outBuffer, "EXT");
            bytesSent = send(client_sock, (char *)&outBuffer, strlen(outBuffer), 0);
            if (bytesSent < 0 || bytesSent != strlen(outBuffer))
            {
                cout << "client: error in sending" << endl;
            }

            exit(1);
        }
        else if (strncmp(msg, "1", 1) == 0)
        {
            strcpy(outBuffer, "SET");
            cout << "Enter message:" << endl;
            fgets(msg, MAX_MSG_SIZE, stdin);
            strncat(outBuffer, msg, strlen(msg));

            bytesSent = send(client_sock, (char *)&outBuffer, strlen(outBuffer), 0);
            if (bytesSent < 0 || bytesSent != strlen(outBuffer))
            {
                cout << "client: error in sending" << endl;
                exit(1);
            }
            hasMsg = true;
        }
        else if (strncmp(msg, "2", 1) == 0)
        {
            strcpy(outBuffer, "TRN");
            cout << "Enter transformation sequence:" << endl;
            fgets(msg, MAX_MSG_SIZE, stdin);
            strncat(outBuffer, msg, strlen(msg));

            bytesSent = send(client_sock, (char *)&outBuffer, strlen(outBuffer), 0);
            if (bytesSent < 0 || bytesSent != strlen(outBuffer))
            {
                cout << "client: error in sending" << endl;
                exit(1);
            }
            hasMsg = true;
        }
        else
        {
            continue;
        }
    }

    return 0;
}
