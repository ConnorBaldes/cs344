#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>




#define THREADS 4 
//initialize all buffers along with a count of elements in buffer
//and the index the producer and consumer of the buffer are currently at
//also a mutex and conditional variable for each buffer
char buffer_1[50][1000];
int count_1 = 0;
int prod_line_1 = 0;
int con_line_1 = 0;
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;

char buffer_2[50][1000];
int count_2 = 0;
int prod_line_2 = 0;
int con_line_2 = 0;
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;

char buffer_3[50][1000];
int count_3 = 0;
int prod_line_3 = 0;
int con_line_3 = 0;
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;

char* get_user_input() {
    //set a temporary buffer to hold string from getline
    char buffer[1000];
    //pointer to char array to pass to getline
    char* b;
    size_t buff_size = 1000;
    memset(buffer, '\0',sizeof(buffer));
    //getline pulls all characters from stin including \n and puts them in the buffer
    getline(&b, &buff_size, stdin);
    //a pointer to the buffer is returned 
    return b;

}

/*
 Put an item in buff_1
*/
void put_buff_1(char* string){
  // Lock the mutex before putting the item in the buffer
  pthread_mutex_lock(&mutex_1);
  // Put the item in the buffer
  strcpy(buffer_1[prod_line_1], string);
  // Increment the index where the next item will be put.
  prod_line_1 = prod_line_1 + 1;
  count_1++;
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&full_1);
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_1);
}

void *get_input(void *args) {
    //continue taking input until stop is received
    while(1) {
        char* string = get_user_input();
        if(strcmp(string, "STOP\n") == 0) {
            put_buff_1(string);
            break;
        }
        //put input from stdin into first buffer
        put_buff_1(string);
        
    }
    return NULL;
}

/*
Get the next item from buffer 1
*/
char* get_buff_1(){
  // Lock the mutex before checking if the buffer has data
  pthread_mutex_lock(&mutex_1);
  while (count_1 == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_1, &mutex_1);
  char* item = buffer_1[con_line_1];
  // Increment the index from which the item will be picked up
  con_line_1 = con_line_1 + 1;
  count_1--;
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_1);
  // Return the item
  return item;
}

/*
 Put an item in buff_2
*/
void put_buff_2(char* string){
  // Lock the mutex before putting the item in the buffer
  pthread_mutex_lock(&mutex_2);
  // Put the item in the buffer
  strcpy(buffer_2[prod_line_2], string);
  // Increment the index where the next item will be put.
  prod_line_2 = prod_line_2 + 1;
  count_2++;
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&full_2);
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_2);
}
//this fuction checks each character from buffer one to see of it is the newline character and if it is 
//it replaces that character with a space
void *line_seperator(void *args) {

    char* string;
    char new_string[1000];
    int location;
    char* pch;
    while(1) {
        //gets string from buffer one
        string = get_buff_1();
        //check to see if the current string is stop and if so breaks out of the loop
        if(strcmp(string, "STOP\n") == 0) {
            //puts stop in next buffer so next thread will know to stop 
            put_buff_2(string);
            break;
        }
        //copy the string into character array
        strcpy(new_string, string);
        //check for new line character in full string
        pch = memchr(string, '\n', strlen(string));
        //if it is found go to location in char array and change char to a space
        if( pch != NULL) {
            location = pch - string;
            new_string[location] = ' ';
        }  
        //put adjusted string into buffer two
        put_buff_2(new_string);
    }
    return NULL;

}
/*
Get the next item from buffer 2
*/
char* get_buff_2(){
  // Lock the mutex before checking if the buffer has data
  pthread_mutex_lock(&mutex_2);
  while (count_2 == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_2, &mutex_2);
  char* item = buffer_2[con_line_2];
  // Increment the index from which the item will be picked up
  con_line_2 = con_line_2 + 1;
  count_2--;
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_2);
  // Return the item
  return item;
}

/*
 Put an item in buff_3
*/
void put_buff_3(char* string){
  // Lock the mutex before putting the item in the buffer
  pthread_mutex_lock(&mutex_3);
  // Put the item in the buffer
  strcpy(buffer_3[prod_line_3], string);
  // Increment the index where the next item will be put.
  prod_line_3 = prod_line_3 + 1;
  count_3++;
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&full_3);
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_3);
}

