///////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

struct fun_desc{
    char *name;
    void (*fun)();
};

typedef struct{
    char FileName[102];
    char *map_start;               
    struct stat FD;
    Elf32_Ehdr *Ehdr;            
} ELF_FILE;

ELF_FILE *Elf_files[100];
int Elf_files_number;
int debug;
int Current_FD = -1;

void toggle_debug_mode();
void examineELFfile();
void printSectionNames();
void printSymbols();
void checkFilesForMerge();
void mergeELFfiles();
void quit();
void closeFd();
void munmapFunc();
void print_Ehdr(ELF_FILE *elfFile);
void debugPrint(int idx, Elf32_Shdr *section, ELF_FILE *elfFile);
Elf32_Shdr *ifOnlyOneSymtab(ELF_FILE *info);
void symbIsUndefined(Elf32_Sym *symtab, int sizeOfSymtab, char *strtab, char *symName);
void symbIsDefined(Elf32_Sym *symtab, int sizeOfSymtab, char *strtab, char *symName);
int get_new_index(Elf32_Shdr *shdr, int size, char *shstrtab, char *name);


void toggle_debug_mode()
{
    if (debug){
        printf("Debug flag is now off\n");
        debug = 0;
    } else {
        printf("Debug flag is now on\n");
        debug = 1;
    }
}

void examineELFfile(){
    char File_Name[256];
    ELF_FILE *elfFile = malloc(sizeof(ELF_FILE));
    
    printf("Please enter <file name>\n> ");
    if (scanf("%s", File_Name) == EOF){
        File_Name[strlen(File_Name) - 1] = 0;
    }

    Current_FD = open(File_Name, O_RDONLY);
    if (Current_FD == -1){
        perror("Error : can't open file\n");
        return;
    }

    int currentState = fstat(Current_FD, &elfFile->FD);
    if (currentState != 0){
        close(Current_FD);
        Current_FD = -1;
        perror("Error : stat failed\n");
        return;
    }
    if ((elfFile->map_start = mmap(0, elfFile->FD.st_size, PROT_READ, MAP_PRIVATE, Current_FD, 0)) == MAP_FAILED){
        close(Current_FD);
        Current_FD = -1;
        perror("Error : mmap failed\n");
        return;
    }
    strcpy(elfFile->FileName, File_Name);
    elfFile->Ehdr = (Elf32_Ehdr *)elfFile->map_start;
    print_Ehdr(elfFile);
    Elf_files[Elf_files_number] = elfFile;
    Elf_files_number = Elf_files_number + 1;
}


