#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

int err_n_die(char *message);

int main(void) {
  
  // Load a pointer to the index.html file
  FILE *html_data;
  html_data = fopen("index.html", "r");

  //Make sure index.html is proper
  if (html_data == NULL)
    err_n_die("Issue opening index.html");

  // Create a string and store index.html contents into it
  char response_data[1024];
  fgets(response_data, 1024, html_data);

  // Create the HTTP response
  char http_header[2048] = "HTTP/1.1 200 OK\r\n\n";

  // Concatenate the HTTP response and response data
  strcat(http_header, response_data);

  // Create a socket
  int server_socket;
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  
  // Define the address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(8002);
  server_address.sin_addr.s_addr = INADDR_ANY;

  // Bind the socket to the port
  bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));

  //Listen for connections
  listen(server_socket, 5);

  //Make a variable for the client socket
  int client_socket;

  //Continually accept connections and send the HTTP data
  while (1) {
    client_socket = accept(server_socket, NULL, NULL);
    printf("Got connection\n");
    send(client_socket, http_header, sizeof(http_header), 0);
    close(client_socket);
  }
  
  return 0;
}

int err_n_die(char *message) {
  printf("ERROR: %s\n", message);
  printf("Error code %d: %s\n", errno, strerror(errno));
  printf("Exiting...\n");
  exit(EXIT_FAILURE);
}
