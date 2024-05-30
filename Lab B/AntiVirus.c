#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 10000

typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link link;

struct link {
    link *nextVirus;
    virus *vir;
};

void printHex(unsigned char* buffer, int length) {
    for (int i = 0; i < length; i++) {
        printf("%02X ", buffer[i]);
    }
}

void printVirus(virus* virus, FILE* output) {
    fprintf(output, "Virus name: %s\n", virus->virusName);
    fprintf(output, "Virus size: %d\n", virus->SigSize);
    fprintf(output, "Virus signature:\n");
    printHex(virus->sig, virus->SigSize);
    fprintf(output, "\n");
}

void list_print(link *virus_list, FILE *output) {
    if (virus_list == NULL) {
        fprintf(output, "the list is empty\n");
    } else {
        link *current = virus_list;
        while (current != NULL) {
            printVirus(current->vir, output);
            fprintf(output, "\n");
            current = current->nextVirus;
        }
    }
}

link* list_append(link *virus_list, virus *data) {
    link *new_node = (link*) malloc(sizeof(link));
    if (new_node == NULL) {
        printf("Error: failure to allocate memory\n");
        return virus_list;
    }
    new_node->vir = data;
    new_node->nextVirus = NULL;
    if (virus_list != NULL) {
        link *current = virus_list;
        while (current->nextVirus != NULL) {
            current = current->nextVirus;
        }
        current->nextVirus = new_node;
        
    } else {
        virus_list = new_node;
    }
    return virus_list;
}

virus* readVirus(FILE* file) {
    virus* new_virus = (virus*)malloc(sizeof(virus));
    if(new_virus == NULL){
        printf("Error: failure to allocate memory\n");
        return NULL;
    }
    new_virus->SigSize = 0;
    fread(&new_virus->SigSize, 2, 1, file);
    fread(new_virus->virusName, 16, 1, file);
    new_virus->virusName[16] = '\0';
    new_virus->sig = (unsigned char *)malloc(new_virus->SigSize);
    fread(new_virus->sig, new_virus->SigSize, 1, file);
    return new_virus;
}

link* LoadSignatures(char *fileName, link *virus_list){
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        printf("Error: file %s could not be opened\n", fileName);
        return virus_list;
    }
    char magicNumber[4];
    fread(magicNumber, 4, 1, file);
    if (strcmp(magicNumber, "VISL") != 0) {
        fprintf(stderr, "Error: magic number is incorrect\n");
        exit(1);
    }
    while (!feof(file)) {
        virus* new_virus = readVirus(file);
        if(new_virus && new_virus->SigSize > 0){
            virus_list = list_append(virus_list, new_virus);
        }
        else{
            free(new_virus->sig);
            free(new_virus);
        }
    }
    fclose(file);
    printf("Successful loading of signatures from file %s\n\n", fileName);
    return virus_list;
}

void list_free(link *virus_list) {
    link *current = virus_list;
    while (current != NULL) {
        link *temp = current;
        current = current->nextVirus;
        free(temp->vir->sig);
        free(temp->vir);
        free(temp);
    }
}

 void detect_viruses(char *buffer, unsigned int size, link *virus_list){
    if (size == 0) {
        return;
    }
    link *current = virus_list;
    while (current) {
        virus *v = current->vir;
        int signature_size = v->SigSize;
        for (int i = 0; i <= size - signature_size; i++) {
            if (memcmp(buffer + i, v->sig, signature_size) == 0) {
                printf("Virus detected at byte %d\n", i);
                printf("Virus name: %s\n", v->virusName);
                printf("Signature size: %d bytes\n", signature_size);
                printf("\n");
            }
        }
        current = current->nextVirus;
    }
}

void neutralize_virus(char *fileName, int signatureOffset) {
    FILE *file = fopen(fileName, "r+");
    if (file == NULL) {
        printf("Error: file %s could not be opened\n", fileName);
        return;
    }
    fseek(file, signatureOffset, SEEK_SET);
    char ret[] = {0xC3}; 
    fwrite(ret, sizeof(char), 1, file);
    fclose(file);
}

void fixFile(char *fileName, link *virus_list){
    FILE *file = fopen(fileName, "r+");
    if (file == NULL) {
        printf("Error: file %s could not be opened\n", fileName);
        return;
    }
    char buffer[BUFFER_SIZE];
    int size = fread(buffer, 1, BUFFER_SIZE, file);
    fclose(file);
    if (size == 0) {
        return;
    }
    link *current = virus_list;
    while (current) {
        virus *v = current->vir;
        int signature_size = v->SigSize;    
        for (int i = 0; i <= size - signature_size; i++) {
            if (memcmp(buffer + i, v->sig, signature_size) == 0) {
                neutralize_virus(fileName, i);
            }
        }
        current = current->nextVirus;
    }
}

link* load(link* list_vir, char** argv) {
    char fileName[1024];
    printf("Please enter filename: ");
    int c;
    while ((c = getchar()) != '\n' && c != EOF); // Clear input buffer
    fgets(fileName, 1024, stdin);
    fileName[strcspn(fileName, "\n")] = 0; // take the name without \n
    // Call LoadSignatures() with fileName and list_vir for further processing
    return LoadSignatures(fileName, list_vir);
}

link* print(link *list_vir, char** argv){
    list_print(list_vir, stdout);
    return list_vir;
}

link* detect(link *list_vir, char** argv){
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error: file %s could not be opened\n", argv[1]);
        return list_vir;
    }
    char buffer[BUFFER_SIZE];
    unsigned int size = fread(buffer, 1, BUFFER_SIZE, file);
    fclose(file);
    // Call LoadSignatures() with fileName and list_vir for further processing
    detect_viruses(buffer, size, list_vir);
    return list_vir;
}

link* fix(link *list_vir, char** argv){
    fixFile(argv[1],list_vir);
    return list_vir;
}

link* quit(link *list_vir, char** argv){
    list_free(list_vir);
    return list_vir;
}

struct fun_desc {
char *name;
link* (*fun)(link*, char**);
};


int main(int argc, char** argv) {
    link *virus_list = NULL;
    struct fun_desc menu[] = {{"Load signatures", load},{"Print signatures", print},{"Detect viruses", detect},{"Fix file", fix},{"Quit", quit},{NULL, NULL}};
    int size_of_the_menu = sizeof(menu)/sizeof(struct fun_desc)-1;
    printf("Please choose a function:\n");
    for(int i=0; i<size_of_the_menu; i++){
      printf("%d) %s\n", i+1, menu[i].name);
    }
    printf("Option: ");
    while(1){
    int option;
    scanf("%d",&option);
    if(option < 1 || option > 5){
        printf("Not within bounds\n");
    } else {
        if(option==5){
        virus_list = menu[option-1].fun(virus_list, argv);
        exit(1);
        }else if((option == 4 || option == 3) && argc < 2){
            printf("Error: there is no file to fix, you have to write name of the file in the arguements\n");
            exit(1);
        }else{
        virus_list = menu[option-1].fun(virus_list, argv);
        }
    }
    printf("Please choose a function:\n");
    for(int i=0; i<size_of_the_menu; i++){
      printf("%d)  %s\n", i+1, menu[i].name);
    }
    printf("Option: ");
    }
 
   return 0;

}

