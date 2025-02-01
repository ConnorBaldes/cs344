#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

//these variable are for when a signal handler is called
// they can be adjusted in the signal handler then viewed
// during normal program execution.
volatile sig_atomic_t s_flag = 0;
volatile sig_atomic_t cur_mode = 0;
// struct to hold value for each token 
struct token {
    char* value; 
};

//struct to hold values gathered from command line
struct command_line {
    char input[2049];
    struct token tokens[512]; 
    int tok_num; 
    int input_size;
};

//struct to hold important shell values
struct shell_values {
    pid_t processes[100];
    int num_processes;
    int status;
    int term_ext;
    int forg_mode;

};

// this handler was made for handling ^C singal but
// i do not end up using it in my program
void handle_SIGINT_parent(int signo){

    sleep(.5);
    char* message = " Terminate by signal 2 \n";
    write(STDOUT_FILENO, message, 24);
    
}
//this signal handler is used when the ^Z signal is caught
// it prints a message based on if the program is currently in
// foreground only mode or not 
void handle_SIGTSTP(int signo){
 
    int on_v = 50;
    int off_v = 49;
    char* t_on = "\nEntering foreground-only mode (& is now ignored)\n";
    char* t_off = "\nExiting foreground-only mode (& is now enabled)\n";

    if(cur_mode == 0) {

        write(STDOUT_FILENO, t_on, on_v);
        cur_mode = 1;
    }
    else {
        write(STDOUT_FILENO, t_off, off_v);
        cur_mode = 0;
    }
    //setting the flag so the program knows to switch either in or out of foreground only mode
    //after completion of current proccess. 
    s_flag = 1;
    
}

//this function is used to set the values in the shell values struct
void set_shell_values(struct shell_values* shell) {

    //set array that holds proccesses to zero
    for(int i = 0; i < 100; i++) {
        shell->processes[i] = 0;
    }

    shell->num_processes = 0;
    shell->status = 0;
    shell->term_ext = 0;
    shell->forg_mode = 0;

}

// this function flushes the values gathered and stored in the command lin structure
void flush_command(struct command_line* command) {

    memset(command->input, '\0', sizeof(command->input));

    for(int i = 0; i < 512; i++) {
        command->tokens[i].value = NULL;
    }
}

//this function is where the values from the command line are parsed and stored into 
//the command line structure
int set_command(struct command_line* command, char* buffer) {

    int input_length;
    input_length = strlen(buffer);
    char* pch = NULL;
    int pid;
    //check to make sure that the the user entered some text and if not return
    if(strcmp(buffer, "\n") == 0) {
        return 1;
    }

    // make sure the user did not input more than the allowed 2048 characters. 
    if(input_length-1 > 2049) {

      printf("Exceeded Maximum Character limit. ");
      fflush(stdout);
      return 1;
    }

    // copy the contents of the buffer to the input variable in the command line structure
    strcpy(command->input, buffer);
    
    //since getline is used to receive information from the user the last character in the 
    // string is a new line character here that is replaced with the null character
    if(command->input[input_length-1] == '\n') { 
        command->input[input_length-1] = '\0';
    }

    //search for any instances of $$ in user input
    pch = strstr(command->input, "$$");
    // if there are instances of $$ this loop will find them and replace them with the smallsh's
    // pid it will also adjust the characters around $$ so that they are not over written
    //(I am actually very proud of this loop it took forever to figure out)
    while(pch != NULL) {
        pid = getpid();
        char cpid[sizeof(pid)];
        snprintf(cpid, sizeof(pid), "%d", pid);
        memmove((pch+sizeof(cpid)-1), (pch+2), (sizeof(command->input)-sizeof(pch)));
        memmove(pch, cpid, sizeof(cpid)-1);
        //after moving other character pid is placed where $$ was
        pch = strstr(command->input, "$$");
    }
    command->input_size = strlen(command->input);
    // I recheck the input size because the pid could be larger that $$ and in extreme scenarios
    // that coud cause the buffer to execeed its allowed size.
    if(command->input_size > 2049) {

        printf("Exceeded Maximum Character limit. ");
        fflush(stdout);
        return 1;
    }
    //at this point I token out all of the commands seperated by spaces 
    // in the user input. each token is placed in the command->tokens array
    // I also check here to make sure that the user did not input more than 
    // the allowed 512 arguments.
    command->tokens[0].value = strtok(command->input, " ");
    command->tok_num = 1;

    while(command->tokens[command->tok_num-1].value != NULL ){

        command->tokens[command->tok_num].value = strtok(NULL, " ");
        command->tok_num += 1;
        if(command->tok_num > 512) {

            printf("Exceeded Maximum Number of Arguments.");
            fflush(stdout);
            return 1; 
        }

    }
    
    return 0;

}
//this function checks to see if the user input a built in command and if so runs that command
int b_in_func(struct command_line* command, struct shell_values* shell) {

    //if the user entered exit the shell kills all running proccesses and exits normally
    if(strcmp(command->tokens[0].value, "exit") == 0) {

        for(int i = 0; i < shell->num_processes; i++) {
            kill(shell->processes[i], 9);
        }
        exit(0);
    }

    // if the user entered cd without a path than the directory will be set to the home directory
    // if the user did enter a path the directory is changed to the directory stated in the path. 
    else if(strcmp(command->tokens[0].value, "cd") == 0) {

        if(command->tokens[1].value != NULL) {
            chdir(command->tokens[1].value);
        }

        else {
            chdir(getenv("HOME"));
        }

        return 1;
    }

    //status either prints out the last exit value or signal call
    else if(strcmp(command->tokens[0].value, "status") == 0) {
        // term_ext is a shell variable that is used to know whether the the other 
        // shell variable status is a signal value(1) or exit value(0).
        if(shell->term_ext == 0) {
            printf("exit value: %d\n", shell->status);
        }
        else {
            printf("Process terminated with signal: %d\n", shell->status);
        }

        return 1;
    }
    // since comments and blank spaces are ignored by the shell I decided to deal with them here
    else if(strcmp(command->tokens[0].value, "#") == 0 || strcmp(command->tokens[0].value, "\0") == 0 ) {
        
        return 1;
    }
    
    return 0;
}