void printSectionNames(){
    if (Elf_files_number == 0){
        fprintf(stderr, " Error: you didn't examined a file yet!\n");
        return;
    }
    for (int i = 0; i < Elf_files_number; i++){
        ELF_FILE *elfFile = Elf_files[i];
        if (Current_FD == -1){
            printf("Error : there is no file directory\n");
            return;
        }
        printf("Sections of %s ELF file :", Elf_files[i]->FileName);
        printf("\nSection Headers:\n[Nr]   Name\t\tAddr\t\tOff\t\tSize\t\tType\n");
        int sectionsNumber = elfFile->Ehdr->e_shnum;
        int sectionIndex = 0;
        Elf32_Shdr *sectionTable;
        sectionTable = (Elf32_Shdr *)(elfFile->map_start + elfFile->Ehdr->e_shoff);
        char Type[256];
        while (sectionIndex < sectionsNumber){
            Elf32_Word section_name = sectionTable[sectionIndex].sh_name;
            Elf32_Addr section_address = sectionTable[sectionIndex].sh_addr;
            Elf32_Off section_offset = sectionTable[sectionIndex].sh_offset;
            Elf32_Word section_size = sectionTable[sectionIndex].sh_size;
            Elf32_Word section_type = sectionTable[sectionIndex].sh_type;

            switch (section_type)
            {
            case SHT_NULL:
                strcpy(Type, "NULL");
                break;
            case SHT_PROGBITS:
                strcpy(Type, "PROGBITS");
                break;
            case SHT_SYMTAB:
                strcpy(Type, "SYMTAB");
                break;
            case SHT_STRTAB:
                strcpy(Type, "STRTAB");
                break;
            case SHT_RELA:
                strcpy(Type, "RELA");
                break;
            case SHT_HASH:
                strcpy(Type, "HASH");
                break;
            case SHT_DYNAMIC:
                strcpy(Type, "DYNAMIC");
                break;
            case SHT_NOTE:
                strcpy(Type, "NOTE");
                break;
            case SHT_NOBITS:
                strcpy(Type, "NOBITS");
                break;
            case SHT_REL:
                strcpy(Type, "REL");
                break;
            case SHT_SHLIB:
                strcpy(Type, "SHLIB");
                break;
            case SHT_DYNSYM:
                strcpy(Type, "DYNSYM");
                break;
            case SHT_INIT_ARRAY:
                strcpy(Type, "INIT_ARRAY");
                break;
            case SHT_FINI_ARRAY:
                strcpy(Type, "FINI_ARRAY");
                break;
            case SHT_PREINIT_ARRAY:
                strcpy(Type, "PREINIT_ARRAY");
                break;
            case SHT_GROUP:
                strcpy(Type, "GROUP");
                break;
            case SHT_GNU_LIBLIST:
                strcpy(Type, "GNU_LIBLIST");
                break;
            case SHT_CHECKSUM:
                strcpy(Type, "CHECKSUM");
                break;
            case SHT_LOSUNW:
                strcpy(Type, "LOSUNW");
                break;
            case SHT_SUNW_COMDAT:
                strcpy(Type, "SUNW_COMDAT");
                break;
            case SHT_SUNW_syminfo:
                strcpy(Type, "SUNW_syminfo");
                break;
            case SHT_SYMTAB_SHNDX:
                strcpy(Type, "SYMTAB_SHNDX");
                break;
            case SHT_NUM:
                strcpy(Type, "NUM");
                break;
            case SHT_LOOS:
                strcpy(Type, "LOOS");
                break;
            case SHT_GNU_ATTRIBUTES:
                strcpy(Type, "GNU_ATTRIBUTES");
                break;
            case SHT_GNU_HASH:
                strcpy(Type, "GNU_HASH");
                break;
            case SHT_GNU_verdef:
                strcpy(Type, "GNU_verdef");
                break;
            case SHT_GNU_verneed:
                strcpy(Type, "GNU_verneed");
                break;
            case SHT_GNU_versym:
                strcpy(Type, "GNU_versym");
                break;
            case SHT_HIPROC:
                strcpy(Type, "HIPROC");
                break;
            case SHT_HIUSER:
                strcpy(Type, "HIUSER");
                break;
            case SHT_LOUSER:
                strcpy(Type, "LOUSER");
                break;
            case SHT_LOPROC:
                strcpy(Type, "LOPROC");
                break;
            default:
                strcpy(Type, "\0");
                break;
            }
            char *currentTable;
            currentTable = (char *)(elfFile->map_start + sectionTable[elfFile->Ehdr->e_shstrndx].sh_offset);
            if (sectionIndex >= 10){
                printf("[%2d] %-16s\t%08x\t%08x\t%08x\t%s\n",
                       sectionIndex, &currentTable[sectionTable[sectionIndex].sh_name], section_address, section_offset, section_size, Type);
            } else {
                printf("[%2d] %-16s\t%08x\t%08x\t%08x\t%s\n",
                       sectionIndex, &currentTable[sectionTable[sectionIndex].sh_name], section_address, section_offset, section_size, Type);
            }
            sectionIndex = sectionIndex + 1;
            debugPrint(sectionIndex, sectionTable, elfFile);
        }
        printf("\n");
    }
}

char* getSectionName(int index, ELF_FILE *elfFile){
    Elf32_Shdr* Shdr = (Elf32_Shdr*) (elfFile->map_start + elfFile->Ehdr->e_shoff);
    char* sh_strtab1 = elfFile->map_start + Shdr[elfFile->Ehdr->e_shstrndx].sh_offset;
    return (&sh_strtab1[Shdr[index].sh_name]);
}

