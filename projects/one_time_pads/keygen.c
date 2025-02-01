// this program generates a key of a given size and outputs it to stdout
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

int main(int argc,char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "KEYGEN: Must input Key size.\n");
        exit(1);
    }
    //get key size
    int key = atoi(argv[1]);
    if(key == 0) {
        fprintf(stderr, "KEYGEN: Key size invalid.\n");
        exit(1);
    }
    char* key_string;
    //create key array of given size
    key_string = (char*)malloc((key+2) * sizeof(char));
    memset(key_string, '\0', sizeof(key_string));
    srand(time(0));
    int rand_num;
    int conversion;
    for(int i = 0; i < (key); i++){
        //loop through array and fill with random characters
        rand_num = (rand() % (27));
        
        if(rand_num == 26) {
            key_string[i] = ' ';
        }
        else {
            conversion = rand_num + 65;
            
            key_string[i] = (char)conversion;

        }
    }
    //end key string will new line 
    key_string[key] = '\n';
    printf("%s",key_string);
    //free key
    free(key_string);
    return 0;
}