// this function takes one string at a time from buffer two and goes through it to look for the presence of the substring "++" if that is 
// found that substring is replaced with the '^' character and the size of the string is reduced by one and all characters after "++" are 
// moved back one to match the new size of the array 
// found the algorithm for finding and replacing substring here https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/
void *plus_sign(void *args) {


    char* string;
    //string being searched for 
    char plus[] = "++";
    //string that will replace it
    char carat[] = "^";
    //new resulting string after conversion
    char* result;
    int i, count = 0;
    //size of sub string being searched for and substring to replace it 
    int plus_len = strlen(plus);
    int carat_len = strlen(carat);
    //looop until stop is received 
    while(1) {
        //get next string in buffer two
        string = get_buff_2();
        if(strcmp(string, "STOP\n") == 0) {
            put_buff_3(string);
            break;
        }
        // Counting the number of times ++
        // occur in the string 
        for (i = 0; string[i] != '\0'; i++) { 
            if (strstr(&string[i], plus) == &string[i]) { 
                count++; 
  
                // Jumping to index after ++ 
                i += plus_len - 1; 
            } 
        }
        // Making new string of enough length 
        result = (char*)malloc(i + count * (carat_len - plus_len) + 1); 
  
        i = 0; 
        while (*string) { 
            // compare the substring with the result 
            if (strstr(string, plus) == string) { 
                strcpy(&result[i], carat); 
                i += carat_len; 
                string += plus_len; 
            } 
            else
                result[i++] = *string++; 
        } 
        result[i] = '\0'; 
        //put resulting string in buffer three
        put_buff_3(result);
    } 
    return NULL; 
}
/*
Get the next item from buffer 3
*/
char* get_buff_3(){
  // Lock the mutex before checking if the buffer has data
  pthread_mutex_lock(&mutex_3);
  while (count_3 == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_3, &mutex_3);
  char* item = buffer_3[con_line_3];
  // Increment the index from which the item will be picked up
  con_line_3 = con_line_3 + 1;
  count_3--;
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_3);
  // Return the item
  return item;
}
//this function takes string from buffer three and parses them into string of size 80 characters then prints those strings to 
//stdout
void *output(void *args) {


    char* string;
    char new_string[1000];
    int org_string_length;
    int string_length;
    char output_string[82];
    int output_len = 0;
    int cur_pos = 0;
    memset(output_string, '\0',sizeof(output_string));
    memset(new_string, '\0', sizeof(new_string));
    //keep going until stop is received 
    while(1) {
        //get next string from buffer three
        string = get_buff_3();
        //if next string is stop then break out of loop
        if(strcmp(string, "STOP\n") == 0) {
            break;
        }
        //copy string from buffer three into char array
        strcpy(new_string, string);
        //get length of string in char array
        string_length = strlen(new_string);
        //set variable holding the length of array that will not change throughout execution of loop
        org_string_length = string_length;
        //as long as there are still characters in the current string from buffer three keep looping
        while(string_length > 0) {
            //if the number of characters in string or number of characters left in the string is less than the number of characters
            //still allowed to be put in the output string add those characters to the end of the output string 
            if(string_length <= (80-output_len)) {
                strcat(output_string, new_string);
                output_len += string_length;
                //since all character are now in output string the input string length is set to zero
                string_length = 0;
            }
            //if there are more characters in the string remaining than the allowed amount in the output string place the number of characters
            //that are still allowed in the output string into output
            else {
                //for the number of characters output string needs to reach 80 chars put that amount of characters from buffer three into output
                for(int j = 0; j < (80-output_len); j++) {
                    output_string[j+output_len] = new_string[0];
                    //decrement buffer 3 and decrease its size by one
                    for(int w = 0; w < string_length; w++){
                        if(new_string[w+1] != '\0') {
                            new_string[w] = new_string[w+1];
                        }
                        else {
                            new_string[w] =  '\0';
                        }
                    }
                    string_length -= 1;

                }
                //output length gets the number of characters input from buffer 3
                output_len += (org_string_length-string_length);
            }
            //if we have reached character limit manually set new line and null character after 80 charactes 
            if(output_len >= 80) {
                output_string[80] = '\n';
                output_string[81] = '\0';
                //print 80 character line and flush buffer
                printf("%s", output_string);
                fflush(stdout);
                output_len = 0;
                memset(output_string, '\0',sizeof(output_string));
            }

        }
        memset(new_string, '\0', sizeof(new_string));
        
    }
    return NULL;
}

int main() {
    //set the memory of all of the buffers to null just to be safe
    for(int i = 0; i < 50; i++) {
        memset(buffer_1[i],'\0',sizeof(buffer_1[i]));
    }
    for(int i = 0; i < 50; i++) {
        memset(buffer_2[i],'\0',sizeof(buffer_2[i]));
    }
    for(int i = 0; i < 50; i++) {
        memset(buffer_3[i],'\0',sizeof(buffer_3[i]));
    }
    //initialize threads
    pthread_t input_t, line_seperator_t, plus_sign_t, output_t;

    // Create the threads
    pthread_create(&input_t, NULL, get_input, NULL);
    pthread_create(&line_seperator_t, NULL, line_seperator, NULL);
    pthread_create(&plus_sign_t, NULL, plus_sign, NULL);
    pthread_create(&output_t, NULL, output, NULL);

    // Wait for the threads to terminate
    pthread_join(input_t, NULL);
    pthread_join(line_seperator_t, NULL);
    pthread_join(plus_sign_t, NULL);
    pthread_join(output_t, NULL);

    return 0;
}