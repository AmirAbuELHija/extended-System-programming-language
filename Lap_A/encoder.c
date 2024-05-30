#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv) {
    int add_or_subs = 0;// 1 is add ---- -1 is subs
    char *key = NULL;
    char *input_file = NULL;// -i
    char *output_file = NULL;// -o
    int debug = 0; // 0 for off --- 1 for on
    FILE *input_stream = stdin;
    FILE *output_stream = stdout;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "+D") == 0) {
            debug = 1;
        } else if (strcmp(argv[i], "-D") == 0) {
            debug = 0;
        } else if (argv[i][0]=='+' && argv[i][1]=='e') {
            add_or_subs = 1;// 1 is add  -1 is subs = 1;
            key = argv[i] + 2;
        } else if (argv[i][0]=='-' && argv[i][1]=='e') {
            add_or_subs = -1;// 1 is add  -1 is subs = -1;
            key = argv[i] + 2;
        } else if (argv[i][0]=='-' && argv[i][1]=='i') {
            input_file = argv[i] + 2;
        } else if (argv[i][0]=='-' && argv[i][1]=='o') {
            output_file = argv[i] + 2;
        }
        if(debug==1 && strcmp(argv[i], "+D") != 0){
            fprintf(stderr,"%s\n",argv[i]);
        }
    }
    if (input_file != NULL) {
        input_stream = fopen(input_file, "r");
    }
    
    if (output_file != NULL) {
        output_stream = fopen(output_file, "w");
    }
    if (add_or_subs != 0) {
        int index = 0;
        int charr;
        while ((charr = fgetc(input_stream)) != EOF) {
            int key_value = key[index] - '0';
            if (charr <= 'Z' && charr >= 'A'){
                if(add_or_subs==1){
                charr = 'A'+((charr + key_value - 'A')%26);
                }
                else if (add_or_subs==-1){
                charr = 'A'+((charr - key_value - 'A'+26)%26);
                }
            } else if (charr <= 'z' && charr >= 'a') {
                if(add_or_subs==1){
                charr = 'a'+((charr + key_value - 'a')%26);
                }
                else if (add_or_subs==-1){
                charr = 'a'+((charr - key_value - 'a'+26)%26);
                }
            } else if (charr <= '9' && charr >= '0') {
                if(add_or_subs==1){
                charr = '0'+((charr + key_value - '0')%10);
                }
                else if (add_or_subs==-1){
                charr = '0'+((charr - key_value - '0'+10)%10);
                }
            }
        index = (index + 1);
        if(key[index]=='\0'){
            index=0;
        }
        fputc(charr, output_stream);
        }
    } else {
        int charr;
        while ((charr = fgetc(input_stream)) != EOF) {
            fputc(charr, output_stream);
        }
    }
    fclose(input_stream);
    fclose(output_stream);
    return 0;
}

