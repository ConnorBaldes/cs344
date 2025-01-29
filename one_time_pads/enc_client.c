#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) { 
  fprintf(stderr, msg); 
  exit(1); 
} 
//check to see if any of the characters in the text file are not A-Z or a space
int bad_input(char string[], int size) {
    for(int i = 0; i < size; i++) {
        if(string[i] == ' ') {
            continue;
        }   
        else if(string[i] < 'A' || string[i] > 'Z') {
            return 1;
        }
    }
    return 0;
}

//sends data from sever to client via the socket connection
void send_data(int socketFD, void* text_buffer, void* key_buffer, size_t text_length, size_t key_length, int flags) {
    ssize_t chars_sent;
    ssize_t chars_rec;
    char* ptr1 = text_buffer;
    char* ptr2 = key_buffer;
    char confirm[11];
    //set up an array that will act as a deliminator for the data being sent 
    //to the client so it can be tokened out into its different sections.
    char done[6] = "done##";
    memset(confirm, '\0', sizeof(confirm));
    char text_end[2];
    memset(text_end, '#', sizeof(text_end));
    char key_end[2];
    memset(key_end, '@', sizeof(key_end));
 
    //loop for the size of the data being sent from client to server
    while(text_length > 0) {
        //send as many characters as are in the data or as many as possible
      chars_sent = send(socketFD, ptr1, text_length, flags);
      if(chars_sent <= 0) {
          error("CLIENT: ERROR writing to socket\n");
      }
      //move pointer to newt character to be read
      //subtract length of data to be sent
      ptr1 += chars_sent;
      text_length -= chars_sent;
    }
    //send receive confirmation
    chars_sent = send(socketFD, text_end, sizeof(text_end), flags);
    //receive key
    chars_rec = recv(socketFD, confirm, sizeof(confirm), flags);
    //same as loop above but for key
    while(key_length > 0) {
      chars_sent = send(socketFD, ptr2, key_length, flags);
      if(chars_sent <= 0) {
        error("CLIENT: ERROR writing to socket\n");
      }
      ptr2 += chars_sent;
      key_length -= chars_sent;
    }
    
  chars_sent = send(socketFD, key_end, sizeof(key_end), flags);

  //chars_rec = recv(socketFD, confirm, sizeof(confirm), flags);

  //chars_sent = send(socketFD, done, sizeof(done), flags);

  
}
// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname("localhost"); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {
  //declare all necessary variables
  int socketFD, portNumber, charsWritten, charsRead, charsSent, charsRec;
  struct sockaddr_in serverAddress;
  char* text_buffer = NULL;
  size_t text_length = 0;
  char* key_buffer = NULL;
  size_t key_length = 0;
  ssize_t text_read = 0;
  ssize_t key_read = 0;
  char temp_buf[1000];
  char* ptr = NULL;
  memset(temp_buf, '\0', sizeof(temp_buf));
  int chars_sent;
  int chars_rec;
  char handshake[] = "enc_server";
  char confirm[5];
  memset(confirm, '\0', sizeof(confirm));
  // Check usage & args
  if (argc < 4) { 
    fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
    exit(1); 
  } 
 
  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket\n");
  }
  //open files that data and key will be read from
  FILE* text_file = fopen(argv[1], "r");
  FILE* key_file = fopen(argv[2], "r");
  text_read = getline(&text_buffer, &text_length, text_file);
  if(text_read == -1) {
      error("CLIENT: ERROR reading text file\n");
  }
  key_read = getline(&key_buffer, &key_length, key_file);
  if(key_read == -1) {
      error("CLIENT: ERROR reading key file\n");
  }
  //close files afte data is extracted
  fclose(text_file);
  fclose(key_file);
  // Remove the trailing \n that getline adds
  text_buffer[strcspn(text_buffer, "\n")] = '\0'; 
  text_read -= 1;
  key_buffer[strcspn(key_buffer, "\n")] = '\0';
  key_read -= 1;
  if(text_read > key_read) {
      error("CLIENT: ERROR key is smaller than text\n");
  }
  if(bad_input(text_buffer, text_read) == 1) {
      error("CLIENT: ERROR text input bad\n");
  }
  if(bad_input(key_buffer, key_read) == 1) {
      error("CLIENT: ERROR key input bad\n");
  }
   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]));

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting\n");
  }
  chars_sent = send(socketFD, handshake, sizeof(handshake), 0);
  chars_rec = recv(socketFD, confirm, sizeof(confirm),0);
  if(strcmp(confirm, "yes") != 0) {
    close(socketFD); 
    error("Tried connectiong to wrong server\n");
  }

////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Send message to server
  // Write to the server

  send_data(socketFD, text_buffer, key_buffer, text_read, key_read, 0);

  // Get return message from server
  // Clear out the buffer again for reuse
  
    memset(text_buffer, '\0', sizeof(text_buffer));
    
    while( ptr == NULL){
      
      memset(temp_buf, '\0', sizeof(temp_buf));
      charsRead = recv(socketFD, temp_buf, 999, 0); 
      if (charsRead < 0){
          error("CLIENT: ERROR reading from socket\n");
      }
      if(strstr(temp_buf, "!!") != NULL){
        continue;
      }
      ptr = strstr(temp_buf, "##");
      strcat(text_buffer, temp_buf);
      //printf("%s\n", text_buffer);
      
    }
    text_buffer[strcspn(text_buffer, "#")] = '\n';
    text_buffer[strcspn(text_buffer, "#")] = '\0';

    printf("%s", text_buffer);
    
  // Close the socket
  close(socketFD); 
  
  return 0;
}
