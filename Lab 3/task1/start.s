section .data
    nl db 10            
    myChar: db 0
    outFile: dd 1
    inFile: dd 0
section .text
    global main
    extern strlen

main:
    push ebp            
    mov ebp, esp        

    ; Get argc and argv
    mov esi, [ebp+8]    
    mov edi, [ebp+12]   
    ; Loop through each argument and print it to stdout
    sub esi, 1          
    add edi, 4          
    cmp esi, 0
    je encoder
print_loop:
    ; Print argument string
    mov ecx, dword [edi]    
    push ecx                
    call strlen             
    add esp, 4              
    mov edx, eax            
    mov eax, 4              
    mov ebx, 1              
    int 0x80                
    
    ; Print newline
    mov eax, 4              
    mov ebx, 1              
    mov ecx, nl             
    mov edx, 1              
    int 0x80                
    mov ecx, dword [edi]    
    cmp word[ecx], "-i"
    je open_input_file
    cmp word[ecx], "-o"
    je open_output_file
    return_loop:
    ; Move to next argument
    add edi, 4              
    dec esi
    cmp esi, 0
    jne print_loop
    encoder:
    mov eax, 3              
    mov ebx, dword[inFile]              
    mov ecx, myChar             
    mov edx, 1              
    int 0x80                
    cmp eax, 0
    jle exit_program
    cmp byte[myChar], 'A'
    jl skip_print
    cmp byte[myChar], 'z'
    jg skip_print
    inc byte[myChar]
    skip_print:
    mov eax, 4              
    mov ebx, dword[outFile]              
    mov ecx, myChar             
    mov edx, 1             
    int 0x80                
    jmp encoder
    exit_program:
    ; Exit program
    xor eax, eax            
    mov ebx, 0              
    int 0x80                

    ; Clean up stack and return
    mov esp, ebp
    pop ebp
    ret
open_input_file:
    mov eax, 5
    mov ebx, ecx
    add ebx, 2
    mov ecx, 0
    int 0x80
    cmp eax, 0
    jl exit_program
    mov dword[inFile], eax
    jmp return_loop
    
open_output_file:
    mov eax, 5
    mov ebx, ecx
    add ebx, 2
    mov ecx, 1101o ;O_TRUNC | O_CREAT | O_WRONLY
    mov edx, 644o
    int 0x80
    cmp eax, 0
    jl exit_program
    mov dword[outFile], eax
    jmp return_loop
