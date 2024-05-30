/*this code by Amir abu elhijaa
 id:213034655*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 

char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  /* TODO: Complete during task 2.a */
  char *prt=array;
  for(int i=0;i<array_length;i++){
    mapped_array[i]=f(*(prt+i));
  }
  return mapped_array;
}
char my_get(char c){
   char a = fgetc(stdin);
   return a;
}
char cprt(char c){
    if(c>=0x20 && c<=0x7E){
        printf("%c\n",c);
    }
    else{
        printf("%c\n",'.');
    }
    return c;
}
char encrypt(char c){
    if(c>=0x20 && c<=0x7E){
        return c+1;
    }
    else{
        return c;
    }
}
char decrypt(char c){
    if(c>=0x20 && c<=0x7E)
        return c-1;
    else
        return c;
}
char xprt(char a){
    if(a>=0x20 && a<=0x7E)printf("%X\n",a);
    else printf("%c\n",'.');
    return a;
}

struct fun_desc {
char *name;
char (*fun)(char);
};

int main(int argc, char **argv){
  int size_of_array=5;
  char* carray = (char*)(malloc(size_of_array*sizeof(char)));
  struct fun_desc menu[] = {{"Get String", my_get},{"Print String", cprt},{"Encrypt", encrypt},{"Decrypt", decrypt},{"Print Hex", xprt},{NULL, NULL}};
  int size_of_the_menu = sizeof(menu)/sizeof(struct fun_desc)-1;
    printf("Please choose a function (ctrl^D for exit):\n");
    for(int i=0; i<size_of_the_menu; i++){
      printf("%d)  %s\n", i, menu[i].name);
    }
  printf("Option: ");
  char input[100];
  while(fgets(input,100, stdin) != NULL){
    int choice = atoi(input);
    if(choice >=0 && choice < size_of_the_menu){
      printf("Within bounds\n");
      carray = map(carray, size_of_array, menu[choice].fun);
      printf("DONE.\n\n");
    }
    else{
      printf("Not within bounds\n");
      free(carray);
      return 0;
    }
    printf("Please choose a function (ctrl^D for exit):\n");
    for(int i=0; i<size_of_the_menu; i++){
      printf("%d)  %s\n", i, menu[i].name);
    }
    printf("Option: ");
  }
  free(carray);
  return 0;
} 