void printSymbolTable(Elf32_Sym* sym_tab, int sym_num, char* str_tab,  ELF_FILE *elfFile){
  int i, symbValue, symbIndex;
  char* symbName;
  printf("Num: Value     Ndx  Sec_Name          Sym_Name\n");
  for(i = 0; i<sym_num; i++){
    symbName = str_tab + sym_tab[i].st_name;
    symbValue = sym_tab[i].st_value;
    symbIndex = sym_tab[i].st_shndx;
    
    if(symbIndex == SHN_UNDEF) printf("%3d: %08x  %-3s  %-16s  %s\n",i,symbValue,"UND","UND",symbName);
    else if(symbIndex == SHN_ABS) printf("%3d: %08x  %-3s  %-16s  %s\n",i,symbValue,"ABS","ABS",symbName);
    else printf("%3d: %08x  %3d  %-16s  %-16s\n",i,symbValue,symbIndex,getSectionName(symbIndex,elfFile),symbName);
  }
}

void printSymbols(){
    for (int i = 0; i < Elf_files_number; i++){
        ELF_FILE *elfFile = Elf_files[i];
        if (Current_FD == -1){
            perror("Error : there is no file directory\n");
            return;
        }
        Elf32_Sym* symbTable;
        Elf32_Sym* dynSymb;
        char* strTable;
        char* dynStrTable;
        int symbNum = 0;
        int dynSymbNum = 0;
        int i, sectionsNum;
        sectionsNum = elfFile->Ehdr->e_shnum;
        Elf32_Shdr* Shdr = (Elf32_Shdr*) (elfFile->map_start + elfFile->Ehdr->e_shoff);
        for(i=0; i < sectionsNum; i++){
            if(Shdr[i].sh_type == SHT_SYMTAB){
            symbTable = (Elf32_Sym*)(elfFile->map_start + Shdr[i].sh_offset);
            symbNum = Shdr[i].sh_size/sizeof(Elf32_Sym);
            strTable = elfFile->map_start + Shdr[Shdr[i].sh_link].sh_offset;
            } else if(Shdr[i].sh_type == SHT_DYNSYM){
            dynSymb = (Elf32_Sym*)(elfFile->map_start + Shdr[i].sh_offset);
            dynSymbNum = Shdr[i].sh_size/sizeof(Elf32_Sym);
            dynStrTable = elfFile->map_start + Shdr[Shdr[i].sh_link].sh_offset;
            }
        }
        if(dynSymbNum > 0){
        printf("\nSymbol table '.dynsym' contains %d entries:\n",dynSymbNum);
        printSymbolTable(dynSymb, dynSymbNum, dynStrTable, elfFile);
        }
    
        if(symbNum > 0){
        printf("\nSymbol table '.symtab' contains %d entries:\n",symbNum);
        printSymbolTable(symbTable, symbNum, strTable, elfFile);
        }
    }
}


void checkFilesForMerge(){
    if (Elf_files_number < 2){
        printf("Error : you must have at least 2 ELF files!\n");
        return;
    }
    Elf32_Shdr *sectionHDR1 = ifOnlyOneSymtab(Elf_files[0]);
    Elf32_Shdr *sectionHDR2 = ifOnlyOneSymtab(Elf_files[1]);
    
    Elf32_Shdr *sectionFile1 = (Elf32_Shdr *)(Elf_files[0]->map_start + Elf_files[0]->Ehdr->e_shoff);
    int symbolsTableSize1 = sectionHDR1->sh_size / sectionHDR1->sh_entsize;
    char *strTable1 = (char *)(Elf_files[0]->map_start + sectionFile1[sectionHDR1->sh_link].sh_offset);
    Elf32_Sym *symbTable1 = (Elf32_Sym *)(Elf_files[0]->map_start + sectionHDR1->sh_offset);


    Elf32_Shdr *sectionFile2 = (Elf32_Shdr *)(Elf_files[1]->map_start + Elf_files[1]->Ehdr->e_shoff);
    int symbolsTableSize2 = sectionHDR2->sh_size / sectionHDR2->sh_entsize;
    char *strTable2 = (char *)(Elf_files[1]->map_start + sectionFile2[sectionHDR2->sh_link].sh_offset);
    Elf32_Sym *symbTable2 = (Elf32_Sym *)(Elf_files[1]->map_start + sectionHDR2->sh_offset);

    int i = 1;
    while (i < symbolsTableSize1){
        switch (symbTable1[i].st_shndx){
        case SHN_UNDEF: 
            symbIsUndefined(symbTable2, symbolsTableSize2, strTable2, strTable1 + symbTable1[i].st_name);
            break;
        default: 
            symbIsDefined(symbTable2, symbolsTableSize2, strTable2, strTable1 + symbTable1[i].st_name);
            break;
        }
        i = i + 1;
    }

    i = 1;
    while (i < symbolsTableSize2){
        switch (symbTable2[i].st_shndx){
        case SHN_UNDEF:
            symbIsUndefined(symbTable1, symbolsTableSize1, strTable1, strTable2 + symbTable2[i].st_name);
            break;
        default:
            symbIsDefined(symbTable1, symbolsTableSize1, strTable1, strTable2 + symbTable2[i].st_name);
            break;
        }
        i = i + 1;
    }
}

