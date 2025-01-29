
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define PREFIX "movies_sample"

//movie struct
struct movie {
    char* title;
    int year;
    char languages[5][21];
    float rating;
    struct movie* next;
};

//function to create a new movie link for the movie linked list
struct movie *create_movie(char *currLine) {
    struct movie *currmovie = malloc(sizeof(struct movie));
    
    // For use with strtok_r
    char *saveptr;
    char* ptr_c;
    // The first token is the title
    char *token = strtok_r(currLine, ",", &saveptr);
    currmovie->title = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currmovie->title, token);
     
    // The next token is the year 
    token = strtok_r(NULL, ",", &saveptr);
    //currmovie->year = calloc(strlen(token) + 1, sizeof(char));
    currmovie->year = atoi(token);
    
    // The next token is the languages  
    int x = 1;
    int y = 0;

    while(x) {
               
        token = strtok_r(NULL, "[];", &saveptr);

        if( token[0] == ',') {
            break;
        }
        
        //currmovie->languages[y] = calloc(strlen(token) + 1, sizeof(char));
        strcpy(currmovie->languages[y], token);

        y += 1;    
    }
    // The last token is the rating
    token = strtok_r(NULL, ",", &token);

    currmovie->rating = atof(token);
    
    // Set the next node to NULL in the newly created movie entry
    //currmovie->next = NULL;

    return currmovie;    
}
//function tasked with processing the chosen file and extracting the required info 
struct movie *processFile(char *filePath, int* movie_count)
{
    // Open the specified file for reading only
    FILE *movieFile = fopen(filePath, "r");
    
    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;
    char *token;
    
    // The head of the linked list
    struct movie *head = NULL;
    // The tail of the linked list
    struct movie *tail = NULL;
    
    // Read the file line by line
    getline(&currLine, &len, movieFile);
    
    while ((nread = getline(&currLine, &len, movieFile)) != -1)
    {
        // Get a new movie node corresponding to the current line
        struct movie *newNode = create_movie(currLine);

        // Is this the first node in the linked list?
        if (head == NULL)
        {
            // This is the first node in the linked link
            // Set the head and the tail to this node
            head = newNode;
            tail = newNode;
        }
        else
        {
            // This is not the first node.
            // Add this node to the list and advance the tail
            tail->next = newNode;
            tail = newNode;
        }
        *movie_count+= 1;
    }   
    free(currLine);
    fclose(movieFile);
      
    return head;
}

//this function sorts the link list on movies into an array that holds the movie from each unique year
// with the highest rating
void year_sort(struct movie* list, struct movie array_two[], int movie_count, int* array_two_size) {

    //temporary list for moving through linked list
    struct movie* temp_list = list;
    //array that will be filled with all movies from linked list
    struct movie array_one[movie_count];
    bool duplicate; 

    // fill array one with all movies from linked list
    for(int i = 0; i < movie_count; i++) {

        array_one[i] = *temp_list; 
        temp_list = temp_list->next;       
    }

    //put the first contents of array one into array two
    array_two[0] = array_one[0];
    //increase the size of array two
    *array_two_size = 1;
    //for each movie in array one array two will be checked for a movie with the corresponding year.
    //If one is not found the contents of that movie from array one will be added to the end of array two
    //and array twos size will be increased by one. 
    for(int i = 0; i < movie_count; i++) {

        for(int j = 0; j < *array_two_size; j++) {

            duplicate = false;

            if(array_one[i].year == array_two[j].year) {
            
                duplicate = true;
                break;
            }
        }
        if(duplicate == false) {

            array_two[*array_two_size] = array_one[i];

            *array_two_size += 1;
        }
    }
    return;

}

void movie_parse(struct movie* list, int movie_count) {

    //temporary list for moving through linked list
    struct movie* temp_list; //so we can keep track of pointer to list head
    int array_size = 0;
    char* name = "baldesc.movies.";
    int rand_num = random() % 99999; //get random number for 
    char rand_str[6]; //string for random munber
    struct movie year_array[movie_count]; //array for every unique year in movie list
    char dir_name[256];
    FILE* new_file;
    char file_name[32];

    //set all values in new arrays to '\0'
    memset(year_array, '\0', sizeof(year_array));
    memset(file_name, '\0', sizeof(file_name));
    memset(dir_name, '\0', sizeof(dir_name));
    strcpy(dir_name, name);
    sprintf(rand_str, "%d" ,rand_num);
    strcat(dir_name, rand_str);
    mkdir(dir_name, 0750);

    printf("\n New Directory: %s has been created \n", dir_name);

    //gets all unique years in list and puts them in year_array
    year_sort(list, year_array, movie_count, &array_size);

    //for each uniquie year a file with that year as its name is created within the
    //new directory its permissions are set and every movie from that year is placed
    //on its own line within the new file
    for(int i = 0; i < array_size; i++){
        temp_list = list; 
        sprintf(file_name, "./%s/%d.txt",dir_name,year_array[i].year);
        
        new_file = fopen(file_name, "w+");
        chmod(file_name, 0640);
        if(new_file == NULL) {

            printf("ya done goofed \n");
            exit(1);
        }

        for(int j = 0; j < movie_count; j++) {

            //check if entered year is equal to year in linked list node
            if(temp_list->year == year_array[i].year) {

                fprintf(new_file, "%s\n", temp_list->title);
            // ***** push to file year *****

            temp_list = temp_list->next;
            }

            else {
                //move to next link
                temp_list = temp_list->next;
            }
        }
        fclose(new_file);
    }
    
    return;
}

