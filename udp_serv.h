#include <iostream>       // cout
#include <sys/socket.h>   // socket(), bind(), sendto(), and recv
#include <arpa/inet.h>    // for sockaddr_in, inet_addr()
#include <stdlib.h>       // for atoi(), exit()
#include <cstring>        // for memset()
#include <unistd.h>       // for close()

using namespace std;

#define MAX_MSG_LEN 128       /// Max size that the server can recieve

/**
 *  Creates and initializes a UDP socket for a server
 * 
 * @param sock a the UDP socket number
 * @param port_num the number of the port which `sock` will be binded to 
 * @param serverAddr a pointer to a struct containing server address
 * @return an error code which indicates if the initialization of socket is successful
 */
int init_servSock(int &sock, int port_num, sockaddr_in *serverAddr)
{

  /* Create UDP socket */
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if ( sock < 0 )
  {
    cout << "socket() failed" << endl;
    return -1;
  }

  /* Free up the port */
  int opt = 1;
  if ( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0)
  {
    cout << "setsockopt() failed" << endl;
    return -1;
  }


  /* Initialize Server Info */
  memset((char *)serverAddr, 0, sizeof(sockaddr_in));
  serverAddr->sin_family = AF_INET;
  serverAddr->sin_port = htons(port_num);
  serverAddr->sin_addr.s_addr = htonl(INADDR_ANY);

  /* Bind to local address */
  if ( bind(sock, (sockaddr *)serverAddr , sizeof(sockaddr_in)) < 0 )

  {
    cout << "bind() failed" << endl;
  }

  return 0;
}