void mergeELFfiles(){
    if (Elf_files_number < 2){
        printf("Error : you must have at least 2 ELF files!\n");
        return;
    }
    FILE *mergedElfFile = fopen("out.ro", "wb");

    Elf32_Shdr *sectionHDR1 = ifOnlyOneSymtab(Elf_files[0]);
    void *map_start1 = Elf_files[0]->map_start;
    Elf32_Ehdr *HDR1 = Elf_files[0]->Ehdr;
    Elf32_Shdr *Shdr1 = (Elf32_Shdr *)(map_start1 + HDR1->e_shoff);
    char *Shstrtab1 = (char *)(map_start1 + Shdr1[HDR1->e_shstrndx].sh_offset);

    Elf32_Shdr *sectionHDR2 = ifOnlyOneSymtab(Elf_files[1]);
    void *map_start2 = Elf_files[1]->map_start;
    Elf32_Ehdr *HDR2 = Elf_files[1]->Ehdr;
    Elf32_Shdr *Shdr2 = (Elf32_Shdr *)(map_start2 + HDR2->e_shoff);
    char *Shstrtab2 = (char *)(map_start2 + Shdr2[HDR2->e_shstrndx].sh_offset);

    int update_section_headers = fwrite((char *)map_start1, 1, 54, mergedElfFile);

    char new_section_hdr[HDR1->e_shnum * HDR1->e_shentsize];
    memcpy(new_section_hdr, (char *)(Shdr1), HDR1->e_shnum * HDR1->e_shentsize);
    printf("\nSection Headers:\n");
    for (int i = 0; i < HDR1->e_shnum; i++){
        int newOffset = ftell(mergedElfFile);
        if (i == 0){
            printf("\noffset: 0\n");
        } else {
            printf("The section header name: %s\n", Shstrtab1 + Shdr1[i].sh_name);
            printf("The offset: %x\n", newOffset);
        }
        if (strcmp(Shstrtab1 + Shdr1[i].sh_name, ".text") == 0 || strcmp(Shstrtab1 + Shdr1[i].sh_name, ".data") == 0 || strcmp(Shstrtab1 + Shdr1[i].sh_name, ".rodata") == 0){
            Elf32_Shdr *section;
            char *Name = Shstrtab1 + Shdr1[i].sh_name;
            for (int j = 0; j < HDR2->e_shnum; j++){
                if (strcmp(Name, Shstrtab2 + Shdr2[j].sh_name) == 0){
                    section = Shdr2 + j;
                }
            }
            update_section_headers = fwrite((char *)(map_start1 + Shdr1[i].sh_offset), 1, Shdr1[i].sh_size, mergedElfFile);
            if (section != NULL){
                update_section_headers = fwrite((char *)(map_start2 + section->sh_offset), 1, section->sh_size, mergedElfFile);
                printf("Size = %x \n", ((Elf32_Shdr *)new_section_hdr)[i].sh_size + section->sh_size);
                ((Elf32_Shdr *)new_section_hdr)[i].sh_size += section->sh_size;
            }
        }

        else if (strcmp(Shstrtab1 + Shdr1[i].sh_name, ".symtab") == 0){
            char symbols[Shdr1[i].sh_size];
            memcpy(symbols, (char *)(map_start1 + Shdr1[i].sh_offset), Shdr1[i].sh_size);
            Elf32_Shdr *SectionHeader1 = (Elf32_Shdr *)(Elf_files[0]->map_start + Elf_files[0]->Ehdr->e_shoff);
            Elf32_Shdr *SectionHeader2 = (Elf32_Shdr *)(Elf_files[1]->map_start + Elf_files[1]->Ehdr->e_shoff);
            int symTableSize1 = sectionHDR1->sh_size / sectionHDR1->sh_entsize;
            int symTableSize2 = sectionHDR2->sh_size / sectionHDR2->sh_entsize;
            char *firstStrtab = (char *)(Elf_files[0]->map_start + SectionHeader1[sectionHDR1->sh_link].sh_offset);
            char *secondStrtab = (char *)(Elf_files[1]->map_start + SectionHeader2[sectionHDR2->sh_link].sh_offset);
            Elf32_Sym *symTable1 = (Elf32_Sym *)(Elf_files[0]->map_start + sectionHDR1->sh_offset);
            Elf32_Sym *symTable2 = (Elf32_Sym *)(Elf_files[1]->map_start + sectionHDR2->sh_offset);
            for (int i = 1; i < symTableSize1; i++){
                if (symTable1[i].st_shndx == SHN_UNDEF)
                {
                    for (int j = 1; j < symTableSize2; j++){
                        if (strcmp(firstStrtab + symTable1[i].st_name, secondStrtab + symTable2[j].st_name) == 0){
                            char *name = Shstrtab2 + Shdr2[(symTable2 + j)->st_shndx].sh_name;
                            ((Elf32_Sym *)symbols + i)->st_shndx = get_new_index(Shdr1, symTableSize1, Shstrtab1, name);
                            ((Elf32_Sym *)symbols + i)->st_value = (symTable2 + j)->st_value;
                        }
                    }
                }
            }

            update_section_headers = fwrite(symbols, 1, Shdr1[i].sh_size, mergedElfFile);
            printf("Size = %x \n", Shdr1[i].sh_size);
        } else {

            update_section_headers = fwrite((char *)(map_start1 + Shdr1[i].sh_offset), 1, Shdr1[i].sh_size, mergedElfFile);
            printf("Size = %x \n", Shdr1[i].sh_size);
        }
        if (i != 0)
            ((Elf32_Shdr *)new_section_hdr)[i].sh_offset = newOffset;
        printf("\n");
    }

    printf("\nELF Header:\n");
    int mergedShoff = ftell(mergedElfFile);
    update_section_headers = fwrite((char *)new_section_hdr, 1, HDR1->e_shnum * HDR1->e_shentsize, mergedElfFile);
    printf("\nThe start of section headers: %d\n", mergedShoff);
    fseek(mergedElfFile, 32, SEEK_SET);
    update_section_headers = fwrite((char *)&mergedShoff, 4, 1, mergedElfFile);
    printf("The number of section headers: %d\n", HDR1->e_shnum);
    fclose(mergedElfFile);
}

