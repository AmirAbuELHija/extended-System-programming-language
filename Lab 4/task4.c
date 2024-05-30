#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


int digit_counter(char *input){
    int index = 0;
    int counter = 0;
    if (input != NULL){
        while (*(input + index) != 0 && *(input + index) != 10 && *(input + index) != 32){
            if (input[index] <= 57 && input[index] >= 48){
                counter = counter + 1;
            }
            index = index + 1;
        }
        return counter;
    }
    return -1;
}
int main(int argc, char *argv[]){
    printf("%d\n", digit_counter(argv[1]));
}