void menu() {

    struct dirent *aDir;  //pointer to current directory
    struct stat dirStat1;
    struct stat dirStat2;
    char file_name[256]; 
    struct movie* list; 
    int movie_count;
    int choice;
    bool go_again = true;
    int file_count = 0;

    //loop if user wants to run program multiple times
    while(go_again) {

        //check to see if user wants to process a file 
        printf("\n 1. Select file to process. \n 2. Exit the Program \n Enter a choice 1 or 2: ");
        scanf("%d", &choice);
        printf("\n");

        if(choice == 1) {
            
            while(go_again) {
            
                //opens current directory and assigns "currDir" to point to it
                DIR* currDir = opendir(".");
                movie_count = 0;
                go_again = false;
                printf(" Which file do you want to process? \n Enter 1 to pich the largest file \n Enter 2 to pick the smallest file \n Enter 3 to specify the name of a file \n Enter a choice 1 to 3: ");
                scanf("%d", &choice);
                printf("\n");

                if(choice == 1) {
                 // Go through all the files to find largest file

                    while((aDir = readdir(currDir)) != NULL){
                        
                        if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0){
                        // Get meta-data for the current file
                            stat(aDir->d_name, &dirStat1); 

                            if(file_count == 0) {
                                //set name of first file name to file_name
                                memset(file_name, '\0', sizeof(file_name));
                                strcpy(file_name, aDir->d_name);

                                stat(aDir->d_name, &dirStat2);

                            }

                            else {
                                //check to see if the size of the current file in loop is larger than the largest
                                //file found so far
                                if(dirStat1.st_size > dirStat2.st_size) {
                                
                                    memset(file_name, '\0', sizeof(file_name));
                                    strcpy(file_name, aDir->d_name);
                                          
                                }

                                stat(aDir->d_name, &dirStat2);
                            
                            }
                            file_count += 1;
                        }
                    }
                    printf("\n Now processing the chosen file named %s \n",file_name);

                    struct movie* list = processFile(file_name, &movie_count);
                    movie_parse(list, movie_count);
                }        

                else if(choice == 2) {

                    // Go through all the files

                    while((aDir = readdir(currDir)) != NULL){
                    
                        if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0){
                        // Get meta-data for the current file
                            stat(aDir->d_name, &dirStat1); 

                            if(file_count == 0) {
                            
                                memset(file_name, '\0', sizeof(file_name));
                                strcpy(file_name, aDir->d_name);

                                stat(aDir->d_name, &dirStat2);


                            }

                            else {
                                if(dirStat1.st_size < dirStat2.st_size) {
                                
                                    memset(file_name, '\0', sizeof(file_name));
                                    strcpy(file_name, aDir->d_name);          
                                }

                                stat(aDir->d_name, &dirStat2);
                            
                            }
                            file_count += 1;
                        }
                    }
                    printf("\n Now processing the chosen file named %s \n",file_name);

                    struct movie* list = processFile(file_name, &movie_count);
                    movie_parse(list, movie_count);
                }
            
                else if(choice == 3) {
                    memset(file_name, '\0', sizeof(file_name));
                    printf(" Please enter the name of the file you would like to process: ");
                    scanf("%s", &file_name);
                    
                    // Go through all the entries
                    while((aDir = readdir(currDir)) != NULL){
                        
                        if(strncmp(file_name, aDir->d_name, strlen(file_name)) == 0){
                        // Get meta-data for the current entry  

                            go_again = false;
                            printf("\n Now processing the chosen file named %s \n",file_name);
                            struct movie* list = processFile(file_name, &movie_count);
                            movie_parse(list, movie_count);

                            break;
    
                        }
                        else {

                            go_again = true;
                        }
                    }
                    if(go_again == true) {
                        printf(" The file %s was not found. Try again \n\n", file_name);
                    }

                }

            
            
                else {
                    printf("\n Enter a number between 1 and 3. \n \n ");
                }
                // Close the directory
                closedir(currDir);
            }
            
        }

        else if(choice == 2) {

            break;          
            
        }
        
        else {

            printf("\n Enter a number between 1 and 2. \n \n ");
        }
        go_again = true;
        
         

    }

    return;
}


/*
*   Process the file provided as an argument to the program to
*   create a linked list of movie structs and print out the list.
*   Compile the program as follows:
*       gcc --std=gnu99 -o main main.c
*/

int main(void) {
     
    srand(time(NULL));

    menu();


 
    return 0;
}
