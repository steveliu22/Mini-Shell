
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAX_BUFFER_SIZE 100
/*
    The data structure used to store a token. It will be a 2d array of characters 
    with each i-th array being the i-th token.
*/
typedef struct token_storer {
    char **tokens; //the tokens stored
    unsigned int length; //the number of tokens stored
} t_store;

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

int main(int argc, char** argv){

    t_store *input = parse(argv[1], " ");

    for(int i = 0; i < input->length; i += 1) {
        printf("%s\n", input->tokens[i]);
    }

    remove_from_memory(input);
    return 0;
}