void quit(){
    closeFd();
    munmapFunc();

    if (debug == 1)
    {
        printf("Quitting..\n");
    }

    exit(0);
}

void closeFd(){
    if (!(Current_FD == -1)){
        close(Current_FD);
        Current_FD = -1;
    }
}
void munmapFunc(){
    for (int i = 0; i < Elf_files_number; i++){
        ELF_FILE *elfFile = Elf_files[i];
        if (elfFile->map_start == NULL){
            return; 
        }

        if (munmap(elfFile->map_start, elfFile->FD.st_size) < 0){
            perror("Error : munmap\n");
        }
    }
}


void print_Ehdr(ELF_FILE *elfFile){
    printf("\nELF Header:\n");
    printf("Magic number:\t\t\t%c %c %c\n",
        elfFile->Ehdr->e_ident[EI_MAG1],
        elfFile->Ehdr->e_ident[EI_MAG2],
        elfFile->Ehdr->e_ident[EI_MAG3]);
    printf("Data encoding scheme:\t\t\t");
    if(elfFile->Ehdr->e_ident[EI_DATA] == 1) printf("2's complement, little endian\n");
    else if(elfFile->Ehdr->e_ident[EI_DATA] == 2) printf("2's complement, big endian\n");
    else printf("Invalid encoding.\n");
    printf("Entry point:\t\t\t\t%X\n", elfFile->Ehdr->e_entry);
    printf("Section header table offset:\t\t%d (bytes into file)\n",elfFile->Ehdr->e_shoff);
    printf("Number of section header entries:\t%d\n",elfFile->Ehdr->e_shnum);
    printf("Size of each section header entry:\t%d (bytes)\n",elfFile->Ehdr->e_shentsize);
    printf("Program header table offset:\t\t%d (bytes into file)\n",elfFile->Ehdr->e_phoff);
    printf("Number of program header entries:\t%d\n",elfFile->Ehdr->e_phnum);
    printf("Size of each program header entry:\t%d (bytes)\n\n",elfFile->Ehdr->e_phentsize);
}