//this function is designed to check to see if the user elected to redirect standard input
// or output the function scans the tokens looking for the < or > arguments if they are found 
// the function returns 1 indication a new path has been set if they are 0 is returned saying that 
// no path was set
int check_new_path(char** args, struct command_line* command) {
    
    for(int i = 0; i < command->tok_num-1; i++) {
        
        if((strcmp(args[i],">") == 0) || (strcmp(args[i],"<") == 0)) {
            
            return 1;
        }
    }
    return 0;
}

/*
The following four fuctions do essentially the same thing, all are designed to look for
either < or > in the command tokes and if one is found either the array position of the previous
token is returned or the array position of the next position is returned each indicating either 
the target or source value of a redirection
*/

int target_ar(char** args, struct command_line* command) {

    for(int i = 0; i < command->tok_num-1; i++) {
        
        if(strcmp(args[i],">") == 0) {
        
            return (i+1);
        }
    }
    return -1;
}
int target_al(char** args, struct command_line* command) {

    for(int i = 0; i < command->tok_num-1; i++) {

        if(strcmp(args[i],"<") == 0) {
            return (i-1);
        }
    }
    return -1;
}
int source_ar(char** args, struct command_line* command) {

    for(int i = 0; i < command->tok_num-1; i++) {

        if(strcmp(args[i],">") == 0) {
            return (i-1);
        }
    }
    return -1;
}
int source_al(char** args, struct command_line* command) {

    for(int i = 0; i < command->tok_num-1; i++) {

        if(strcmp(args[i],"<") == 0) {
            return (i+1);
        }
    }
    return -1;
}
// checks to see if a given file is open and if not exits the current proccess
void check_open(int i, char* file) {
    if(i == -1) {
        printf("Cannot open %s\n", file);
        exit(1);

    }
}
//checks to see if the file redirection was successful and if not exits the current process
void check_dup2(int i, char* file) {
    if(i == -1) {
        printf("Error with dup2(%s)", file);
        exit(1);

    }
}
//this function was supposed to remove the file redirection arguments 
// from command->tokens. unfortunately after many many hours of trying to get
// it to work I gave up and chose a different approach 
void set_new_path(char** args, struct command_line** command) {

    for(int i = 0; i < (*command)->tok_num-1; i++) {

        if(strcmp((*command)->tokens[i].value,">") == 0) {
            strcpy((*command)->tokens[i+1].value,'\0');
            strcpy((*command)->tokens[i].value,'\0');
            (*command)->tok_num -= 2;
            return;
        }
        else if(strcmp((*command)->tokens[i].value,"<") == 0 ) {
            strcpy((*command)->tokens[i+1].value,'\0');
            strcpy((*command)->tokens[i].value,'\0');
            (*command)->tok_num -= 2;
            return;
        }
        
    } 

}

