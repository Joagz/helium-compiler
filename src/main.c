#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RETURN 0x01
#define INT_LIT 0x02
#define SEMI 0x03

#define RETURN_SIZE_ASM 39

typedef  struct {
    int type;
    char * value;
} token ;

void put_in_token_arr(token * tokens, char buff[], int buff_size, int *t, int type){

    char * val = (char*) malloc(buff_size); 
    bzero(val, buff_size);

    strcpy(val, buff);
    token tkn = {type, val};
    tokens[*t] = tkn;
    (*t)++; 
}

token * tokenize(const char str[], size_t size, size_t * tokens_size){

    token * tokens = (token*) malloc(sizeof(token) * size);
    bzero(tokens, sizeof(tokens));

    char *buff = (char*)malloc(size);
    bzero(buff, size);

    for(int i = 0, j = 0, t=0; i < size; i++, j=0){  

        char c = str[i];
        if(isspace(c)) continue;

        else if (str[i] == ';') {
            token tkn = {SEMI, NULL};
            tokens[t] = tkn;
            (*tokens_size)++;
        }

        else if(isalpha(c)){

            while(isalpha(str[i]) && str[i] != ';') {
                buff[j]=str[i];
                j++;
                i++;
            }  

            if(strcmp(buff, "return") == 0){
                put_in_token_arr(tokens, buff, sizeof(buff), &t, RETURN);
                (*tokens_size)++;
            }
            i--;

        }

        else if(isdigit(c)){

            while(isdigit(str[i]) && str[i] != ';') {
                buff[j]=str[i];
                j++;
                i++;
            } 

            put_in_token_arr(tokens, buff, sizeof(buff), &t, INT_LIT);
            (*tokens_size)++;
            i--; 
        }

        bzero(buff, sizeof(buff));
    }
    free(buff);

    return tokens;

}

char * allocate_extra(char * in, size_t size) {
    char * newVal = (char*) malloc(size);

    strcpy(newVal, in);

    return newVal;
}

char * tokens_to_asm(token tokens[], size_t size){
    char start[] =  "global _start\nstart:\n";
    char * output = (char*) malloc(strlen(start) + 1); 
    strcpy(output,start);

    for(int i = 0; i < size; i++) {

        if(tokens[i].type == RETURN){


            if(i+1 < size && tokens[i+1].type == INT_LIT){
                if(i + 2 < size  && tokens[i+2].type == SEMI){
                    size_t needed_size = RETURN_SIZE_ASM + strlen(tokens[i + 1].value) + 1;
                    if (strlen(output) < needed_size) {
                        output = allocate_extra(output, needed_size + strlen(output));
                    }

                    strcat(output, "    mov rax, 60\n");
                    strcat(output, "    mov rdi, ");
                    strcat(output, tokens[i + 1].value);
                    strcat(output, "\n    syscall");


                }
            }

        }

    }
    printf(output);

    return output;     

}

int create_asm_file(char * str){

    FILE * fptr;

    fptr = fopen("out.asm", "w+");

    if(fptr == NULL){
        printf("Could not create assembly file");
        return -1;
    }

    fprintf(fptr, str);

    fclose(fptr);

    return 0;

}

int main(int argc, char** argv) {

    if(argc != 2){
        printf("Incorrect usage, please try: helium <input.he>");
    }

    FILE * file;
    file = fopen(argv[1], "r+");

    if(file == NULL){exit(-1);}
    char buffer[64];
    bzero(buffer, sizeof(buffer));

    token * tokens;
    size_t tokens_size = 0;
    size_t total_token_size = 0;
    size_t * ts_pointer = &tokens_size;

    int available_size = 255;
    char * to_write = (char*) malloc(available_size);

    while(fgets(buffer, sizeof(buffer), file)){
        token * temp = tokenize(buffer, sizeof(buffer), ts_pointer);

        if(temp == NULL){
            printf("Could not get tokenized values. TOKENS == NULL");
            exit(0);
        }
        total_token_size += *ts_pointer;


        char * assembly = tokens_to_asm(temp, *ts_pointer);
        
        if(available_size < strlen(assembly)){
            to_write = (char *) malloc(available_size + strlen(assembly));
        }

        strcat(to_write, assembly);

        available_size -= strlen(assembly);

        free(assembly);

        for(int i = 0; i < tokens_size; i++){



            if(temp[i].value != NULL)
                free(temp[i].value);
        }

        *ts_pointer=0;
        tokens_size=0;

        free(temp);
    }

    create_asm_file(to_write);

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    fclose(file);

    return 0;
}

