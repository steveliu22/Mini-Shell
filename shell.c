#include <stdio.h>
#include <stdlib.h>
#include <signal.h> 
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 100 //Max input length for each line of command.
char *prev_cmd; //Stores the user's previous command.


/*
    The data structure used to store a token. It will be a 2d array of characters 
    with each i-th array being the i-th token.
*/
typedef struct token_storer {
    char **tokens; //tokens to be stored.
    unsigned int length; //
} t_store;

/* Executes and runs an instance of the mini-shell. Runs until the user either 
   enters the "exit" command or the user presses CTRL + C. */
void execute_mini_shell();

/* Executes the specified command string provided. Parses between the delimiters "|", ";", 
   and " ".  */
void execute_command(char* cmds);

/* Executes a piped command with a single pipe in it.  */
void execute_piped_commands(t_store *t);

/* Same thing as execute_command except that the provided command is executed in a child 
   process rather than in the parent process. */
void execute_command_fork(char* cmd);

/* Signal handler to determine whether the user has decided to terminate the shell. */
void sigint_handler(int sig){

	write(1,"\nmini-shell terminated\n", sizeof("\nmini-shell terminated\n")); 
	
    exit(0);
}


/*
A parser that splits the provided string into a set of tokens based on the provided delimiter.
*/
t_store *parse(char* text, char* delimiter) {
    if(text == NULL) { //checks whether the provided text is nothing.
        return NULL;
    }

    //Allocating memory for token storage
    t_store *t = (t_store*) malloc(sizeof(t_store)); 
    
    if(t == NULL) {
        printf("Insufficient memory.\n");
        exit(1);
    }

    //Current token being parsed from the input text.
    char* curr_token = strtok(text, delimiter); 
    int curr_counter = 0; //Counting the number of tokens

    //Allocating memory for array of tokens.
    char** parsed_tokens = (char**) malloc(sizeof(char*) * MAX_BUFFER_SIZE); 

    if(parsed_tokens == NULL) {
        
        printf("Insufficient memory.\n");
        
        exit(1);
    }

    while(curr_token != NULL) {
        //Allocating memory for the current token.
        parsed_tokens[curr_counter] = malloc(sizeof(char*) * strlen(curr_token)); 

        if(parsed_tokens[curr_counter] == NULL) {
            printf("Insufficient memory.\n");
            exit(1);
        }

        //Storing the current token being parsed into the array.
        strcpy(parsed_tokens[curr_counter], curr_token); 

        curr_counter++; 

        //Checking whether or the parser has run out of text.
        curr_token = strtok(NULL, delimiter); 
    }


    t->tokens = parsed_tokens;
    t->length = curr_counter;

    return t;
}



/* Frees the provided token storage from memory. */
void remove_from_memory(t_store *t) {
    
    for(int i = 0; i < t->length; i+= 1) {
        free(t->tokens[i]); //Frees the current token.
    }

    free(t->tokens);

    free(t);
}

/* Executes the cd command, which changes directory of the current running shell. 
   The token storage should be parsed based on the " " delimiter. */
void cd_command(t_store *t) {

    char* specified_directory = t->tokens[1]; //The directory for cd to change into.

    int dir_result = chdir(specified_directory); //Changing the directory.

    if(dir_result == -1) { //Checking whether or not the specified directory exists.
        printf("-mini-shell: cd: %s: No such file or directory found.\n", specified_directory); 
    }

    else { //Directory has been moved successfully. 
        printf("-mini-shell: Current directory moved to %s \n", specified_directory);
    }
}

/* Executes a set of commands from each line of text from the provided file. 
   The token storage should be parsed based on the " " delimiter. */
void src_command(t_store *t) {
    char* file_directory = t->tokens[1]; //The file directory to be read from.
    
    FILE *file = fopen(file_directory, "r"); //Opening the file read-only.

    if(file == NULL) { //File wasn't opened correctly.
        printf("Error trying to open %s\n", file_directory);
        exit(0);
    }

    char *line = (char*) malloc(sizeof(char) * MAX_BUFFER_SIZE); //Allocating memory to store the read command.

    if(line == NULL) {
        printf("Insufficient memory.\n");
        exit(1);
    }
    
    while(fgets(line, MAX_BUFFER_SIZE, file) != NULL) { //Parsing through each command in the file and executing them.
       
        execute_command(line);
    
    }


    fclose(file); //Closing the file.

    free(line);

}