void debugPrint(int idx, Elf32_Shdr *section, ELF_FILE *elfFile){
    if (debug){
        printf("\nshstrndx = 0x%-27X", elfFile->Ehdr->e_shstrndx);
        printf("section name offset = 0x%X\n\n", section[idx].sh_name);
    }
}


Elf32_Shdr *ifOnlyOneSymtab(ELF_FILE *info){
    Elf32_Shdr *shdr = (Elf32_Shdr *)(info->map_start + info->Ehdr->e_shoff);
    Elf32_Shdr *section = NULL;
    int counter = 0;
    int i = 0;
    while (i < info->Ehdr->e_shnum){
        if (shdr[i].sh_type == SHT_SYMTAB || shdr[i].sh_type == SHT_DYNSYM){
            section = &shdr[i];
            counter++;
        }
        i++;
    }
    if (counter != 1){
        section = NULL;
    }
    return section;
}

void symbIsUndefined(Elf32_Sym *symtab, int sizeOfSymtab, char *strtab, char *symName){
    int i = 1;
    while (i < sizeOfSymtab){
        if (strcmp(symName, strtab + symtab[i].st_name) == 0){
            if (symtab[i].st_shndx == SHN_UNDEF){
                fprintf(stderr, "Symbol %s undefined\n", symName);
            }
            return;
        }
        i++;
    }
    printf("Symbol %s undefined\n", symName);
}
void symbIsDefined(Elf32_Sym *symtab, int sizeOfSymtab, char *strtab, char *symName){
    if (strcmp(symName, "") == 0){
        return; 
    }
    int i = 1;
    while (i < sizeOfSymtab){
        if (strcmp(symName, strtab + symtab[i].st_name) == 0){
            if (symtab[i].st_shndx != SHN_UNDEF){
                fprintf(stderr, "Symbol %s defined multiple times\n", symName);
            }
            return;
        }
        i++;
    }
}

int get_new_index(Elf32_Shdr *Shdr, int size, char *shstrtab, char *Name){
    int newIndex = SHN_UNDEF;
    int i = 0;
    while (i < size){
        if (strcmp(shstrtab + Shdr[i].sh_name, Name) == 0){
            newIndex = i;
            return newIndex;
        }
        i++;
    }
    return newIndex;
}

int main(int argc, char *argv[]){
    ELF_FILE *elfFile = malloc(sizeof(ELF_FILE));
    debug = 0;
    Elf_files_number = 0;

    struct fun_desc menu[] = {{"Toggle Debug Mode", toggle_debug_mode},
                              {"Examine ELF File", examineELFfile},
                              {"Print Section Names", printSectionNames},
                              {"Print Symbols", printSymbols},
                              {"Check Files for Merge", checkFilesForMerge},
                              {"Merge ELF Files", mergeELFfiles},
                              {"Quit", quit},
                              {NULL, NULL}};

    while (true){
        printf("Please choose an action:\n");
        for (int i = 0; i < 7; i++){
            printf("%d-%s\n", i, menu[i].name);
        }
        printf("Your option is: ");

        int option;
        if (scanf("%d", &option) == EOF){
            printf("\n\n");
            return 0;
        }
        if (option < 0 || option > 6){
            printf("Error: index out of bounds\n");
            return 0;
        }
        menu[option].fun();
        printf("\n");
    }
    return 0;
}