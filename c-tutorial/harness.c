#include "list.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

char echo(FILE *file){

    char line [41];

    while (fgets(line, sizeof(line), file) != NULL) {
                    
        // Skip empty lines
        if (strlen(line) > 0) {
            // Remove newline character if present
            char *newline = strchr(line, '\n');
            while(newline != NULL){
                *newline = ' ';
                newline = strchr(line, '\n');
            }
            // Tokenize the line into words and display them
            char *word = strtok(line, " ");
            while (word != NULL) {
                printf("%s\n", word);
                word = strtok(NULL, " ");
            }
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Please write 2 parameters");
        return 1;
    }

    char *fileName = argv[1];
    char *operation = argv[2];

    FILE *file = fopen(fileName, "r");


        fseek(file, 0, SEEK_END);
        if (ftell(file) == 0) {
            printf("<EMPTY>\n");
            fclose(file);
            return 0;  // File is empty
        }
        fseek(file, 0, SEEK_SET);

    if (file == NULL) {
        printf("EMPTY");
        return 1;
    }
            
    if (strcmp(operation, "echo") == 0) {
        echo(file);
    } else {
        list_t myList;
        list_init(&myList, NULL, NULL); 

        char line [41];

        while (fgets(line, sizeof(line), file) != NULL) {
            // Remove newline character if present
            char *newline = strchr(line, '\n');
            while(newline != NULL){
                *newline = ' ';
                newline = strchr(line, '\n');
            }
            char *word = strtok(line, " ");
            while (word != NULL) {
                size_t word_length = strlen(word);
                char *word_copy = malloc(word_length + 1);  
                if (word_copy != NULL) {
                    strcpy(word_copy, word);
                    list_insert_tail(&myList, word_copy);
                }
                word = strtok(NULL, " ");

            }
        }
        
        if (strcmp(operation, "tail") == 0) {
                if(is_list_empty(&myList)){
                    printf("<EMPTY>\n");
                }
                else{
                    list_visit_items(&myList, print_line);
                }             
            } else if (strcmp(operation, "tail-remove") == 0) {  

                if(is_list_empty(&myList)==0){
                        printf("<EMPTY>\n");
                    }

                while(is_list_empty(&myList)==0){
                    list_remove_head(&myList);
                    list_remove_head(&myList);
                    list_remove_head(&myList);
                    list_visit_items(&myList, print_line);
                    if(is_list_empty(&myList)==0){
                        printf("---------------\n");
                    }
                }
                printf("\n");
        }  
    }
    return 0;
}