/*
    Returns the help command for this mini shell. Returns information about all of the different built-in commands.  
*/
char* help_command() {

    int intro = sizeof("Steve's mini-shell.\nType in a program and its arguments to run.\n");
    int cmds = sizeof("Built in commands are:\n");
    int exiting = sizeof("exit -- exits the current mini-shell that you are working on\n");
    int cd = sizeof("cd [dir-path] -- moves the current directory to the specified directory location \n");
    int source = sizeof("source [file-name] -- executes all of the commands from the provided source file\n");
    int prev = sizeof("prev -- re-executes your previous command\n");
    int help = sizeof("help -- provides information on all of the built-in commands");

    //Allocating the proper memory to store the command.
    char* help_cmd = 
    (char *) malloc(sizeof(char) * (intro + cmds + exiting + cd + source + prev + help) + 1);

    //Memory error.
    if(help_cmd == NULL) {
        printf("Insufficient memory.\n");
        exit(1);
    }

    help_cmd = "Steve's mini-shell.\nType in a program and its arguments to run."
    "\nBuilt in commands are:"
    "\nexit -- exits the current mini-shell that you are working on"
    "\ncd [dir-path] -- moves the current directory to the specified directory location"
    "\nsource [file-name] -- executes all of the commands from the provided source file"
    "\nprev -- re-executes your previous command"
    "\nhelp -- provides information on all of the built-in commands\n";

    return help_cmd;
}

/* Executes other linux commands in the shell that are not built in to this mini-shell such as 'ls', 'cat' etc. */
void execute_not_built_in_program(t_store *input) {
    char* args[input->length + 1]; //Creating array for storing the arguments used to execute the program.

    for(int i = 0; i < input->length; i += 1) {
        args[i] = input->tokens[i];
    }

    args[input->length] = NULL;

    int cmd = execvp(args[0], args); //Executing the command.
        
    if(cmd == -1) { //Command failed to execute or can't be found.
         
        printf("%s: Command not found.\n", args[0]);
         
        exit(1);
    }


}

/* Executes the specified program based on the provided tokens. Should be parsed based on the " " delimiter. */
void execute_program(t_store *input) {
    
    char* specified_cmd = input->tokens[0]; //Checks the specified command to be executed.

    if(strcmp(specified_cmd, "cd") == 0) { //Case where the command is cd.
        cd_command(input);
    }  

 /* Case where the command is source, since this function is only called in functions 
    where it forks into a child process, exit must be called at the end.     */
    else if(strcmp(specified_cmd, "source") == 0) { 

        src_command(input);
        exit(0);
    }    

    //Executes the previous command.
    else if(strcmp(specified_cmd, "prev") == 0) {
       
        printf("%s\n", prev_cmd); //Prints the previous command.
        execute_command(prev_cmd); //Executes the previous command.
    
    } 

    /*Case where the command is help, since this function is only called in functions where it forks into a 
    child process, exit must be called at the end.   */
    else if(strcmp(specified_cmd, "help") == 0) {    
        
        char* helpers = help_command();
        
        fputs(helpers, stdout);
        
        free(helpers); //Freeing the char array for the help command.
        
        exit(0); 
    }

    /* Case where the command is exit. Since this command is being executed in a child process, 
       exit must be called at the end.  */
    else if(strcmp(specified_cmd, "exit") == 0) {
       
       exit(0);
    
    }

    //Executes a non-built in program script.
    else {
        execute_not_built_in_program(input);
    }

      

}

/* Executes multiple commands in a row given a set of tokens that have been 
   split based on the ";" delimiter.  */
void process_multiple_commands(t_store *tokes) {
    

    /* This for loop executes the i-th command in the provided tokens by 
       creating a separate child process. */
    for(int i = 0; i < tokes->length; i += 1) {

        int status; //Status of the current child.
        pid_t pid = fork(); 
        
        //Parsing the i-th command to be executed.
        t_store *cmd = parse(tokes->tokens[i], " "); 

        if(pid == -1) { //Checking if there is an error in forking.
            printf("Error creating child.\n");
            exit(1);        
        }

        if(pid == 0) {
            execute_program(cmd); //Performing the command in the child process.
        }

        else {
            //Waiting for the child process to finish executing.
            waitpid(-1, &status, 0);

            //Frees the current token and continues.
            remove_from_memory(cmd);

        }
    }

    for(int i = 0; i < tokes->length; i += 1) {
        
        exit(0); //exiting out of all of the children processes
    
    }

}

/* Executes the provided command by first parsing it in the descending delimiter order "|", ";" 
   and " " and then executing the tokens created. */
void execute_command(char* cmds) {
    
    //The tokens for the parsed command to be stored into.
    t_store *tokens; 
    
    //Must first check if the command contains a pipe since that has to be executed first.
    if(strchr(cmds, '|') != NULL) { 
        
        //Parsing the command into tokens split by pipes.
        tokens = parse(cmds, "|"); 
        execute_piped_commands(tokens);
    } 

    /* Next check if the command contains a multiple command indicator since that has to be 
       executed second. */
    else if(strchr(cmds, ';') != NULL) { 
        //Parsing the command into tokens split by the multiple commands indicator. 
        tokens = parse(cmds, ";"); 
        process_multiple_commands(tokens);
    }   

    else {
    
    /*  Lastly check if the command does not contain a pipe or multiple command 
        indicator, execute it normally using space. */

        tokens = parse(cmds, " "); 
        execute_program(tokens);    
    }

    //Removes the create token from memory after it has been used.

    remove_from_memory(tokens); 

}

