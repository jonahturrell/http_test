#include "http_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT "8080"
#define MAX_QUEUED 10
#define MAX_REQUEST_SIZE 1024

int main(void) {

  //  Initialize Winsock if on Windows
  #if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)) {
      fprintf(stderr, "Winsock failed to initialize.\n");
      return 1;
    }
  #endif

  //  Configure the local address
  printf("Configuring local address...\n");
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  // Store the local address in a structure
  struct addrinfo *bind_address;
  int addrinfo_return = getaddrinfo(0, PORT, &hints, &bind_address);

  if (addrinfo_return != 0) { // Make sure a valid address is given
    fprintf(stderr, "getaddrinfo() failed; %s\n", gai_strerror(addrinfo_return));
    return EXIT_FAILURE;
  }

  //  Create the socket
  printf("Creating socket...\n");
  SOCKET socket_listen; 
  socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

  if (!ISVALIDSOCKET(socket_listen)) { // Check that the socket was successfully created
    fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
    return EXIT_FAILURE;
  }

  //  Bind socket to the address
  printf("Binding socket to local address...\n");
  if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) { //  Make sure the socket is bound properly
    fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
    return EXIT_FAILURE;
  }
  //  Free the bind_address memory
  freeaddrinfo(bind_address);

  //  Listen for a connection
  printf("Listening for connections...\n");
  if (listen(socket_listen, MAX_QUEUED) < 0) { // Make sure listen() works properly
    fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
    return EXIT_FAILURE;
  }

  // Wait for a connection
  printf("Waiting for a connection...\n");

  struct sockaddr_storage client_address;  // Set up stucture for client address information
  socklen_t client_len = sizeof(client_address);

  SOCKET socket_client = accept(socket_listen, (struct sockaddr*) &client_address, &client_len);

  if (!ISVALIDSOCKET(socket_client)) {
  fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
  return EXIT_FAILURE;
  }

  printf("Client is connected...\n");
  char address_buffer[512];
  int getnameinfo_return = getnameinfo((struct sockaddr*) &client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);

  if (getnameinfo_return != 0) { // Check that the client address information is properly returned
    printf("getnameinfo() failed. %s\n", gai_strerror(getnameinfo_return));
  }

  printf("Client IP: %s\n", address_buffer);

  //  Attempt to read the client request
  printf("Reading client request...\n");
  char request[MAX_REQUEST_SIZE];
  int bytes_received = recv(socket_client, request, MAX_REQUEST_SIZE, 0);

  if (bytes_received < 0) { //  Check that bytes are received properly
    printf("recv() failed. (%d)\n", GETSOCKETERRNO());
  } else {
    printf("Received %d bytes\n", bytes_received);
    printf("Request contents:\n%.*s\n", bytes_received, request);
  }

  //  Attempt to send a response
  printf("Sending response...\n");

  const char *response =
    "HTTP/1.1 CONNECTION OK\r\n"
    "Connection: close\r\n"
    "Content-Type: text/plain\r\n\r\n"
    "<html><body>This is a webpage</body></html>";

  int bytes_sent = send(socket_client, response, strlen(response), 0);
  if (bytes_sent < 0) {
    printf("send() failed. (%d)\n", GETSOCKETERRNO());
  } else {
    printf("Sent %d of %lu bytes\n", bytes_sent, strlen(response));
  }

  // Close the connection
  printf("Closing connection...\n");
  CLOSESOCKET(socket_client);

  printf("Closing listening socket...\n");
  CLOSESOCKET(socket_listen);

  #if defined(_WIN32)
   WSACleanup();
  #endif

  printf("Finished\n");
 
  return EXIT_SUCCESS;
}