// this function ended up much bigger that I originally intended but that was because I was having trouble with the 
// file redirection so I had to put in what made it work. The function first checks if the process should be run in 
// the background if so it then check wether the user specified a path if so the command is redirected based on the 
// path and then executed. if no path was given the command executes and the output or input is sent to dev/null. 
// it works the same way if the command is to be run in the foreground except if no path is set stdout and stdin are 
// used. after forking the child processes also adjust the signal handlers based on how each process was supposed to
// handle signals. 
void fork_shell(struct command_line* command, struct shell_values* shell, struct sigaction SIGINT_action, struct sigaction SIGTSTP_action) {
       
    int new_path = 0;
    pid_t newpid = 5;
    int newpid_status;
    int dup_result = 0;
    //set target variable so if no target or source is found they are not used
    int target_left =-1;
    int target_right =-1;
    int source_left =-1;
    int source_right = -1;

    //created an array to hold values in tokens 
    // I use this array within the function but it ends up being fairly 
    // unnecessary because I am unable to use it when calling exec which
    // was its intended purpose
    char* args[command->tok_num];
    memset(args, '\0', sizeof(args));

    for(int i = 0; i < command->tok_num; i++) {
        
        args[i] = command->tokens[i].value;       
    }
    
    //check for background mode
    if((shell->forg_mode == 0) && (strcmp(args[(command->tok_num)-2], "&") == 0)) {
        
        newpid = fork();

        switch(newpid) {

            case -1:
                perror("fork() failed.");
                exit(1);              

            case 0:
                //set sigint to be ignored
                SIGINT_action.sa_handler = SIG_IGN;
	            sigfillset(&SIGINT_action.sa_mask);
	            SIGINT_action.sa_flags = 0;
                sigaction(SIGINT, &SIGINT_action, NULL);
                //set sigstop to be ignored
                SIGTSTP_action.sa_handler = SIG_IGN;
	            sigfillset(&SIGTSTP_action.sa_mask);
	            SIGTSTP_action.sa_flags = 0;
                sigaction(SIGTSTP, &SIGTSTP_action, NULL);
                // this loop ended up not being necessary
                for(int i = 0; i < command->tok_num; i++) {
        
                    args[i] = command->tokens[i].value;       
                }

                args[(command->tok_num)-2] = '\0';
                //check wether a new path is given by user 
                new_path = check_new_path(args, command);
                //if so new path == 1
                if(new_path == 1) {
                    //get target and source values
                    target_right = target_ar(args, command);
                    source_right = source_ar(args, command);
                    target_left = target_al(args, command);
                    source_left = source_al(args, command);

                    //check for double redirection
                    if(target_right != -1 && target_left != -1){
                        
                    }
                    //check for redirecting standard out
                    else if(target_right != -1) {
                        //redirect standard out
                        int targ_file = open(args[target_right], O_WRONLY | O_CREAT | O_TRUNC);
                        check_open(targ_file, args[target_right]);
                        dup_result = dup2(targ_file, 1);
                        check_dup2(dup_result, args[target_right]);
                        //set_new_path(args, &command);

                    }
                    //check for redirection standard in
                    else if (target_left != -1) {
                        //redirect standard in
                        int targ_file = open(args[source_left], O_RDONLY);
                        check_open(targ_file, args[source_left]);
                        dup_result = dup2(targ_file, 0);
                        check_dup2(dup_result, args[source_left]);
                        //set_new_path(args, &command);

                    }
                    //if no no file is given after < or > process exits
                    else {
                        printf("Must include target file");
                        exit(1);
                    }
                    //checks for extensions to commands then executed command
                    char* ptr = strchr(args[1],'-');
                    if(ptr!= NULL) {
                        char* new_args[] = {args[0], args[1], NULL};
                        execvp(new_args[0], new_args);
                    }
                    else {
                        char* new_args[] = {args[0], NULL};
                        execvp(new_args[0], new_args);
                    }
                    printf("\n%s: No such file or directory\n", args[0]);
                    fflush(stdout);
                    exit(1);
                    
                }

                else {
                    // if no path execution is sent to dev/null
                    int file =  open("/dev/null", O_WRONLY);
                    dup_result = dup2(file, 1); 
                    check_dup2(dup_result, "/dev/null");              
                    execvp(args[0], args);
                    printf("\n%s: No such file or directory\n", args[0]);
                    fflush(stdout);
                    exit(1);
                }
                
            default:
            //send child process id to shell processes array
            shell->processes[shell->num_processes] = newpid;
            shell->num_processes += 1;
            //indicate new backround proccess started
            printf("New background Process: %d began\nls",newpid);
            fflush(stdout);
            //wait for child to finish but allow parent process to continue in foreground
            pid_t childpid = waitpid(newpid, &newpid_status, WNOHANG);
            fflush(stdout);           
        }
    }

    else {
        //same as background only difference will be redirection with no path set
        pid_t childpid;
        newpid = fork();
        switch(newpid) {

            case -1:
                perror("fork() failed.");
                exit(1);
                
            case 0:
                
                SIGINT_action.sa_handler = SIG_DFL;
	            sigfillset(&SIGINT_action.sa_mask);
	            SIGINT_action.sa_flags = 0;
                sigaction(SIGINT, &SIGINT_action, NULL);

                SIGTSTP_action.sa_handler = SIG_IGN;
	            sigfillset(&SIGTSTP_action.sa_mask);
	            SIGTSTP_action.sa_flags = 0;
                sigaction(SIGTSTP, &SIGTSTP_action, NULL);

                new_path = check_new_path(args, command);
                
                if(new_path == 1) {
                    
                    target_right = target_ar(args, command);
                    source_right = source_ar(args, command);
                    target_left = target_al(args, command);
                    source_left = source_al(args, command);

                    if(target_right != -1 && target_left != -1){
                        
                    }
                    else if(target_right != -1) {
                        int targ_file = open(args[target_right], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        check_open(targ_file, args[target_right]);
                        dup_result = dup2(targ_file, 1);
                        check_dup2(dup_result, args[target_right]);

                        
                    }
                    else if (target_left != -1) {
                        int targ_file = open(args[source_left], O_RDONLY);
                        check_open(targ_file, args[source_left]);
                        dup_result = dup2(targ_file, 0);
                        check_dup2(dup_result, args[source_left]);
                        
                    }
                    else {
                        printf("\nMust include target file\n");
                        exit(1);
                    }
    
                    char* ptr = strchr(args[1],'-');
                    if(ptr!= NULL) {
                        char* new_args[] = {args[0], args[1], NULL};
                        execvp(new_args[0], new_args);
                    }
                    else {
                        char* new_args[] = {args[0], NULL};
                        execvp(new_args[0], new_args);
                    }
                    
                    printf("\n%s: No such file or directory\n", args[0]);
                    fflush(stdout);
                    exit(1);
                
                }
                else{
                    // command run and printed to stdout
                    execvp(args[0], args);
                    printf("\n%s: No such file or directory\n", args[0]);
                    fflush(stdout);
                    exit(1);
                }
                

            default:
                // hault until child process returns
                childpid = waitpid(newpid, &newpid_status, 0);
                fflush(stdout);
                //check if child proccess exited normally or not
                //if yes status is set in shell and set to exit status if no 
                // status is set to terminating signal and termination signal is 
                // printed to stdout
                if(WIFEXITED(newpid_status)){
                    shell->status = WEXITSTATUS(newpid_status);
                    shell->term_ext = 0;
                } 
                else{
                    shell->status = WTERMSIG(newpid_status);
                    shell->term_ext = 1;

                    printf(" Proccess terminated by signal %d\n",shell->status);
                    fflush(stdout);
                }
        }
    }   
    return;
}
//this function checks the array of proccess stored in the shell struct to see if there 
// are any zombie proccesses and cleans them up and prints what process it was 
void check_processes(struct shell_values* shell) {

    pid_t childpid = 0;
    int child_status;

    for(int i = 0; i < shell->num_processes; i++) {
        childpid = waitpid(shell->processes[i], &child_status, WNOHANG);

        if(childpid != 0 && childpid != -1){
            printf("Process: %d finished\n", childpid);
        }
    }
}


int main() {

    //set up signal handlers
    struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0};

    // Fill out the SIGINT_action struct
    // Register handle_SIGINT as the signal handler
    SIGINT_action.sa_handler = SIG_IGN;
    
    // Block all catchable signals while handle_SIGINT is running
	sigfillset(&SIGINT_action.sa_mask);
    // No flags set
	SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);

    //same as SIGINT except handler is set to custom handler
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);


    int skip;
    struct command_line command;
    struct shell_values shell; 
    char* input_buffer;
    size_t input_length; 
    int get_err = 0;
    //initialize shell variable
    set_shell_values(&shell);
    //loop as long as user chooses not to call command "exit"
    while(1) {

        skip = 0;
        input_buffer = NULL;
        input_length = 0;
        //set command struct values to null
        flush_command(&command);
        //check to see whether to change to or back from foreground only mode
        if(s_flag) {
            fflush(stdin);
            if(shell.forg_mode == 0) {
                shell.forg_mode = 1;    
            }
            else {
              shell.forg_mode = 0;  
            }
            s_flag = 0;
        }
        fflush(stdout);
        
        printf(": ");
        fflush(stdout);
        //use getline to get input from user
        get_err = getline(&input_buffer, &input_length, stdin); 
        //if signal interupts getline get_err will return -1 and clearerr will
        // clean up stdin 
        if (get_err == -1) {
            clearerr(stdin);
            continue;
        } 
        //set command variable
        skip = set_command(&command, input_buffer);
        //if bad input was given user is repromted by being sent to top of loop using continue
        if(skip == 1) {
            continue;
        }
        //check for built in command call
        skip = b_in_func(&command, &shell);
        //if a built in command is called or foreground only more is called loop is reset
        if(skip == 1 || s_flag == 1) {
            continue;
        }
        fork_shell(&command, &shell, SIGINT_action, SIGTSTP_action);
        check_processes(&shell);
        
    }
    //make sure input buffer is freed.
    free(input_buffer);

    return 0;
}

