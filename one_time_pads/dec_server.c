#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
// Error function used for reporting issues

struct data {
  char text_buffer[100000];
  char key_buffer[100000];
  char temp_text[1000];
  char temp_key[1000];
  char* ptr1;
  char* ptr2;
  int connectionSocket, charsRead;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo;
};
//this struct is used to pass arguments into a new process 
void setup_data(struct data* new, int connectionSocket_s, struct sockaddr_in clientAddress_s, struct sockaddr_in serverAddress_s, socklen_t sizeOfClientInfo_s) {
  memset(new->temp_text, '\0', sizeof(new->temp_text));
  memset(new->temp_key, '\0', sizeof(new->temp_key));
  memset(new->text_buffer, '\0', sizeof(new->text_buffer));
  memset(new->key_buffer, '\0', sizeof(new->key_buffer));
  new->ptr1 = NULL;
  new->ptr2 = NULL;
  new->connectionSocket = connectionSocket_s;
  new->charsRead = 0;
  new->serverAddress = serverAddress_s;
  new->clientAddress = clientAddress_s;
  new->sizeOfClientInfo = sizeOfClientInfo_s;
  
}
//if there is an error print the error message and exit with a value of 1
void error(const char *msg) {
  fprintf(stderr,msg);
  exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

//sends data from sever to client via the socket connection
void send_data(int socketFD, void* buffer, size_t length, int flags) {
    ssize_t chars_sent;
    char* ptr = buffer;
    //set up an array that will act as a deliminator for the data being sent 
    //to the client so it can be tokened out into its different sections.
    char end[3];
    memset(end, '\0', sizeof(end));
    end[0] = '#';
    end[1] = '#';
    //loop for the size of the data being sent from server to client
    while(length > 0) {
        //send as many characters as are in the data or as many as possible
        chars_sent = send(socketFD, ptr, length, flags);
        if(chars_sent <= 0) {
            error("SERVER: ERROR writing to socket");
        }
        //move pointer to newt character to be read
        //subtract length of data to be sent
        ptr += chars_sent;
        length -= chars_sent;
    }
    //send deliminator
    chars_sent = send(socketFD, end, sizeof(end), flags);
}
//This function takes the data from the client and decrypts it using the random key from keygen.c
void decrypt(struct data* arg) {
  int t;
  int x;
  int k;
  int length = strlen(arg->text_buffer);
  for(int i = 0; i < length; i++) {
    //these if statements check if the character given was a space in which case we assign it the number
    //26 for the ecryption
    if(arg->text_buffer[i] == 32 || arg->key_buffer[i] == 32) {
        
      if(arg->key_buffer[i] == 32 && arg->text_buffer[i] != 32) {
        k = 26;
        t = (((int)arg->text_buffer[i])-65);
      }
      else if(arg->text_buffer[i] == 32 && arg->key_buffer[i] != 32) {
        t = 26;
        k = (((int)arg->key_buffer[i])-65);
      }
      else {
          k = 26;
          t = 26;
      }
    }
    else {
      //set rest of characters to their number in the alphabet 0-25
      t = (((int)arg->text_buffer[i])-65);
      k = (((int)arg->key_buffer[i])-65);
    }
  
    //subtracts them
    t = (t - k);  
    //check for negative number and add 27 to it
    if(t < 0) {
        t = (27-abs(t));
    }
    t = (t % 27);
    //add back to get ascii char value
    if(t == 26) {
      arg->text_buffer[i] = ' ';
    }
    else{
        t += 65;
      arg->text_buffer[i] = (char)(t);
    }
  }
}
//this fuction is passed to a thread and is the function that each connection thread will
//execute 
void* thread_action(void* arg_s) {
    struct data* arg = (struct data*)arg_s;
    char response[] = "received";
    char confirm[10];
    int count = 0;
    memset(confirm, '\0', sizeof(confirm));
    int chars_sent;
    // Get the message from the client and display it
    memset(arg->text_buffer, '\0', sizeof(arg->text_buffer));
    memset(arg->key_buffer, '\0', sizeof(arg->key_buffer));
    
    while(arg->ptr1 == NULL || arg->ptr2 == NULL ){
      

      if(arg->ptr1 == NULL) {
        memset(arg->temp_text, '\0', sizeof(arg->temp_text));
        arg->charsRead = recv(arg->connectionSocket, arg->temp_text, 999, 0); 
        if (arg->charsRead < 0){
          error("SERVER: ERROR reading from socket\n");
        }
        arg->ptr1 = strstr(arg->temp_text, "##");
        strcat(arg->text_buffer, arg->temp_text);
        
      }
      else {
        memset(arg->temp_key, '\0', sizeof(arg->temp_key));
        arg->charsRead = recv(arg->connectionSocket, arg->temp_key, 999, 0); 
        if (arg->charsRead < 0){
          error("SERVER: ERROR reading from socket\n");
        }
        arg->ptr2 = strstr(arg->temp_key, "@@");
        strcat(arg->key_buffer, arg->temp_key);  
      }  
      if(arg->ptr1 != NULL) {
        if(count == 0) {
          chars_sent = send(arg->connectionSocket, response, sizeof(response), 0);
          count = 1;
        }
        
      }
  

    }
    //chars_sent = send(arg->connectionSocket, response, sizeof(response), 0);
    //arg->charsRead = recv(arg->connectionSocket, confirm, sizeof(confirm), 0);
    arg->text_buffer[strcspn(arg->text_buffer, "#")] = '\0';
    arg->text_buffer[strcspn(arg->text_buffer, "#")] = '\0';
    arg->key_buffer[strcspn(arg->key_buffer, "@")] = '\0';
    arg->key_buffer[strcspn(arg->key_buffer, "@")] = '\0';


    //printf("SERVER: I received this text from the client: \"%s\"\nSERVER: I recieved this key from the client: \"%s\"\n", arg->text_buffer, arg->key_buffer);
    // Send a Success message back to the client

    decrypt(arg);
    
    arg->text_buffer[strcspn(arg->text_buffer, "\0")] = '#';
    arg->text_buffer[strcspn(arg->text_buffer, "\0")] = '#';
    //printf("%s",arg->text_buffer);
    send_data(arg->connectionSocket, arg->text_buffer, sizeof(arg->text_buffer), 0);



    // Close the connection socket for this client
    
    close(arg->connectionSocket); 
  

}

int main(int argc, char *argv[]){
  int connectionSocket, charsRead;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);
  struct data thread_data;
  int chars_sent;
  int chars_rec;
  char confirm[25];
  char yes[] = "yes";
  char no[] = "no";
  memset(confirm, '\0', sizeof(confirm));
  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket\n");
  }
  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));
  // Associate the socket to the port
  if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
    error("ERROR on binding\n");
  }
  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5); 
  // Accept a connection, blocking if one is not available until one connects
  while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 

    if (connectionSocket < 0){
      error("ERROR on accept\n");
    }
    chars_rec = recv(connectionSocket, confirm, sizeof(confirm), 0);
    if(strcmp(confirm, "dec_server") ==0) {
        chars_sent = send(connectionSocket, yes, sizeof(yes), 0);
    }
    else{
        chars_sent = send(connectionSocket, no, sizeof(no), 0);
        close(connectionSocket);
        continue;
    }

    //printf("SERVER: Connected to client running at host %d port %d\n", ntohs(clientAddress.sin_addr.s_addr), ntohs(clientAddress.sin_port));
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    pthread_t new_thread;

    setup_data(&thread_data, connectionSocket, clientAddress, serverAddress, sizeOfClientInfo);
 
    pthread_create(&new_thread, NULL, thread_action, (void*)&thread_data);

    pthread_join(new_thread,NULL);

  }
  // Close the listening socket
  close(listenSocket); 
  return 0;
}