/*
    Checking whether or not the provided command is indicating to exit. Assumption is 
    that the command is >= length of 4. Returns 0 if the command is exit or 1 if not.
*/
int check_exit(char* cmds) {

    if(cmds[0] == 'e' && cmds[1] == 'x' && cmds[2] == 'i' && cmds[3] == 't') { 
        return 0;
    }  

    else {
        return 1;
    }
}

/* Same thing as execute_command except that the provided command is executed in a child 
   process rather than in the parent process. */
void execute_command_fork(char* cmds) {
 
    pid_t pid = fork();
    
    if(pid == -1) { //Checking if there is an error in forking.
     
     printf("Error creating child.\n");            
     
     exit(1);        
    }
    
    if(pid == 0) { //Executing the command in the child.

        execute_command(cmds);

    }

    else { /*The parent waits for the child to finish executing and then checks 
             if the provided user command is an exit command. Since execute_command 
             does nothing in this case, the parent process will 
             be the one responsible for handling exiting out of the current shell process.. */
            

        wait(NULL);

        if(strlen(cmds) >= 4) {
            
            if(check_exit(cmds) == 0) { //Exits out of the current shell process.
                exit(0);
            }
        }

    }
}

/*
    Pipes the two provided commands together. The result of the first command will be used as input to the second command.
*/
 void pipe_help(char* cmd1, char* cmd2) {
    int pipefd[2], status;

    pid_t cpid[2];

    /* Create a pipe and confirm it was created
    pipefd[0] is the read end of the pipe
    pipefd[1] is the write end of the pipe
    Data written to the write end of the pipe is
    buffered by the kernel until it is read
    from the read end of the pipe */
    int piped = pipe(pipefd);

    if(piped == -1) {
        
        printf("Pipe failure.");

        exit(EXIT_FAILURE);
    }

    cpid[0] = fork();

    if(cpid[0] == -1) { //Checking if there is an error in forking.
    
     
        printf("Error creating child.\n");            
     
        exit(1);        
    }

    if(cpid[0] == 0) {

        close(pipefd[0]); //closing reading side of pipe in child.

        dup2(pipefd[1], STDOUT_FILENO); //move output to write side of pipe.
        
        execute_command(cmd1); //Execute the command.

    }

    if(cpid[1] == -1) { //Checking if there is an error in forking.
     
        printf("Error creating child.\n");            
     
        exit(1);        
    }

    cpid[1] = fork();

    if(cpid[1] == 0) {
        
        close(pipefd[1]); //closing writing side of pipe in child.

        dup2(pipefd[0], STDIN_FILENO); //move input to read side of pipe.

        execute_command(cmd2); //Execute the command.
    }

    //closing both of the pipes in the parent.

    close(pipefd[1]);

    close(pipefd[0]);

    //waiting for both child processes to finish executing.
    waitpid(-1, &status, 0);

    waitpid(-1, &status, 0);

 }

/*
    Pipes the provided tokens together as long as there is only one pipe. 
    The result of the first command will be used as input to the second command.
*/
 void execute_piped_commands(t_store *t) {

    pipe_help(t->tokens[0], t->tokens[1]);

}

//Executes and runs an instance of the mini-shell.
void execute_mini_shell() {

    //The buffer to which the command line input will be stored.
    char *line = (char*) malloc(sizeof(char) * MAX_BUFFER_SIZE);
    
    while(1) {

        printf("Steve's mini-shell> ");

        //Storing the input from stdin into the line buffer.
        char* input = fgets(line, MAX_BUFFER_SIZE, stdin); 

        //If nothing is inputted, continue to the next iteration.
        if(strcmp(input, "\n") == 0) {
           continue;
        }

        //Getting rid of new lines from the input.
        line[strcspn(line, "\n")] = 0;  

        /* Checking whether or not the input command is "prev". If it's not prev, 
           Set the previous command to the one currently being entered.*/
        
        if(strcmp(line, "prev") != 0) {
        
            prev_cmd = (char *) malloc(sizeof(char) * sizeof(line));

            strcpy(prev_cmd, line);
        } 

        /* Checking whether or not the input command contains a pipe.
           If it does, the command has to be executed in the parent since pipe
           will already have children for the command to execute in.*/

        if(strchr(line, '|') != NULL) {
            execute_command(line);
        } 

        /* Execute the command in a child process.*/
        else {
            execute_command_fork(line); 
        }
    
    }

}

int main() {

    //Installing the signal interrupter.

    signal(SIGINT, sigint_handler); 

    execute_mini_shell();

    free(prev_cmd);

    return 0;

}

