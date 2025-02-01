#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct movie {
    char* title;
    int year;
    char languages[5][21];
    float rating;
    struct movie* next;
};

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

//asks user for a specific year(int) they would like to see movies from and searches
//linked list of movies for movies released in that year and displays them
void year_release(struct movie* list, int movie_count, int year) {

    //temporary list for moving through linked list
    struct movie* temp_list = list;
    for(int i = 0; i < movie_count; i++){

        //check if entered year is equal to year in linked list node
        if(temp_list->year == year) {

            //if yes title of movie is printed 
            printf(" %s\n", temp_list->title);
            temp_list = temp_list->next;
        }
        else {
            //move to next link
            temp_list = temp_list->next;
        }

    }
    return;
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
    //and array twos size will be increased by one. If one is found and the rating of the movie in array one is
    // greater than the rating of the movie in array two then the movie value from array one will replace the 
    // movie value in array two at that spot. 
    for(int i = 0; i < movie_count; i++) {

        for(int j = 0; j < *array_two_size; j++) {

            duplicate = false;

            if(array_one[i].year == array_two[j].year) {
            
                if(array_one[i].rating > array_two[j].rating) {

                    array_two[j] = array_one[i];
                }
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

void rating(struct movie* list, int movie_count) {

    int array_size = 0;
    struct movie movie_array[movie_count];
    year_sort(list, movie_array, movie_count, &array_size);

    //prints the resulting array from year_sort
    for(int i = 0; i < array_size; i++) {

        printf(" %d %.1f %s\n", movie_array[i].year, movie_array[i].rating, movie_array[i].title); 
    }

    return;
}

void language(struct movie* list, int movie_count) {

    //temporary list for moving through linked list
    struct movie* temp_list = list;
    char language[21];
    int count = 0;
    printf(" Enter the language for which you would like to see movies: ");
    scanf("%s", language);

    for(int i = 0; i < movie_count; i++) {

        for(int j = 0; j < ((sizeof temp_list->languages) / (sizeof *temp_list->languages)); j++) {
            //check to see if the input language is equal to any language from the current movie
            if(strcmp(temp_list->languages[j], language) == 0) {

                printf(" %d %s \n", temp_list->year, temp_list->title );

                count += 1;
            }
        }
        temp_list = temp_list->next;
    }
    //if no movies contain given language message is desplayed 
    if(count < 1) {
        printf("\n\n No data about movies in %s. \n", language);
    }
    return;
}

void menu(struct movie* list, int movie_count) {

    struct movie* temp_list;
    int choice;
    int year;
    bool movie_year;
    bool go_again = true;

    //loop if user wants to run program multiple times
    while(go_again) {
        //temporary list for moving through linked list
        temp_list = list;
        printf("\n 1. Movies release in a specified year. \n 2. Highest rated movie for each year. \n 3. Movies released in a specific language. \n 4. Exit. \n \n Please enter the number corresponding to movies you would like to see or choose to exit: ");
        scanf("%d", &choice);
        printf("\n");

        if(choice < 1 || choice > 4) {

            printf("\n Enter a number between 1 and 4. \n \n ");
        }

        else {
            if(choice == 1) {
                printf(" Please enter the year you would like to see movies from: ");
                scanf("%d", &year);
                printf("\n");

                for(int i = 0; i < movie_count; i++) {
                    
                    if(temp_list->year == year) {

                        year_release(list, movie_count, year);
                        movie_year = true;
                        break;
                    }
                    else{
                        movie_year = false;
                        temp_list = temp_list->next;

                    }
                }
                if(movie_year == false) {
                    printf("\n No data for movies in the year %d. \n", year);
                }

            }
            else if(choice == 2) {

                rating(list, movie_count); 

            }
            else if(choice == 3) {

                language(list, movie_count);

            } 
            else {
                go_again = false;
            }
        }
    }
    return;
}


/*
*   Process the file provided as an argument to the program to
*   create a linked list of movie structs and print out the list.
*   Compile the program as follows:
*       gcc --std=gnu99 -o main main.c
*/

int main(int argc, char *argv[]) {

    int x = 0;
    int movie_count = 0;
    if (argc < 2)
    {
        printf("You must provide the name of the file to process\n");
        
        return 1;
    }
    struct movie *list = processFile(argv[1], &movie_count);
    
    printf("\nProcessed File %s and parsed data for %d movies. \n\n", argv[1], movie_count);


    menu(list, movie_count);
    
    return 0;
}
