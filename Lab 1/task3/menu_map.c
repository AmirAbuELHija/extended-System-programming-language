#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  for (int i = 0; i < array_length; i++){
    mapped_array[i] = f(*(array+i));
  }
  return mapped_array;
}
char* get_string(){
  char *str = (char*)(malloc(5*sizeof(char)));
  fgets(str, 5, stdin);
  return str;
}
char my_get(char c){
  return fgetc(stdin);
}

char cprt(char c){
  if(c >= 0x20 && c <= 0x7E){
    printf("%c\n", c);
  }else{
    printf(".\n");
  }
  return c;
}

char encrypt(char c){
  if(c >= 0x20 && c <= 0x7E){
    return c + 1;
  }
  return c;
}

char decrypt(char c){
  if(c >= 0x20 && c <= 0x7E){
    return c - 1;
  }
  return c;
}

char xprt(char c){
  if(c >= 0x20 && c <= 0x7E){
    printf("%x\n", c);
  }else{
    printf(".\n");
  }
  return c;
}

struct fun_desc {
char *name;
char (*fun)(char);
};

int main(int argc, char **argv){
char* carray = (char*)(malloc(5*sizeof(char)));
  struct fun_desc menu[] = {
        {"Get String", my_get},
        {"Print String", cprt},
        {"Encrypt", encrypt},
        {"Decrypt", decrypt},
        {"Print Hex", xprt},
        {NULL, NULL}
    };
  printf("Please choose a function (ctrl^D for exit):\n");
  char input[10];
  for(int i=0; i<5; i++){
      printf("%d)  %s\n", i, menu[i].name);
  }
  printf("Option: ");
  
  while(fgets(input, 10, stdin) != NULL){
    printf("\n");
    int option = input[0]-'0';
    
    if(option >= 0 && option < 5){
      printf("Within bounds\n");
      carray = map(carray, 5, menu[option].fun);
    }

    else{
      printf("Not within bounds\n");
      break;
    }
    printf("DONE.\n\n");
    printf("Please choose a function (ctrl^D for exit):\n");
    for(int i=0; i < 5; i++){
      printf("%d)  %s\n", i, menu[i].name);
    }
    printf("Option: ");
  }
  
} 