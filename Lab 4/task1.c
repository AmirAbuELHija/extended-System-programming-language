#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#define INPUT_SIZE 1024
#define FILE_NAME_SIZE 100

static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};

typedef struct{
    bool debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    bool display_mode; 
} state;

typedef void (*menu_function)(state*);
typedef struct {
  char* name;
  menu_function func;
} menu_FUNC;


bool withinRange(int bounds, int c){
  if (c > bounds && c < 0 ){
        printf("Not within bounds \n");
    }
    return c <= bounds && c >= 0;
}
void print_menu(menu_FUNC menu[]){
    for (int i = 0; menu[i].name != NULL; i++){
      printf("%d) %s \n", i, menu[i].name);
    }
}

int menu_range(menu_FUNC menu[]){
    int counter = 0;
    for (int i = 0; menu[i].name != NULL; i++)
        counter++;

    return counter - 1;
}

int FileSize(char *filename){
    FILE *file = fopen(filename, "r");
    int size = 0;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);
    return size;
}

void toggle_debug_mode(state *s){
    if (s->debug_mode){
        printf("Debug flag now off\n");
        s->debug_mode = false;
    }else{
        s->debug_mode = true;
        printf("Debug flag now on\n");
    }
}


void set_file_name(state *s){
    printf("\nEnter file name: ");
    fgets(s->file_name, sizeof(s->file_name), stdin);
    s->file_name[strcspn(s->file_name, "\n")] = '\0'; 
    if (s->debug_mode == true) {
        fprintf(stderr, "Debug: file name set to '%s'\n", s->file_name);
    }
}

void set_unit_size(state *s){
    int UnitSize;
    printf("Enter unit size 1, 2, or 4: ");
    scanf("%d", &UnitSize);

    if (UnitSize == 1 || UnitSize == 2 || UnitSize == 4) {
        s->unit_size = UnitSize;
        if (s->debug_mode == true) {
            fprintf(stderr, "Debug: set size to %d\n", s->unit_size);
        }
    } else {
        printf("Invalid unit size.\n");
    }
    while (getchar() != '\n'); 
}


void load_into_memory(state *s){
    if (strcmp(s->file_name, "") == 0) {
        printf("Error: There is no specified file name.\n");
        return;
    }
    FILE* file = fopen(s->file_name, "rb");
    if (file == NULL) {
        printf("Failed to open file: %s\n", s->file_name);
        return;
    }
    unsigned int location, length;
    char input[INPUT_SIZE];
    printf("Please enter <location> <length>\n");
    fgets(input, INPUT_SIZE, stdin);
    sscanf(input, "%x %d", &location, &length);

    if (s->debug_mode == true) {
        fprintf(stderr, "File Name: %s, Location: %x, Length: %d\n", s->file_name, location, length);
    }

    fseek(file, 0, SEEK_END);
    unsigned int file_size = ftell(file);
    if (location >= file_size) {
        printf("Error: Invalid location. Location should be within the file's size.\n");
        fclose(file);
        return;
    }
    fseek(file, location, SEEK_SET);
    fread(s->mem_buf, s->unit_size, length, file);
    printf("Loaded %d units into memory\n", length);
    s->mem_count = s->unit_size * length;
    fclose(file);
}

void toggle_display_mode(state *s){
    if (s->display_mode == 0) {
        s->display_mode = 1;
        printf("Display flag now on, hexadecimal representation\n");
    } else {
        s->display_mode = 0;
        printf("Display flag now off, decimal representation\n");
  }
}

