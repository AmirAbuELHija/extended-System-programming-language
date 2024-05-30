#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char **argv) {
  int debugMode = 0;
  int encodingMode = 0;
  
  FILE *infile = stdin;
  FILE *outfile = stdout;
  char *str = NULL;
  char *encoding_str = NULL;

  for(int i=1; i<argc; i++){
    if(strcmp(argv[i],"+D") == 0){
      debugMode = 1;
    }
    if(debugMode){
      fprintf(stderr, "%s\n", argv[i]);
    }
    if(strcmp(argv[i],"-D") == 0){
      debugMode = 0;
    }
    
    str = argv[i];
    if(*(str) == '-' && *(str + 1) == 'i'){
      infile = fopen(argv[i] + 2, "r");
      continue;

    } else if (*(str) == '-' && *(str + 1) == 'o'){
      outfile = fopen(argv[i] + 2, "w");
      continue;
    }

    if(*(str) == '+' && *(str + 1) == 'e'){
      encoding_str = argv[i] + 2;
      encodingMode = 1;
      continue;

    } else if (*(str) == '-' && *(str + 1) == 'e'){
      encoding_str = argv[i] + 2;
      encodingMode = -1;
      continue;
    }
    
    
  }
  str = encoding_str;
  int c;
  while((c = fgetc(infile)) != EOF){
    if(*str == '\0'){
      str = encoding_str;
    }
    int upperBound;
    int lowerBound;
    if(c >= 'A' && c <= 'Z'){
      upperBound = 'Z';
      lowerBound = 'A';
    } else if (c >= 'a' && c <= 'z'){
      upperBound = 'z';
      lowerBound = 'a';
    } else if (c >= '0' && c <= '9'){
      upperBound = '9';
      lowerBound = '0';
    } else {
      upperBound = c;
      lowerBound = c;
    }

    if(upperBound != lowerBound){
      c += encodingMode * (*(str++) - '0');
    }else{
      str++;
    }
    
    if(c > upperBound){
      c = lowerBound + (c - upperBound - 1);
    }
    fputc(c,outfile);
  }
  fclose(infile);
  fclose(outfile);
  return 0;
}