#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <elf.h>
#include <unistd.h>
#include <string.h>

void *map_start = NULL;
extern int startup(int argc, char** argv, int (*func)(int, char**));
extern int system_call();

char* give_type(Elf32_Phdr* phdr)
{ 
 char* type = "";
    switch (phdr->p_type) {
        case PT_NULL:
            type = "NULL";
            break;
        case PT_LOAD:
            type = "LOAD";
            break;
        case PT_DYNAMIC:
            type = "DYNAMIC";
            break;
        case PT_INTERP:
            type = "INTERP";
            break;
        case PT_NOTE:
            type = "NOTE";
            break;
        case PT_SHLIB:
            type = "SHLIB";
            break;
        case PT_PHDR:
            type = "PHDR";
            break;
        case PT_TLS:
            type = "TLS";
            break;
        case PT_NUM:
            type = "NUM";
            break;
        case PT_LOSUNW:
            type = "OS-specific";
            break;
        default:
            type = "Unknown";
            break;
    }
    return type;

}

void print_phdr(Elf32_Phdr* phdr, int phdr_num) {
    const char* type = "";
    const char* flag = "";
    const char* prot_flags = "";
    const char* map_flags = "";
    if (phdr->p_flags & PF_R) {
        prot_flags = "PROT_READ";
        flag = "R";
        map_flags = "MAP_PRIVATE";
    }
    else if (phdr->p_flags & PF_W) {
        prot_flags = "PROT_WRITE";
        flag = "W";
    }
    else if (phdr->p_flags & PF_X) {
        prot_flags = "PROT_EXEC";
        flag = "E";
    }

    if (phdr->p_flags & PF_R)
        map_flags = "MAP_PRIVATE";
    else
        map_flags = "MAP_SHARED";

    printf("%-6s    %08x    %08x    %08x    %06x    %06x    %2s    %x    %-12s    %-12s \n",give_type(phdr), phdr->p_offset, phdr->p_vaddr, phdr->p_paddr,phdr->p_filesz, phdr->p_memsz, flag, phdr->p_align, prot_flags, map_flags);
}


size_t give_flags(Elf32_Phdr *phdr){
    size_t prot = 0;
    if (phdr->p_flags & PF_R)
        prot |= PROT_READ;
    if (phdr->p_flags & PF_W)
        prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X)
        prot |= PROT_EXEC;

    return prot;

}
void load_phdr(Elf32_Phdr *phdr, int fd) {
    void* vaddr = (void* )(phdr->p_vaddr&0xfffff000);
    off_t offset = phdr->p_offset&0xfffff000;
    size_t padding = phdr->p_vaddr & 0xfff;
    void *map = mmap(vaddr, phdr->p_memsz + padding, give_flags(phdr), MAP_PRIVATE | MAP_FIXED, fd, offset);
    if (map == MAP_FAILED) {
        perror("mmap");
        return;
    }
}
int foreach_phdr(void* map_start, void (func)(Elf32_Phdr*, int), int arg) {
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*)map_start;
    Elf32_Phdr* phdr = (Elf32_Phdr*)(map_start + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; ++i) {
        func(&phdr[i], i);
        print_phdr(&phdr[i], i);
    }

    return 0;
}

int main(int argc, char **argv) {

    if (argc < 2) {
        fprintf(stderr, "file_usage: %s\n", argv[0]);
        return 1;
    }

    char *file_Name = argv[1];
    int file_desc = open(file_Name, O_RDONLY);
    if (file_desc == -1) {
        perror("Error opening file");
        return 1;
    }
    struct stat st;
    if (fstat(file_desc, &st) == -1) {
        perror("Error state");
        close(file_desc);
        return 1;
    }
    map_start = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, file_desc, 0);
    if (map_start == MAP_FAILED) {
        perror("Error mapping file");
        close(file_desc);
        return 1;
    }
    
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;

    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + ehdr->e_phoff);

    printf("Type      Offset      VirtAddr    PhysAddr    FileSiz    MemSiz    Flg  Align   ProtFlags       MapFlags\n");
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            load_phdr(&phdr[i], file_desc);
            print_phdr(&phdr[i], file_desc);
        }
    }

    //loop_phdr(map_start, print_phdr, 0);
    //munmap(map_start, st.st_size);
    close(file_desc);
    startup(argc - 1, argv + 1, (int (*)(int, char**))(uintptr_t)(ehdr->e_entry));
    munmap(map_start, st.st_size);
    return 0;
}