void memory_display(state *s){
    if (strcmp(s->file_name, "") == 0) {
    printf("Error: No file loaded.\n");
        return;
    }
    char input[INPUT_SIZE];
    unsigned int addr, length;

    printf("Enter address and length:\n");
    fgets(input, INPUT_SIZE, stdin);
    sscanf(input, "%x %d", &addr, &length);

    if (s->display_mode == 0) {
        printf("Decimal\n=======\n");
        for (unsigned int i = 0; i < length; i++) {
            unsigned char* ptr = &s->mem_buf[(addr + i) * s->unit_size];
            unsigned int val = 0;
            for (unsigned int j = 0; j < s->unit_size; j++) {
                val += (unsigned int)(*ptr++) << (j * 8);
            }
            printf(dec_formats[s->unit_size - 1], val);
        }
    } else if (s->display_mode == 1) {
        printf("Hexadecimal\n===========\n");
        for (unsigned int i = 0; i < length; i++) {
            unsigned char* ptr = &s->mem_buf[(addr + i) * s->unit_size];
            unsigned int val = 0;
            for (unsigned int j = 0; j < s->unit_size; j++) {
                val += (unsigned int)(*ptr++) << (j * 8);
            }
        printf(hex_formats[s->unit_size - 1], val);
        }   
    }
}

void save_into_file(state *s){
    if (strcmp(s->file_name, "") == 0) {
        printf("Error: There is no specified file name.\n");
        return;
    }

    FILE* file = fopen(s->file_name, "r+b");
    if (file == NULL) {
        printf("Error: Failed to open file %s.\n", s->file_name);
        return;
    }
    unsigned int source_addr, target_loc, length;
    char input[INPUT_SIZE];
    printf("Please enter <source-address> <target-location> <length>\n");
    fgets(input, INPUT_SIZE, stdin);
    sscanf(input, "%x %x %d", &source_addr, &target_loc, &length);

    if (target_loc >= FileSize(s->file_name)){
        printf("%s", "Error: target location is greater than the file's size\n");
        return;
    }
    unsigned char *buffer = (unsigned char *)s->mem_buf;
    if (source_addr != 0){
        buffer = (unsigned char *)source_addr;
    }
    fseek(file, target_loc, SEEK_SET);
    fwrite(buffer, s->unit_size, length, file);
    fclose(file);
}

void modify_memory(state *s){
    unsigned int location, value;
    char input[INPUT_SIZE];
    printf("Please enter <location> <val>\n");
    fgets(input, INPUT_SIZE, stdin);
    sscanf(input, "%X %X", &location, &value);

    if (s->debug_mode == true) {
        fprintf(stderr, "Location: %x, Value: %X\n",location, value);
    }
    if (location > s->mem_count){ 
        printf("%s", "Error: not enough space in membuf!\n");
        return;
    } else if ((s->unit_size == 1 && value > UCHAR_MAX)|| (s->unit_size == 2 && value > USHRT_MAX)|| (s->unit_size == 4 && value > UINT32_MAX)){
        printf("%s","Error: unrelated value!\n");
        return;
    }
    memmove(s->mem_buf+location,&value,s->unit_size);
}

void quit(state* s) {
    if(s->debug_mode){
        fprintf(stderr, "Quitting\n");
    }
    free(s);
    exit(0);
}


int main(int argc, char **argv){
    state *s = (state *)(malloc(sizeof(state)));
    s->debug_mode = false;
    s->unit_size = 1;
    strcpy(s->file_name, "");
    s->mem_count = 0;
    s->display_mode = false;
    int inputParse = 0;
    char input[INPUT_SIZE];
    char temp[INPUT_SIZE];
    menu_FUNC menu[] = {{"Toggle debug mode", toggle_debug_mode},
                        {"Set file name", set_file_name},
                        {"Set unit size", set_unit_size},
                        {"Load into memory", load_into_memory},
                        {"Toggle display mode", toggle_display_mode},
                        {"Memory display", memory_display},
                        {"Save into file", save_into_file},
                        {"Modify memory", modify_memory},
                        {"Quit", quit},
                        {NULL, NULL}};
    int menuRange = menu_range(menu);
    while (true){
        printf("%s", "Please choose an action \n");
        print_menu(menu);
        printf("Option: ");
        fgets(input, INPUT_SIZE, stdin);
        sscanf(input, "%s", temp);
        inputParse = atoi(temp);
        if (withinRange(menuRange, inputParse)){
            (*menu[inputParse].func)(s);
        }
        printf("%s", "\n");
    }
}

























